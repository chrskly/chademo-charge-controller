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

#include <stdbool.h>


#ifndef CHADEMO_H
#define CHADEMO_H

#define CC_CV_MARGIN 2

void chademo_reinitialise();
bool in_constant_current_window();
bool in_constant_voltage_window();
float chademo_get_target_voltage();
bool chademo_station_voltage_sufficient();
void recalculate_charging_current_request();
void ramp_down_current_request();
uint8_t get_charging_current_request();
void recalculate_charging_time();
uint8_t generate_battery_status_byte();
uint8_t generate_vehicle_status_byte();
bool contactors_are_closed();
void activate_out1();
void deactivate_out1();
bool out1_is_active();
void permit_contactor_close();
void inhibit_contactor_close();
bool contactors_are_allowed_to_close();
void signal_charge_go_ahead();
void initiate_shutdown();
bool error_condition();

#endif