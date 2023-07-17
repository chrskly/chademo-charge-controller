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

using namespace std;

#include <time.h>
#include <stdio.h>
#include <algorithm>

#include "chademostation.h"
#include "chademostatemachine.h"
#include "battery.h"
#include "util.h"

#include "settings.h"


extern ChademoState state;

ChademoStation::ChademoStation() {}

/*
 * When kicking off the charging process, we initialise these variables to zero
 * to ensure we get fresh values from the charging station. We may be restarting
 * a failed charge and have the old values lying around still.
 */
void ChademoStation::reinitialise() {
    controlProtocolNumber = 0;
    maximumVoltageAvailable = 0;
    availableCurrent = 0;
    vehicleConnectorLock = false;
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
bool ChademoStation::initial_parameter_exchange_complete() {
    return (
        controlProtocolNumber != 0 && 
        maximumVoltageAvailable != 0 && 
        availableCurrent != 0 && 
        ! batteryIncompatability
    );
}

// Mark the chademo station as seen right now
void ChademoStation::heartbeat() {
    lastUpdateFromEVSE = get_clock();
}

// Return true if station was seen recently enough to be considered alive
bool ChademoStation::is_alive() {
    return ( ((double)(get_clock() - lastUpdateFromEVSE) / CLOCKS_PER_SEC) < BMS_TTL );
}

// Return true if the connector plug is locked to the car
bool ChademoStation::connector_is_locked() {
    return vehicleConnectorLock;
}

bool ChademoStation::is_reporting_battery_incompatibility() {
    return batteryIncompatability;
}

bool ChademoStation::is_reporting_station_malfunction() {
    return stationMalfunction;
}

bool ChademoStation::is_reporting_charging_system_malfunction() {
    return chargingSystemMalfunction;
}

bool ChademoStation::station_is_allowing_charge() {
    return chargerStopControl;
}






