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
#include "pico/stdlib.h"

#include "mcp2515/mcp2515.h"

extern "C" {
#include "settings.h"
#include "battery.h"
#include "statemachine.h"
}

#include "types.h"

extern MCP2515 mainCAN;
extern ChademoState state;
extern BMS bms;

struct can_frame mainCANInboundFrame;
struct repeating_timer handleMainCANMessageTimer;

/*
 * Process inbound messages on the main CANbus
 */
bool handle_main_CAN_message(struct repeating_timer *t) {

    if ( mainCAN.readMessage(&mainCANInboundFrame) == MCP2515::ERROR_OK ) {

        switch ( mainCANInboundFrame.can_id ) {

            case BMS_LIMITS_MESSAGE_ID:
                bms.maximumVoltage = ( mainCANInboundFrame.data[0] | mainCANInboundFrame.data[1] << 8 ) / 10;
                bms.maximumChargeCurrent = ( mainCANInboundFrame.data[2] | mainCANInboundFrame.data[3] << 8 ) / 10;
                bms.maximumDischargeCurrent = ( mainCANInboundFrame.data[4] | mainCANInboundFrame.data[5] << 8 ) / 10;
                bms.minimumVoltage = ( mainCANInboundFrame.data[6] | mainCANInboundFrame.data[7] << 8 ) / 10;
                state(E_BMS_UPDATE_RECEIVED);
                bms_heartbeat();
                break;

            case BMS_SOC_MESSAGE_ID:
                bms.soc = mainCANInboundFrame.data[0] | mainCANInboundFrame.data[1] << 8;
                state(E_BMS_UPDATE_RECEIVED);
                bms_heartbeat();
                break;

            case BMS_STATUS_MESSAGE_ID:            
                bms.voltage = ( mainCANInboundFrame.data[0] | mainCANInboundFrame.data[1] << 8 ) / 100;
                bms.batteryCurrent = ( mainCANInboundFrame.data[2] | mainCANInboundFrame.data[3] << 8 ) / 10;
                bms.batteryTemperature = ( mainCANInboundFrame.data[4] | mainCANInboundFrame.data[5] << 8 ) / 10;
                bms.measuredVoltage = ( mainCANInboundFrame.data[6] | mainCANInboundFrame.data[7] << 8 ) / 100;
                state(E_BMS_UPDATE_RECEIVED);
                bms_heartbeat();
                break;

            case BMS_ALARM_MESSAGE_ID:
                bms.highCellAlarm = (mainCANInboundFrame.data[0] & ( 1 << 2 )) >> 2;
                bms.lowCellAlarm = (mainCANInboundFrame.data[0] & ( 1 << 4 )) >> 4;
                bms.highTempAlarm = (mainCANInboundFrame.data[0] & ( 1 << 6 )) >> 6;
                bms.lowTempAlarm = mainCANInboundFrame.data[1];
                bms.cellDeltaAlarm = mainCANInboundFrame.data[3];
                bms.highCellWarn = (mainCANInboundFrame.data[4] & ( 1 << 2 )) >> 2;
                bms.lowCellWarn = (mainCANInboundFrame.data[4] & ( 1 << 4 )) >> 4;
                bms.highTempWarn = (mainCANInboundFrame.data[4] & ( 1 << 6 )) >> 6;
                bms.lowTempWarn = mainCANInboundFrame.data[5];
                state(E_BMS_UPDATE_RECEIVED);
                bms_heartbeat();
                break;

            default:
                break;

        }
    }

    return true;
}

void enable_handle_main_CAN_messages() {
    add_repeating_timer_ms(10, handle_main_CAN_message, NULL, &handleMainCANMessageTimer);
}

