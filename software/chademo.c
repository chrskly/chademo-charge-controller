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
#include <time.h>
#include "hardware/gpio.h"
#include "tgmath.h"

#include "chademo.h"
#include "util.h"
#include "battery.h"
#include "station.h"
#include "settings.h"

#include "types.h"

extern Chademo chademo;
extern Battery battery;
extern Station station;
extern BMS bms;

void chademo_reinitialise() {
    // Start at zero. We only update this from handshaking onward.
    chademo.chargingCurrentRequest = 0;

    // Vehicle status flags
    chademo.vehicleChargingEnabled = false;
    chademo.vehicleNotInPark = false;
    chademo.vehicleRequestingStop = false;

    // Set the target state-of-charge and voltage
    chademo.targetSoc = BATTERY_FAST_CHARGE_DEFAULT_SOC_MAX;
    //chademo.targetVoltage = battery_get_voltage_from_soc(BATTERY_FAST_CHARGE_DEFAULT_SOC_MAX);

    // Will be updated with value from BMS
    chademo.maximumVoltage = 0;
}


//
// Charging phases
//


// The output voltage of the station is below target (plus margin)
bool in_constant_current_window() {
    return ( ( station.outputVoltage - CC_CV_MARGIN ) < battery_get_target_voltage() );
}


// The output voltage of the station is within CC_CV_MARGIN of the target voltage
bool in_constant_voltage_window() {
    return (
        ( ( station.outputVoltage - CC_CV_MARGIN ) > battery_get_target_voltage() ) && \
        ( station.outputVoltage < battery_get_target_voltage() )
    );
}


//
// Voltage
//


float chademo_get_target_voltage() {
    return chademo.targetVoltage;
}

void chademo_update_max_voltage_value() {
    chademo.maximumVoltage = bms.maximumVoltage;
}

// Can the station provide enough voltage to charge out battery?
bool chademo_station_voltage_sufficient() {
    // FIXME should this be based on voltage at our target SOC?
    return ( bms.maximumVoltage < station.maximumVoltageAvailable );
}


//
// Current
//

/*
 * Calculate a new value for the charge current (chargingCurrentRequest) that we
 * are asking the station to provide us with. Factor in the limits the BMS is
 * requiring, the maximum current the station can provide right now, and the
 * maximum rate at which the the current request may change (ramp rate).
 */
void recalculate_charging_current_request() {

    clock_t now = get_clock();

    // Get the new limits from the BMS and station
    uint8_t chargingCurrentCeiling = fmin(bms.maximumChargeCurrent, station.availableCurrent);

    /*
     * If the output voltage being reported by the station is less than the
     * target voltage, then we're in the constant current phase. Just max out
     * the amps.
     */

    if ( in_constant_current_window() ) {

        // No change required. Hold at the current level.
        if ( chademo.chargingCurrentRequest == chargingCurrentCeiling ) {
            return;
        }

        // We need to ramp up the current request
        if ( chademo.chargingCurrentRequest < chargingCurrentCeiling ) {

            // Enough time has passed that we can bump up the current request
            if ( chademo.lastCurrentRequestChange > ( now + CHADEMO_RAMP_INTERVAL ) ) {
                // Next step would be more than max, so just increment by max
                if ( ( chargingCurrentCeiling - chademo.chargingCurrentRequest ) > CHADEMO_RAMP_RATE ) {
                    chademo.chargingCurrentRequest += CHADEMO_RAMP_RATE;
                    chademo.lastCurrentRequestChange = now;
                    return;
                }
                // Next step is less than max, so go directly
                else {
                    chademo.chargingCurrentRequest = chargingCurrentCeiling;
                    chademo.lastCurrentRequestChange = now;
                    return;
                }
            }

            // Need to wait longer before we can bump up further
            else {
                return;
            }
        }

        // We need to ramp down the current requests
        else if ( chademo.chargingCurrentRequest > chargingCurrentCeiling ) {

            // Enough time has passed that we can reduce the current request
            if ( chademo.lastCurrentRequestChange > ( now + CHADEMO_RAMP_INTERVAL ) ) {
                // Next step would be more than max, so just decrement by max
                if ( ( chademo.chargingCurrentRequest - chargingCurrentCeiling ) > CHADEMO_RAMP_RATE ) {
                    chademo.chargingCurrentRequest -= CHADEMO_RAMP_RATE;
                    chademo.lastCurrentRequestChange = now;
                    return;
                }
                // Next step is less than max, so go directly
                else {
                    chademo.chargingCurrentRequest = chargingCurrentCeiling;
                    chademo.lastCurrentRequestChange = now;
                    return;
                }
            }

            // Need to wait longer before we can reduce further
            else {
                return;
            }
        }
    }

    /*
     * If the output voltage reported by the station is at the target voltage
     * then we're in the constant voltage phase.
     *
     * Here we're aiming to keep the voltage at the target voltage by adjusting
     * the current request.
     */

    else if ( in_constant_voltage_window() ) {

        // We need to ramp down the current requests
        if ( chademo.chargingCurrentRequest > chargingCurrentCeiling ) {

            // Enough time has passed that we can reduce the current request
            if ( chademo.lastCurrentRequestChange > ( now + CHADEMO_RAMP_INTERVAL ) ) {
                // Next step would be more than max, so just decrement by max
                if ( ( chademo.chargingCurrentRequest - chargingCurrentCeiling ) > CHADEMO_RAMP_RATE ) {
                    chademo.chargingCurrentRequest -= CHADEMO_RAMP_RATE;
                    chademo.lastCurrentRequestChange = now;
                    return;
                }
                // Next step is less than max, so go directly
                else {
                    chademo.chargingCurrentRequest = chargingCurrentCeiling;
                    chademo.lastCurrentRequestChange = now;
                    return;
                }
            }

            // Need to wait longer before we can reduce further
            else {
                return;
            }
        }

        // FIXME any reason to increase here?

        return;

    }

    /*
     * If the output voltage reported by the station is over the target voltage
     * then ramp down the amps until we get back to the target voltage;.
     */

    else {
        chademo.chargingCurrentRequest--;
    }

}

