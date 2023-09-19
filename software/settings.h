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

// LED
#define PICO_DEFAULT_LED_PIN 99

#define CHARGE_INHIBIT_PIN 10 // pin 14

// Chademo pins
#define CHADEMO_IN1_PIN 14  // pin 19, CP  - contactor +ve, (sensed by 'f'), d1 enable signal, chademo plug pin 2
#define CHADEMO_IN2_PIN 15  // pin 20, CP2 - contactor -ve, (sensed by 'g'), d2 enable signal, chademo plug pin 10
#define CHADEMO_OUT1_PIN 12 // pin 16, CP3 - charge enable, (switched by 'k'), chademo plug pin 4
#define CHADEMO_OUT2_PIN 13 // pin 17, contactor relay control, 'e'

#define CHADEMO_CS_PIN 20   // pin 26, CS - pilot wire, input, (a.k.a 'h'), pilot signal, chademo plug pin 7

// Version of ChaDeMo protocol to use. 1 == v0.9, 2 == v1.0
#define CHADEMO_PROTOCOL_VERSION 1

// Messages from ChaDeMo station
#define EVSE_CAPABILITIES_MESSAGE_ID 0x108
#define EVSE_STATUS_MESSAGE_ID 0x109

// Spec says current requests from the car should only vary at a rate of +/- 20A/sec
#define CHADEMO_RAMP_RATE 20
#define CHADEMO_RAMP_INTERVAL 1000 // units = ms

// Messages from BMS

// max/min pack voltage, max dis/charge current
#define BMS_LIMITS_MESSAGE_ID 0x351

// soc
#define BMS_SOC_MESSAGE_ID 0x355

// pack voltage, current (not implemented), temp
#define BMS_STATUS_MESSAGE_ID 0x356

// Alarms/warnings from BMS
#define BMS_ALARM_MESSAGE_ID 0x35A

// If we don't see the BMS in this number of seconds, then something is wrong
#define BMS_TTL 5

// If we don't see the ChaDeMo station in this number of seconds, then something is wrong
#define CHADEMO_STATION_TTL 2

// This scaling factor is used to calculate max charging time from the estimated charging time
#define MAX_CHARGING_TIME_SCALING_FACTOR 1.2

//
// Battery settings
//

#define BATTERY_MAX_VOLTAGE 350         // fixme put in proper value

// The SoC at which to stop fast-charging. Can be overriden via CAN msg.
#define BATTERY_FAST_CHARGE_DEFAULT_SOC_MAX 80

#define BATTERY_MAX_CURRENT 10          // fixme put in proper value
#define BATTERY_MAX_CURRENT_FAILSAFE 10 // maximum current to use if we lose communication with the BMS

#endif
