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

#ifndef CHADEMOSTATEMACHINE_H
#define CHADEMOSTATEMACHINE_H

typedef enum {
    E_PLUG_INSERTED,
    E_PLUG_REMOVED,
    E_IN1_ACTIVATED,
    E_IN1_DEACTIVATED,
    E_IN2_ACTIVATED,
    E_IN2_DEACTIVATED,
    E_STATION_CAPABILITIES_UPDATED,
    E_STATION_STATUS_UPDATED,
    E_BMS_UPDATE_RECEIVED,
    E_CHARGE_INHIBIT_ENABLED,
    E_CHARGE_INHIBIT_DISABLED,
    E_BMS_LIVENESS_CHECK_FAILED,
    E_STATION_LIVENESS_CHECK_FAILED
} ChademoEvent;


typedef void (*ChademoState)(ChademoEvent);

void chademo_state_idle(ChademoEvent event);
void chademo_state_plug_in(ChademoEvent event);
void chademo_state_handshaking(ChademoEvent event);
void chademo_state_await_connector_lock(ChademoEvent event);
void chademo_state_await_insulation_test(ChademoEvent event);
void chademo_state_energy_transfer(ChademoEvent event);
void chademo_state_winding_down(ChademoEvent event);
void chademo_state_charge_inhibited(ChademoEvent event);
void chademo_state_error(ChademoEvent event);

#endif