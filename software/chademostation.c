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

#include "chademostation.h"
#include "util.h"
#include "settings.h"
#include "types.h"

extern Station station;


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

// Mark the chademo station as seen right now
void station_heartbeat() {
    station.lastUpdateFromEVSE = get_clock();
}

// Return true if station was seen recently enough to be considered alive
bool station_is_alive() {
    return ( ((double)(get_clock() - station.lastUpdateFromEVSE) / CLOCKS_PER_SEC) < CHADEMO_STATION_TTL );
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