/*
 * We're at the end of charging. Ramp down the current request all the way to
 * zero at the normal rate.
 */
void ramp_down_current_request() {
    clock_t now = get_clock();
    if ( chademo.lastCurrentRequestChange > ( now + CHADEMO_RAMP_INTERVAL ) ) {
        chademo.chargingCurrentRequest -= CHADEMO_RAMP_RATE;
    }
}

uint8_t get_charging_current_request() {
    return chademo.chargingCurrentRequest;
}


//
// Time
//

void recalculate_charging_time() {
    if ( strcmp(CALCULATE_TIME_REMAINING_BASED_ON, "wh") == 0 ) {
        battery_recalculate_charging_time_minutes_by_wh(chademo.chargingCurrentRequest, chademo.targetSoc);
    } else {
        battery_recalculate_charging_time_minutes_by_ah(chademo.chargingCurrentRequest, chademo.targetSoc);
    }
}


/*
 * Generate byte 4 (battery status) sent in the car status message.
 *
 * bit 0 : battery over voltage. 0:normal, 1:fault
 * bit 1 : battery under voltage. 0:normal, 1:fault
 * bit 2 : Battery current deviation error. 0:normal, 1:fault
 * bit 3 : High battery temperature. 0:normal, 1:fault
 * bit 4 : Battery voltage deviation error. 0:normal, 1:fault
 *
 * FIXME : current deviation error
 * 
 */
uint8_t generate_battery_status_byte() {
    return (
        0x00 |
        bms.highCellAlarm |
        bms.lowCellAlarm << 1 |
        chademo.currentDeviationError << 2 |
        bms.highTempAlarm << 3 |
        chademo.voltageDeviationError << 4
    );
}

/*
 * Generate byte 5 (vehicle status) sent in the car status message.
 *
 * bit 0 : Vehicle charging enabled. 0:disabled, 1:enabled
 * bit 1 : Vehicle shift lever position. 0:Parking, 1:other
 * bit 2 : Charging system fault. 0:normal, 1:fault
 * bit 3 : Vehicle contactor status. 0:closed (or weld detect running), 1: open (or weld detect finished)
 * bit 4 : Normal stop request before charging. 0:no request, 1:request to stop
 *
 */
uint8_t generate_vehicle_status_byte() {
    return (
        0x00 |
        chademo.vehicleChargingEnabled |           // "charge permission"
        chademo.vehicleNotInPark << 1 |
        charging_system_fault_present() << 2 |     // "Charging system fault"
        contactors_are_allowed_to_close() << 3 |   // "vehicle status"
        chademo.vehicleRequestingStop << 4
    );
}





