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

#include <stdio.h>
#include <time.h>

#include "battery.h"
#include "util.h"

#include "settings.h"


Battery::Battery() {
    maximumVoltage = BATTERY_MAX_VOLTAGE;
    maximumChargeCurrent = BATTERY_MAX_CURRENT;
    chargingTimeMinutes = 0;
}


//
// BMS
//

void Battery::bms_heartbeat() {
    bmsLastUpdate = get_clock();
}

bool Battery::bms_is_alive() {
    return ( ((double)(get_clock() - bmsLastUpdate) / CLOCKS_PER_SEC) < BMS_TTL );
}


//
// Voltage
//

float Battery::get_target_voltage() {
    return targetVoltage;
}

// Do a quick and dirty calculation to get battery voltage corresponding to SoC
uint8_t Battery::get_voltage_from_soc(uint8_t soc) {
    if ( soc > 100 ) soc = 100;
    return ( ( maximumVoltage - minimumVoltage ) * soc ) + minimumVoltage;
}

/*
 * Based on our SoC and a given charge current, calculate how many minutes we
 * think it will take to charge to the specified SoC.
 *
 * FIXME : this is not correct
 */
void Battery::recalculate_charging_time_minutes(uint8_t current, uint8_t targetSoc) {

    uint16_t whRemaining = batteryCapacityWH - ( batteryCapacityWH * targetSoc );

    /* Use the average voltage between the current pack voltage and the pack
     * voltage at the target SoC to get a more accurate estimate.
     */
    uint32_t chargingWatts = current * get_voltage_from_soc( ( soc + targetSoc ) / 2 );

    chargingTimeMinutes = (uint8_t)( whRemaining / chargingWatts / 60 );
    chargingTimeMinutesMax = chargingTimeMinutes * MAX_CHARGING_TIME_SCALING_FACTOR;
}

uint8_t Battery::get_charging_time_minutes() {
    return chargingTimeMinutes;
}

uint8_t Battery::get_charging_time_minutes_max() {
    return chargingTimeMinutesMax;
}


bool Battery::is_full() {
    if ( highCellAlarm ) {
        return true;
    }
    if ( voltage >= BATTERY_MAX_VOLTAGE ) {
        return true;
    }
    return false;
}


