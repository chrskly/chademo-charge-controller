/*
 * This file is part of the ev mustang charge controller project.
 *
 * Copyright (C) 2022 Christian Kelly <chrskly@chrskly.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>

#include "chademostatemachine.h"
#include "battery.h"
#include "chademostation.h"
#include "chademo.h"
#include "chademocomms.h"

extern ChademoState state;


/*
 * State : idle
 *
 * IN1/CP      : deactivated
 * IN2/CP2     : deactivated
 * OUT1/CP3    : deactivated
 * OUT2        : deactivated
 * CS          : deactivated
 * Plug locked : no
 *
 */
void chademo_state_idle(ChademoEvent event) {

    switch (event) {

        case E_PLUG_INSERTED:

            printf("Switching to state : plug_in, reason : plugin inserted\n");
            state = chademo_state_plug_in;
            break;

        case E_BMS_UPDATE_RECEIVED:

            // Battery is full, go straight to inhibited state
            if ( battery_is_full() ) {
                printf("Switching to state : charge_inhibited, reason : battery is full\n");
                state = chademo_state_charge_inhibited;
            }

            break;

        case E_CHARGE_INHIBIT_ENABLED:

            printf("Switching to state : charge_inhibited, reason : received charge_inhibit input signal\n");
            state = chademo_state_charge_inhibited;
            break;

        default:

            printf("WARNING : received invalid event [%s]\n", event);

    }

}


/*
 * State : plug_in
 *
 * Plug has been inserted. Waiting on station pull IN1/CP high to indicate
 * station is ready to start.
 *
 * Note: possible issue here if IN1/CP and CS activate simultaneously or in the
 *       wrong order. Can this happen?
 *
 * IN1/CP      : deactivated
 * IN2/CP2     : deactivated
 * OUT1/CP3    : deactivated
 * OUT2/cont   : deactivated
 * CS          : activated
 * Plug locked : no
 *
 * Transitions:
 *  => handshaking
 *     - IN1/CP goes high
 *  => idle
 *     - plug removed
 */
void chademo_state_plug_in(ChademoEvent event) {

    switch (event) {

        case E_BMS_UPDATE_RECEIVED:

            // Battery is full, go straight to inhibited state
            if ( battery_is_full() ) {
                printf("Switching to state : charge_inhibited, reason : battery is full\n");
                state = chademo_state_charge_inhibited;
            }

            break;

        // Station has activated IN1/CP signal indicating it's ready to start charging
        case E_IN1_ACTIVATED:

            reinitialise_station();
            // Begin sending vehicle state over CAN to station
            enable_send_outbound_CAN_messages();
            printf("Switching to state : handshaking, reason : station enabled IN1/CP signal\n");
            state = chademo_state_handshaking;
            break;

        case E_PLUG_REMOVED:

            chademo_reinitialise();
            printf("Switching to state : idle, reason : plug removed\n");
            state = chademo_state_idle;
            break;

        case E_CHARGE_INHIBIT_ENABLED:

            printf("Switching to state : charge_inhibited, reason : received charge_inhibit input signal\n");
            state = chademo_state_charge_inhibited;
            break;

        default:

            printf("WARNING : received invalid event [%s]\n", event);

    }

}


/*
 * State : handshaking
 *  - exchanging params with station over CAN
 *
 * IN1/CP      : activated
 * IN2/CP2     : deactivated
 * OUT1/CP3    : deactivated
 * OUT2/cont   : deactivated
 * CS          : activated
 * Plug locked : no
 *
 * Charging station sends:
 *  - Control protocol number (0x109)
 *  - Available output voltage (0x108)
 *  - Available output current (0x108)
 *  - Battery incompatability (0x109)
 *
 * Car sends:
 *  - Control protocol number (0x102)
 *  - Rated capacity of battery (0x101)
 *  - Maximum battery voltage (0x100)
 *  - Maximum charging time (0x101)
 *  - Target battery voltage (0x102)
 *  - Vehicle charging enabled (0x102)
 *
 */
