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

#include "battery.h"
#include "util.h"
#include "settings.h"
#include "chademostatemachine.h"
#include "types.h"

extern BMS bms;
extern Battery battery;
extern ChademoState state;

//
// BMS
//

void bms_heartbeat() {
    bms.lastHeartbeat = get_clock();
}

/*
 * Return true if we have seen a message from the BMS within the last BMS_TTL
 * seconds. If we lose communication with the BMS we should stop any charging
 * activity straight away.
 */
bool bms_is_alive() {
    return ( ((double)(get_clock() - bms.lastHeartbeat) / CLOCKS_PER_SEC) < BMS_TTL );
}

// Watch for no messages from BMS

struct repeating_timer bmsLivenessCheckTimer;

bool bms_liveness_check(struct repeating_timer *t) {
    if ( ! bms_is_alive() ) {
        state(E_BMS_LIVENESS_CHECK_FAILED);
    }
    return true;
}

void enable_bms_liveness_check() {
    add_repeating_timer_ms(1000, bms_liveness_check, NULL, &bmsLivenessCheckTimer);
}



//
// Voltage
//

float battery_get_target_voltage() {
    return battery.targetVoltage;
}

/*
 * Do a quick and dirty calculation to get battery voltage corresponding to SoC
 */
uint8_t battery_get_voltage_from_soc(uint8_t soc) {
    if ( bms.soc > 100 ) bms.soc = 100;
    return ( ( bms.maximumVoltage - bms.minimumVoltage ) * bms.soc ) + bms.minimumVoltage;
}

/*
 * Based on our SoC and a given charge current, calculate how many minutes we
 * think it will take to charge to the specified SoC.
 *
 * FIXME : this is not correct
 */
void battery_recalculate_charging_time_minutes(uint8_t current, uint8_t targetSoc) {

    uint16_t whRemaining = battery.capacityWH - ( battery.capacityWH * targetSoc );

    /* Use the average voltage between the current pack voltage and the pack
     * voltage at the target SoC to get a more accurate estimate.
     */
    uint32_t chargingWatts = current * battery_get_voltage_from_soc( ( bms.soc + targetSoc ) / 2 );

    battery.chargingTimeMinutes = (uint8_t)( whRemaining / chargingWatts / 60 );
    battery.chargingTimeMinutesMax = battery.chargingTimeMinutes * MAX_CHARGING_TIME_SCALING_FACTOR;
}

uint8_t get_charging_time_minutes() {
    return battery.chargingTimeMinutes;
}

uint8_t get_charging_time_minutes_max() {
    return battery.chargingTimeMinutesMax;
}


bool battery_is_full() {
    if ( bms.highCellAlarm ) {
        return true;
    }
    if ( bms.voltage >= BATTERY_MAX_VOLTAGE ) {
        return true;
    }
    return false;
}

bool battery_is_too_hot() {
    return bms.highTempAlarm;
}

bool battery_is_too_cold() {
    return bms.lowTempWarn || bms.lowTempAlarm;
}


