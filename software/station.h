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

#ifndef CHADEMOSTATION_H
#define CHADEMOSTATION_H

#include <stdbool.h>

void enable_station_liveness_check();
void reinitialise_station();
bool initial_parameter_exchange_with_station_complete();
void station_heartbeat();
bool station_is_alive();
bool connector_is_locked();
bool station_is_reporting_battery_incompatibility();
bool station_is_reporting_station_malfunction();
bool station_is_reporting_charging_system_malfunction();
bool station_is_allowing_charge();
uint16_t station_get_voltage();
uint8_t station_get_current();


#endif