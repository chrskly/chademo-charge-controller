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

#ifndef BATTERY_H
#define BATTERY_H

#include <stdbool.h>

void bms_heartbeat();
bool bms_is_alive();
void enable_bms_liveness_check();
float battery_get_target_voltage();
uint8_t battery_get_voltage_from_soc(uint8_t soc);
void battery_recalculate_charging_time_minutes(uint8_t current, uint8_t targetSoc);
uint8_t get_charging_time_minutes();
uint8_t get_charging_time_minutes_max();
bool battery_is_full();
bool battery_is_too_hot();
bool battery_is_too_cold();


#endif