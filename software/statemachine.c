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

#include "statemachine.h"
#include "battery.h"
#include "station.h"
#include "chademo.h"
#include "chademocomms.h"
#include "inputs.h"
#include "settings.h"

extern State state;


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
void state_idle(Event event) {

    switch (event) {

        case E_PLUG_INSERTED:

            printf("Switching to state : plug_in, reason : plugin inserted\n");
            state = state_plug_in;
            break;

        case E_BMS_UPDATE_RECEIVED:

            chademo_update_max_voltage_value();

            // Auxiliary check for CHARGE_INHIBIT condition
            if ( battery_is_full() || battery_is_too_hot() || battery_is_too_cold() ) {
                printf("Switching to state : charge_inhibited, reason : aux charge_inhibit check fired\n");
                state = state_charge_inhibited;
                break;
            }

            break;

        case E_BMS_LIVENESS_CHECK_FAILED:

            printf("Switching to state : error, reason : communication timeout with BMS\n");
            state = state_error;
            break;

        case E_CHARGE_INHIBIT_ENABLED:

            printf("Switching to state : charge_inhibited, reason : received charge_inhibit input signal\n");
            signal_charge_stop_digital();
            state = state_charge_inhibited;
            break;

        case E_STATION_LIVENESS_CHECK_FAILED:
            // Should never fire
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
void state_plug_in(Event event) {

    switch (event) {

        case E_BMS_UPDATE_RECEIVED:

            chademo_update_max_voltage_value();

            // Auxiliary check for CHARGE_INHIBIT condition
            if ( battery_is_full() || battery_is_too_hot() || battery_is_too_cold() ) {
                printf("Switching to state : charge_inhibited, reason : aux charge_inhibit check fired\n");
                state = state_charge_inhibited;
                break;
            }

            break;

        case E_BMS_LIVENESS_CHECK_FAILED:

            printf("Switching to state : error, reason : communication timeout with BMS\n");
            state = state_error;

            break;

        // Station has activated IN1/CP signal indicating it's ready to start charging
        case E_IN1_ACTIVATED:

            printf("Switching to state : handshaking, reason : station enabled IN1/CP signal\n");
            //FIXME validate value of chademo.maximumVoltage before going ahead
            reinitialise_station();
            // Begin sending vehicle state over CAN to station
            enable_send_outbound_CAN_messages();
            // Signal to station that car gives permission to charge
            activate_out1();
            enable_station_liveness_check();
            state = state_handshaking;
            break;

        case E_PLUG_REMOVED:

            printf("Switching to state : idle, reason : plug removed\n");
            chademo_reinitialise();
            state = state_idle;
            break;

        case E_CHARGE_INHIBIT_ENABLED:

            printf("Switching to state : charge_inhibited, reason : received charge_inhibit input signal\n");
            state = state_charge_inhibited;
            break;

        case E_STATION_LIVENESS_CHECK_FAILED:
            // Should never fire
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
 * OUT1/CP3    : activated
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
void state_handshaking(Event event) {

    switch (event) {

        case E_IN1_DEACTIVATED:

            printf("Switching to state : plug_in, reason : IN1/CP signal was disabled\n");
            signal_charge_stop_digital();
            state = state_plug_in;

        case E_BMS_UPDATE_RECEIVED:

            // Auxiliary check for CHARGE_INHIBIT condition
            if ( battery_is_full() || battery_is_too_hot() || battery_is_too_cold() ) {
                printf("Switching to state : charge_inhibited, reason : aux charge_inhibit check fired\n");
                state = state_charge_inhibited;
                break;
            }

            break;

        case E_BMS_LIVENESS_CHECK_FAILED:

            printf("Switching to state : error, reason : communication timeout with BMS\n");
            state = state_error;

            break;

        case E_STATION_CAPABILITIES_UPDATED:

            // Can the charger provide us with enough voltage?
            if ( ! chademo_station_voltage_sufficient() ) {
                printf("Switching to state : error, reason : station cannot supply sufficient voltage\n");
                signal_charge_stop_digital();
                state = state_error;
                break;
            }

            // If we have received all of the params we need from the station, move to the next step
            if ( initial_parameter_exchange_with_station_complete() ) {
                printf("Switching to state : await_connector_lock, reason : initial param exchange complete\n");
                signal_charge_go_ahead_digital();
                signal_charge_go_ahead_discrete();
                state = state_await_connector_lock;
                break;
            }

            break;

        case E_STATION_STATUS_UPDATED:

            if ( station_is_reporting_station_malfunction() ) {
                disable_send_outbound_CAN_messages();
                printf("Switching to state : error, reason : station reporting station malfunction (electrical, connector lock, or emergency stop button)\n");
                state = state_error;
                break;
            }

            if ( station_is_reporting_battery_incompatibility() ) {
                disable_send_outbound_CAN_messages();
                printf("Switching to state : error, reason : station reporting battery incompatiblity\n");
                state = state_error;
                break;
            }

            if ( station_is_reporting_charging_system_malfunction() ) {
                disable_send_outbound_CAN_messages();
                printf("Switching to state : error, reason : station reporting 'Charging system malfunction'\n");
                state = state_error;
                break;
            }

            // If we have received all of the params we need from the station, move to the next step
            if ( initial_parameter_exchange_with_station_complete() ) {
                printf("Switching to state : await_connector_lock, reason : initial param exchange complete\n");
                signal_charge_go_ahead_digital();
                signal_charge_go_ahead_discrete();
                state = state_await_connector_lock;
                break;
            }

            break;

        case E_PLUG_REMOVED:

            chademo_reinitialise();
            state = state_idle;
            break;

        case E_CHARGE_INHIBIT_ENABLED:

            // FIXME disable CAN msgs
            // disable_send_outbound_CAN_messages();
            state = state_charge_inhibited;

        case E_STATION_LIVENESS_CHECK_FAILED:
            // Should never fire
            break;

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
void state_await_connector_lock(Event event) {

    switch (event) {

        case E_IN1_DEACTIVATED:

            signal_charge_stop_digital();
            printf("Switching to state : plug_in, reason : disabled IN1/CP signal\n");
            state = state_plug_in;

        case E_BMS_UPDATE_RECEIVED:

            // Auxiliary check for CHARGE_INHIBIT condition
            if ( battery_is_full() || battery_is_too_hot() || battery_is_too_cold() ) {
                printf("Switching to state : charge_inhibited, reason : aux charge_inhibit check fired\n");
                state = state_charge_inhibited;
                break;
            }

            break;

        case E_BMS_LIVENESS_CHECK_FAILED:

            printf("Switching to state : error, reason : communication timeout with BMS\n");
            state = state_error;

            break;

        case E_STATION_CAPABILITIES_UPDATED:

            break;

        case E_STATION_STATUS_UPDATED:

            if ( station_is_reporting_station_malfunction() ) {
                disable_send_outbound_CAN_messages();
                printf("Switching to state : error, reason : station reporting station malfunction (electrical, connector lock, or emergency stop button)\n");
                state = state_error;
                break;
            }

            if ( station_is_reporting_battery_incompatibility() ) {
                disable_send_outbound_CAN_messages();
                printf("Switching to state : error, reason : station reporting battery incompatiblity\n");
                state = state_error;
                break;
            }

            if ( station_is_reporting_charging_system_malfunction() ) {
                disable_send_outbound_CAN_messages();
                printf("Switching to state : error, reason : station reporting 'Charging system malfunction'\n");
                state = state_error;
                break;
            }

            // check if lock is complete
            if ( connector_is_locked() ) {
                state = state_await_insulation_test;
                break;
            }

            break;

        case E_PLUG_REMOVED:

            signal_charge_stop_digital();
            chademo_reinitialise();
            state = state_idle;
            break;

        case E_CHARGE_INHIBIT_ENABLED:

            deactivate_out1();
            state = state_charge_inhibited;

        case E_STATION_LIVENESS_CHECK_FAILED:

            printf("Switching to state : error, reason : communication timeout with station\n");
            state = state_error;
            break;

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
void state_await_insulation_test(Event event) {

    switch (event) {

        case E_IN1_DEACTIVATED:

            printf("Switching to state : plug_in, reason : disabled IN1/CP signal\n");
            signal_charge_stop_digital();
            state = state_plug_in;

        case E_BMS_UPDATE_RECEIVED:

            if ( battery_is_full() || battery_is_too_hot() || battery_is_too_cold() ) {
                printf("Switching to state : charge_inhibited, reason : aux charge_inhibit check fired\n");
                signal_charge_stop_digital();
                state = state_charge_inhibited;
                break;
            }

            break;

        case E_BMS_LIVENESS_CHECK_FAILED:

            printf("Switching to state : error, reason : communication timeout with BMS\n");
            signal_charge_stop_digital();
            state = state_error;

            break;

        /* Station has indicated that insulation test is over and it is now
         * ready to go into energy transfer state by pulling IN2/CP2 low.
         */
        case E_IN2_ACTIVATED:

            printf("Switching to state : energy_transfer, reason : IN2/CP2 activated\n");
            permit_contactor_close();
            state = state_energy_transfer;  

        /* This shouldn't be possible as the plug connector lock should be
         * engaged here, but deal with this scenario anyway for safety sake.
         */
        case E_PLUG_REMOVED:

            printf("Switching to state : idle, reason : plug removed\n");
            chademo_reinitialise();
            signal_charge_stop_digital();
            state = state_idle;
            break;

        case E_STATION_CAPABILITIES_UPDATED:
            // FIXME
            break;

        case E_STATION_STATUS_UPDATED:

            if ( station_is_reporting_station_malfunction() ) {
                printf("Switching to state : error, reason : station reporting station malfunction (electrical, connector lock, or emergency stop button)\n");
                signal_charge_stop_digital();
                state = state_error;
                break;
            }

            if ( station_is_reporting_battery_incompatibility() ) {
                printf("Switching to state : error, reason : station reporting battery incompatiblity\n");
                signal_charge_stop_digital();
                state = state_error;
                break;
            }

            if ( station_is_reporting_charging_system_malfunction() ) {
                printf("Switching to state : error, reason : station reporting 'Charging system malfunction'\n");
                signal_charge_stop_digital();
                state = state_error;
                break;
            }

            break;

        case E_CHARGE_INHIBIT_ENABLED:

            printf("Switching to state : charge_inhibited, reason : received charge_inhibit input signal\n");
            signal_charge_stop_digital();
            state = state_charge_inhibited;
            break;

        case E_STATION_LIVENESS_CHECK_FAILED:

            printf("Switching to state : error, reason : communication timeout with station\n");
            signal_charge_stop_digital();
            state = state_error;
            break;

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
void state_energy_transfer(Event event) {

    switch (event) {

        case E_BMS_UPDATE_RECEIVED:

            // Stop charging if the BMS says the battery is full
            if ( battery_is_full() ) {
                // Note : 'Battery Overvoltage' flag will also be set automatically here (102.4.0)
                printf("Switching to state : winding down, reason : battery full\n");
                signal_charge_stop_digital();
                state = state_winding_down;
                break;
            }

            // Stop charging if the BMS says the battery is too hot
            if ( battery_is_too_hot() ) {
                // Note : 'High Battery Temperature' flag will also be set automatically here (102.4.3)
                printf("Switching to state : charge_inhibited, reason : battery is too hot\n");
                signal_charge_stop_digital();
                state = state_winding_down;
            }

            recalculate_charging_current_request();

            break;

        case E_BMS_LIVENESS_CHECK_FAILED:

            printf("Switching to state : error, reason : communication timeout with BMS\n");
            state = state_error;

            break;

        /* This shouldn't be possible as the plug connector lock should be
         * engaged here, but deal with this scenario anyway for safety sake.
         */
        case E_PLUG_REMOVED:

            chademo_reinitialise();
            // FIXME contactors, etc.
            state = state_idle;
            break;

        case E_STATION_CAPABILITIES_UPDATED:

            // Current available at station may have changed
            recalculate_charging_current_request();

            // If current available has changed then the charging time may need updating
            recalculate_charging_time();

            break;

        case E_STATION_STATUS_UPDATED:

            // Station is signalling over CAN that it wants to stop charging
            if ( ! station_is_allowing_charge() ) {
                printf("Switching to state : winding_down, reason : station has requested charge termination\n");
                signal_charge_stop_digital();
                state = state_winding_down;
            }

            if ( station_is_reporting_station_malfunction() ) {
                printf("Switching to state : winding_down, reason : station reporting station malfunction (electrical, connector lock, or emergency stop button)\n");
                signal_charge_stop_digital();
                state = state_winding_down;
                break;
            }

            if ( station_is_reporting_battery_incompatibility() ) {
                printf("Switching to state : winding_down, reason : station reporting battery incompatiblity\n");
                signal_charge_stop_digital();
                state = state_winding_down;
                break;
            }

            if ( station_is_reporting_charging_system_malfunction() ) {
                printf("Switching to state : winding_down, reason : station reporting 'Charging system malfunction'\n");
                signal_charge_stop_digital();
                state = state_winding_down;
                break;
            }

            check_for_current_deviation_error();

            break;

        case E_CHARGE_INHIBIT_ENABLED:

            printf("Switching to state : winding_down, reason : received charge_inhibit input signal\n");
            signal_charge_stop_digital();
            state = state_winding_down;
            break;

        case E_STATION_LIVENESS_CHECK_FAILED:

            printf("Switching to state : error, reason : communication timeout with station\n");
            signal_charge_stop_digital();
            state = state_error;
            break;

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
void state_winding_down(Event event) {

    // No matter the event, always process the ramp down
    ramp_down_current_request();

    switch (event) {

        case E_BMS_UPDATE_RECEIVED:
            break;

        case E_BMS_LIVENESS_CHECK_FAILED:
            break;

        case E_STATION_CAPABILITIES_UPDATED:
            break;

        case E_STATION_STATUS_UPDATED:

            // Winding down is complete
            if ( station_get_current() <= TERMINATION_CURRENT ) {
                signal_charge_stop_discrete();
                //chademo.weldCheckPendingSwitchOn = false;
                inhibit_contactor_close();
                //chademo.weldCheckCycles = 0;
            }
            break;

        case E_PLUG_REMOVED:
            break;

        case E_CHARGE_INHIBIT_ENABLED:
            break;

        case E_STATION_LIVENESS_CHECK_FAILED:
            break;

        default:
            printf("WARNING : received invalid event [%s]\n", event);

    }

}


/*
 * State : weld_detection
 *
 * IN1/CP      : activated
 * IN2/CP2     : activated
 * OUT1/CP3    : deactivated
 * OUT2/cont   : activated
 * CS          : activated
 * Plug locked : yes
 *
 */
void state_weld_detection(Event event) {

    switch (event) {

        case E_BMS_UPDATE_RECEIVED:
            break;

        case E_BMS_LIVENESS_CHECK_FAILED:
            break;

        case E_STATION_CAPABILITIES_UPDATED:
            break;

        case E_STATION_STATUS_UPDATED:

            /*
            // We're waiting for the contactors to turn on
            if ( chademo.weldCheckPendingSwitchOn ) {
                if ( station_get_voltage() > ( bms.voltage * 0.9 ) ) {
                    //chademo.weldCheckPendingSwitchOn = false;
                    //chademo.weldCheckCycles += 1;
                }
            }

            // We're waiting for the contactors to turn off
            else {
                if ( station_get_voltage() < ( bms.voltage * 0.1 ) ) {
                    //chademo.weldCheckPendingSwitchOn = true;
                    //chademo.weldCheckCycles += 1;
                }
            }

            
            if ( chademo.weldCheckCycles > 3 ) {
                // 102.5.3 high
                state = state_plug_in;
            }
            */

            break;

        case E_PLUG_REMOVED:
            break;

        case E_CHARGE_INHIBIT_ENABLED:
            break;

        case E_STATION_LIVENESS_CHECK_FAILED:
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
 * Reasons we can be inhibited
 *   - battery is full
 *   - battery is too hot
 *   - battery is too cold (warming in progress)
 *   - CHARGE_INHIBIT signal from BMS has been activated
 *
 */
void state_charge_inhibited(Event event) {

    switch (event) {

        case E_BMS_UPDATE_RECEIVED:

            // Auxiliary CHARGE_INHIBIT check escape
            if ( ! battery_is_full() && ! battery_is_too_hot() && ! battery_is_too_cold() && ! charge_inhibit_enabled() ) {
                if ( plug_is_inserted() ) {
                    printf("Switching to state : plug_in, reason : aux charge_inhibit check passed\n");
                    state = state_plug_in;
                } else {
                    printf("Switching to state : idle, reason : aux charge_inhibit check passed\n");
                    state = state_idle;
                }
            }

            break;

        case E_BMS_LIVENESS_CHECK_FAILED:

            printf("Switching to state : error, reason : BMS liveness check failed\n");
            state = state_error;

            break;

        case E_PLUG_INSERTED:

            break;

        case E_CHARGE_INHIBIT_DISABLED:

            // We're no longer in an inhibit state, go back to idle or plug_in
            if ( ! battery_is_full() && ! battery_is_too_hot() && ! battery_is_too_cold() && ! charge_inhibit_enabled() ) {
                if ( plug_is_inserted() ) {
                    printf("Switching to state : plug_in, reason : charge_inhibit check passed\n");
                    state = state_plug_in;
                } else {
                    printf("Switching to state : idle, reason : charge_inhibit check passed\n");
                    state = state_idle;
                }
            }

            break;

        /* This event should only fire if we were charging and moved into an
         * inhibited state.
         */
        case E_STATION_LIVENESS_CHECK_FAILED:

            printf("Switching to state : error, reason : bms liveness check failed\n");
            state = state_error;

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
 * We can get into an error state when:
 *   - communication timeout with BMS
 *   - communication timeout with station
 *   - station compatability issue (voltage)
 *   - station reporting a fault
 */
void state_error(Event event) {

    switch (event) {

        case E_BMS_UPDATE_RECEIVED:
            break;

        // Only way out of error is to remove the plug and start over
        case E_PLUG_REMOVED:
            chademo_reinitialise();
            state = state_idle;
            break;

        default:
            printf("WARNING : received invalid event [%s]\n", event);

    }
}