// Return true if plug is inserted in car.
bool plug_is_in() {
    return ( gpio_get(CHADEMO_CS_PIN) == 0 );
}

// Return true if IN1 signal is active (high)
bool in1_is_active() {
    return ( gpio_get(CHADEMO_IN1_PIN) == 1 );
}

// Return true if IN2 signal is active (low)
bool in2_is_active() {
    return ( gpio_get(CHADEMO_IN2_PIN) == 0 );
}

bool contactors_are_closed() {
    return ( in1_is_active() && in2_is_active() && contactors_are_allowed_to_close() );
}

// OUT1 (CP3)

void activate_out1() {
    gpio_put(CHADEMO_OUT1_PIN, 1);
}

void deactivate_out1() {
    gpio_put(CHADEMO_OUT1_PIN, 0);
}

bool out1_is_active() {
    return ( gpio_get(CHADEMO_OUT1_PIN) == 0 );
}

// OUT2 (contactor relay)

void permit_contactor_close() {
    gpio_put(CHADEMO_OUT2_PIN, 1);
}

void inhibit_contactor_close() {
    gpio_put(CHADEMO_OUT2_PIN, 0);
}

bool contactors_are_allowed_to_close() {
    return ( gpio_get(CHADEMO_OUT2_PIN) == 1 );
}

/*
 * Car signals go ahead to start charging
 */

void signal_charge_go_ahead_digital() {
    // charge go ahead signal, 102.5.0
    chademo.vehicleChargingEnabled = true;
}

void signal_charge_go_ahead_discrete() {
    // Enable the vehicle 'charge enable' digital signal, a.k.a CP3
    activate_out1();
}


/*
 * Car wants to initiate a shut down of the charging process.
 */

void signal_charge_stop_digital() {
    // charge go ahead signal, 102.5.0
    chademo.vehicleChargingEnabled = false;
}

void signal_charge_stop_discrete() {
    // Disable the vehicle 'charge enable' digital signal, a.k.a CP3
    deactivate_out1();
}


/*
 * Error checking
 */


/* Compare the current requested with the current delivered (as reported by the
 * station). If it deviates by too much for too long, then set a flag to report
 * an error (102.4.2).
 */
void check_for_current_deviation_error() {

    // CHAdeMO version before v0.9
    if ( station.controlProtocolNumber == 0 ) {
        // If we're getting 12A more than we ask for, that's an error state
        if ( station.outputCurrent < ( chademo.chargingCurrentRequest + 12 ) ) {
            // reset the counter
            chademo.currentDeviationErrorTimer = get_clock();
        }
    }

    // CHAdeMO versions from v0.9 and up
    else if ( station.controlProtocolNumber > 0 ) {
        // If we're getting 12A more or less than we ask for, that's an error state
        if ( station.outputCurrent < ( chademo.chargingCurrentRequest + 12 ) && 
             station.outputCurrent > ( chademo.chargingCurrentRequest - 12 )) {
            // reset the counter
            chademo.currentDeviationErrorTimer = get_clock();
        }
    }

    // Current has deviated for more than 5 seconds, trigger error state
    if ( ( chademo.currentDeviationErrorTimer - get_clock() ) > 1000 ) {
        chademo.currentDeviationError = true;
    } else {
        chademo.currentDeviationError = false;
    }

}

/* Compare the voltage the that charger says its outputting with the voltage
 * that we actually measure. If it deviates by +/- 10V for more than 5 seconds
 * then that's an error.
 */
void check_for_voltage_deviation_error() {
    // Reset the counter
    if ( ( bms.measuredVoltage < ( station.outputVoltage + 10 ) ) &&
         ( bms.measuredVoltage > ( station.outputVoltage - 10 ) ) ) {
        chademo.voltageDeviationErrorTimer = get_clock();
    }
    // Flag the fault if the deviation has been happening for > 5 secs
    if ( chademo.voltageDeviationErrorTimer < ( get_clock() - 5000 ) ) {
        chademo.voltageDeviationError = true;
    }
}

/* Return true if any of the 102.5.2 faults are occurring. They are:
 *   - Station timed-out doing something
 *   - Timeout receiving data from station over CAN
 *   - Any other fault originating in the station detected by the vehicle
 *   - Any other fault originating in the vehicle detected by the vehicle
 */
bool charging_system_fault_present() {
    return ( !station_is_alive() || !bms_is_alive() );
}
