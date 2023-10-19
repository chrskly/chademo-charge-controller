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

#include <time.h>
#include <stdio.h>

#include "station.h"
#include "statemachine.h"
#include "util.h"
#include "settings.h"
#include "types.h"

extern Station station;
extern State state;


/*
 * Record the last time we saw a message from the charging station. If we don't
 * see one within CHADEMO_STATION_TTL timeframe, we've lost communications with
 * the charging station and must terminate the charing session.
 */

void station_heartbeat() {
    station.lastHeartbeat = get_clock();
}

bool station_is_alive() {
    return ( ((double)(get_clock() - station.lastHeartbeat) / CLOCKS_PER_SEC) < CHADEMO_STATION_TTL );
}

struct repeating_timer stationLivenessCheckTimer;

bool station_liveness_check(struct repeating_timer *t) {
    if ( ! station_is_alive() ) {
        state(E_STATION_LIVENESS_CHECK_FAILED);
    }
    return true;
}

void enable_station_liveness_check() {
    add_repeating_timer_ms(1000, station_liveness_check, NULL, &stationLivenessCheckTimer);
}


/*
 * When kicking off the charging process, we initialise these variables to zero
 * to ensure we get fresh values from the charging station. We may be restarting
 * a failed charge and have the old values lying around still.
 */
void reinitialise_station() {
    station.controlProtocolNumber = 0;
    station.maximumVoltageAvailable = 0;
    station.availableCurrent = 0;
    // FIXME check actual local status here
    station.vehicleConnectorLock = false;
}

/*
 * Have we received the initial params from the station?
 *
 * According to the spec the initial params we need to see from the station
 * before we can switch from B1->B2 are:
 *   - controlProtocolNumber
 *   - maximumVoltageAvailable
 *   - availableCurrent
 *   - batteryIncompatability
 */
bool initial_parameter_exchange_with_station_complete() {
    return (
        station.controlProtocolNumber != 0 && 
        station.maximumVoltageAvailable != 0 && 
        station.availableCurrent != 0 && 
        ! station.batteryIncompatability
    );
}



// Return true if the connector plug is locked to the car
bool connector_is_locked() {
    return station.vehicleConnectorLock;
}

bool station_is_reporting_battery_incompatibility() {
    return station.batteryIncompatability;
}

bool station_is_reporting_station_malfunction() {
    return station.stationMalfunction;
}

bool station_is_reporting_charging_system_malfunction() {
    return station.chargingSystemMalfunction;
}

bool station_is_allowing_charge() {
    return station.chargerStopControl;
}