void chademo_state_handshaking(ChademoEvent event) {

    switch (event) {

        case E_IN1_DEACTIVATED:

            initiate_shutdown();
            printf("Switching to state : plug_in, reason : disabled IN1/CP signal\n");
            state = chademo_state_plug_in;

        case E_BMS_UPDATE_RECEIVED:

            // Battery is full, go straight to inhibited state
            if ( battery_is_full() ) {
                initiate_shutdown();
                printf("Switching to state : charge_inhibited, reason : battery full\n");
                state = chademo_state_charge_inhibited;
            }

            break;

        case E_STATION_CAPABILITIES_UPDATED:

            // Can the charger provide us with enough voltage?
            if ( ! chademo_station_voltage_sufficient() ) {
                initiate_shutdown();
                printf("Switching to state : error, reason : station cannot supply sufficient voltage\n");
                state = chademo_state_error;
                break;
            }

            // If we have received all of the params we need from the station, move to the next step
            if ( initial_parameter_exchange_with_station_complete() ) {
                signal_charge_go_ahead();
                printf("Switching to state : await_connector_lock, reason : initial param exchange complete\n");
                state = chademo_state_await_connector_lock;
                break;
            }

            break;

        case E_STATION_STATUS_UPDATED:

            // Check for error conditions from station
            if ( error_condition() ) {
                disable_send_outbound_CAN_messages();
                printf("Switching to state : error, reason : station reporting error condition\n");
                state = chademo_state_error;
            }

            // If we have received all of the params we need from the station, move to the next step
            if ( initial_parameter_exchange_with_station_complete() ) {
                signal_charge_go_ahead();
                printf("Switching to state : await_connector_lock, reason : initial param exchange complete\n");
                state = chademo_state_await_connector_lock;
                break;
            }

            break;

        case E_PLUG_REMOVED:

            chademo_reinitialise();
            state = chademo_state_idle;
            break;

        case E_CHARGE_INHIBIT_ENABLED:

            // FIXME disable CAN msgs
            // disable_send_outbound_CAN_messages();
            state = chademo_state_charge_inhibited;

        default:

            printf("WARNING : received invalid event [%s]\n", event);
    }

}


/*
 * State : await_connector_lock
 *
 * IN1/CP      : activated
 * IN2/CP2     : deactivated
 * OUT1/CP3    : activated
 * OUT2/cont   : deactivated
 * CS          : deactivated
 * Plug locked : no
 * 
 */
void chademo_state_await_connector_lock(ChademoEvent event) {

    switch (event) {

        case E_IN1_DEACTIVATED:

            initiate_shutdown();
            printf("Switching to state : plug_in, reason : disabled IN1/CP signal\n");
            state = chademo_state_plug_in;

        case E_BMS_UPDATE_RECEIVED:

            // Battery is full, go straight to inhibited state
            if ( battery_is_full() ) {
                initiate_shutdown();
                state = chademo_state_charge_inhibited;
            }

            break;

        case E_STATION_CAPABILITIES_UPDATED:

            break;

        case E_STATION_STATUS_UPDATED:

            // Check for error conditions from station
            if ( error_condition() ) {
                disable_send_outbound_CAN_messages();
                state = chademo_state_error;
            }

            // check if lock is complete
            if ( connector_is_locked() ) {
                state = chademo_state_await_insulation_test;
                break;
            }

            break;

        case E_PLUG_REMOVED:

            initiate_shutdown();
            chademo_reinitialise();
            state = chademo_state_idle;
            break;

        case E_CHARGE_INHIBIT_ENABLED:

            deactivate_out1();
            state = chademo_state_charge_inhibited;

        default:

            printf("WARNING : received invalid event [%s]\n", event);

    }

}


/*
 * State : chademo_await_insulation_test
 *
 * IN1/CP      : activated
 * IN2/CP2     : deactivated
 * OUT1/CP3    : activated
 * OUT2/cont   : deactivated
 * CS          : deactivated
 * Plug locked : yes
 *
 */
void chademo_state_await_insulation_test(ChademoEvent event) {

    switch (event) {

        case E_IN1_DEACTIVATED:

            initiate_shutdown();
            printf("Switching to state : plug_in, reason : disabled IN1/CP signal\n");
            state = chademo_state_plug_in;

        case E_BMS_UPDATE_RECEIVED:

            // Battery is full, go straight to inhibited state
            if ( battery_is_full() ) {
                initiate_shutdown();
                state = chademo_state_charge_inhibited;
            }

            break;

        /* Station has indicated that insulation test is over and it is now
         * ready to go into energy transfer state by pulling IN2/CP2 low.
         */
        case E_IN2_ACTIVATED:

            // close contactors
            permit_contactor_close();
            state = chademo_state_energy_transfer;  

        /* This shouldn't be possible as the plug connector lock should be
         * engaged here, but deal with this scenario anyway for safety sake.
         */
        case E_PLUG_REMOVED:

            chademo_reinitialise();
            state = chademo_state_idle;
            break;

        case E_CHARGE_INHIBIT_ENABLED:

            // FIXME how do we exit safely from here? send error msg?
            state = chademo_state_charge_inhibited;

        default:

            printf("WARNING : received invalid event [%s]\n", event);

    }

}


