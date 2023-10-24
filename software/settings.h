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

#ifndef SETTINGS_H
#define SETTINGS_H

#define VERSION 1.0

// Serial port
#define UART_ID      uart0
#define BAUD_RATE   115200
#define UART_TX_PIN      0 // pin 1
#define UART_RX_PIN      1 // pin 2

// SPI
#define SPI_PORT      spi0
#define SPI_MISO        16 // pin 21
#define SPI_CLK         18 // pin 24
#define SPI_MOSI        19 // pin 25
#define CAN_CLK_PIN     21 // pin 27
#define MAIN_CAN_CS     17 // pin 22
#define CHADEMO_CAN_CS  20 // pin 26

// Inputs
#define PROX_PIN 11
#define CHADEMO_IN1_PIN 14  // pin 19, CP  - contactor +ve, (sensed by 'f'), d1 enable signal, chademo plug pin 2
#define CHADEMO_IN2_PIN 15  // pin 20, CP2 - contactor -ve, (sensed by 'g'), d2 enable signal, chademo plug pin 10
#define CHADEMO_CS_PIN 20   // pin 26, CS - pilot wire, input, (a.k.a 'h'), pilot signal, chademo plug pin 7

// Outputs
#define CHARGE_ENABLE_PIN 9     // pin 12
#define CHARGE_INHIBIT_PIN 10   // pin 14
#define CHADEMO_OUT1_PIN 12     // pin 16, CP3 - charge enable, (switched by 'k'), chademo plug pin 4
#define CHADEMO_OUT2_PIN 13     // pin 17, contactor relay control, 'e'
#define PICO_DEFAULT_LED_PIN 99 //


/*
 * General CHAdemMO settings
 */

/* Version of CHADeMO protocol that the vehicle will announce to the station
 * 0 == Before 0.9
 * 1 == v0.9 and v0.9.1
 * 2 == v1.0.0 and v1.0.1
 * 3 == v2.0.0 and v2.0.1
 */
#define CHADEMO_PROTOCOL_VERSION 1

// Messages from ChaDeMo station
#define EVSE_CAPABILITIES_MESSAGE_ID 0x108
#define EVSE_STATUS_MESSAGE_ID 0x109

// Spec says current requests from the car should only vary at a rate of +/- 20A/sec
#define CHADEMO_RAMP_RATE 20
#define CHADEMO_RAMP_INTERVAL 1000 // units = ms

// If we don't receive a CAN message from the ChaDeMo station in this number of
// seconds, then we must abort charging.
#define CHADEMO_STATION_TTL 2000 // units = ms

// This scaling factor is used to calculate max charging time from the estimated charging time
#define MAX_CHARGING_TIME_SCALING_FACTOR 1.2

/* Specify whether to use amp-hours (ah) or watt-hours (wh) to estimate how much
 * time is left to complete charging.
 */
#define CALCULATE_TIME_REMAINING_BASED_ON 'ah'

/* The energy transfer stage is complete when the current drops below this value.
 */
#define TERMINATION_CURRENT 5

/* How long we wait for the contactors to open/close when we're doing weld
 * detection. If it takes longer than this, we consider the contactors welded.
 */
#define WELD_CHECK_TTL 1000 // units = ms


/*
 * General BMS settings
 */

// If we don't receive a CAN message from the BMS in this number of seconds,
// then we must abort charging.
#define BMS_TTL 5000 // units = ms

// BMS CAN message which contains max/min pack voltage, max dis/charge current
#define BMS_LIMITS_MESSAGE_ID 0x351

// BMS CAN messages which contains SoC
#define BMS_SOC_MESSAGE_ID 0x355

// BMS CAN message which contains pack voltage, current (not implemented), temp
#define BMS_STATUS_MESSAGE_ID 0x356

// BMS CAN message which contains alarms/warnings from BMS
#define BMS_ALARM_MESSAGE_ID 0x35A


/*
 * Battery settings
 */

// The SoC at which to stop fast-charging. Can be overriden via CAN msg.
#define BATTERY_FAST_CHARGE_DEFAULT_SOC_MAX 80

#define BATTERY_MAX_CURRENT 10          // fixme put in proper value
#define BATTERY_MAX_CURRENT_FAILSAFE 10 // maximum current to use if we lose communication with the BMS

// Stop charging when current drops to this level
#define TERMINATE_CHARGING_CURRENT 5

#endif