/*
 * State : energy_transfer
 *
 * Station is delivering energy to the batteries.
 *
 * IN1/CP      : activated
 * IN2/CP2     : activated
 * OUT1/CP3    : activated
 * OUT2/cont   : activated
 * CS          : activated
 * Plug locked : yes
 *
 */
void chademo_state_energy_transfer(ChademoEvent event) {

    switch (event) {

        case E_BMS_UPDATE_RECEIVED:

            // Stop charging if the BMS says the battery is full
            if ( battery_is_full() ) {
                initiate_shutdown();
                printf("Switching to state : winding down, reason : battery full\n");
                state = chademo_state_winding_down;
                break;
            }

            chademo_recalculate_charging_current_request();

            break;

        /* This shouldn't be possible as the plug connector lock should be
         * engaged here, but deal with this scenario anyway for safety sake.
         */
        case E_PLUG_REMOVED:

            chademo_reinitialise();
            // FIXME contactors, etc.
            state = chademo_state_idle;
            break;

        case E_STATION_CAPABILITIES_UPDATED:

            // Current available at station may have changed
            chademo_recalculate_charging_current_request();

            // If current available has changed then the charging time may need updating
            recalculate_charging_time();

            break;

        case E_STATION_STATUS_UPDATED:

            // Check for error conditions from station
            if ( error_condition() ) {
                disable_send_outbound_CAN_messages();
                state = chademo_state_error;
                break;
            }

            break;

        case E_CHARGE_INHIBIT_ENABLED:

            // FIXME how do we exit safely from here? send error msg?
            state = chademo_state_charge_inhibited;

        default:
            printf("WARNING : received invalid event [%s]\n", event);

    }

}

/*
 * State : winding_down
 *
 * We've told the station we want to stop charging by deactivating OUT1/CP3
 * and via CAN messaging. All we do here is ramp down the current to zero.
 * Once we reach zero, open the contactors.
 *
 * IN1/CP      : activated
 * IN2/CP2     : activated
 * OUT1/CP3    : deactivated
 * OUT2/cont   : activated
 * CS          : activated
 * Plug locked : yes
 *
 */
void chademo_state_winding_down(ChademoEvent event) {

    switch (event) {

        case E_BMS_UPDATE_RECEIVED:
            ramp_down_current_request();
            break;

        case E_STATION_CAPABILITIES_UPDATED:
            ramp_down_current_request();
            break;

        case E_STATION_STATUS_UPDATED:
            ramp_down_current_request();
            break;

        case E_PLUG_REMOVED:
            break;

        case E_CHARGE_INHIBIT_ENABLED:
            break;

        default:
            printf("WARNING : received invalid event [%s]\n", event);

    }

}

/*
 * State : charge_inhibited
 *
 * IN1/CP      : deactivated
 * IN2/CP2     : deactivated
 * OUT1/CP3    : 
 * OUT2        : 
 * CS          : deactivated
 * Plug locked : no
 *
 */
void chademo_state_charge_inhibited(ChademoEvent event) {

    switch (event) {

        case E_BMS_UPDATE_RECEIVED:
            break;

        case E_CHARGE_INHIBIT_DISABLED:
            break;

        default:
            printf("WARNING : received invalid event [%s]\n", event);

    }
}

/*
 * State : error
 *
 * IN1/CP      : deactivated
 * IN2/CP2     : deactivated
 * OUT1/CP3    : 
 * OUT2        : 
 * CS          : deactivated
 * Plug locked : no
 *
 */
void chademo_state_error(ChademoEvent event) {

    switch (event) {

        case E_BMS_UPDATE_RECEIVED:
            break;

        // Only way out of error is to remove the plug and start over
        case E_PLUG_REMOVED:
            chademo_reinitialise();
            state = chademo_state_idle;
            break;

        default:
            printf("WARNING : received invalid event [%s]\n", event);

    }
}




