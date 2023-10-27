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

#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>

#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"


// Battery

typedef struct {
    float targetVoltage;  // Max voltage to charge to
    uint8_t chargingTimeMinutes;
    uint8_t chargingTimeMinutesMax;

    // Rated capacity of the battery.
    uint16_t capacityWH;
    uint16_t capacityAH;
} Battery;


// BMS

typedef struct {
    clock_t lastHeartbeat;            // Last time we heard from the BMS
    uint16_t soc;                     // Battery SoC
    float maximumVoltage;             // 'Full' battery voltage
    float maximumChargeCurrent;       // Maximum charge current allowed by BMS
    float maximumDischargeCurrent;    // Maximum discharge current allowed by BMS
    float minimumVoltage;             // 'Empty' battery voltage
    float voltage;                    // Actual voltage of battery right now
    float measuredVoltage;            // Voltage measured by the shunt
    float batteryCurrent;             // Current (amps) in/out of the battery right now
    float batteryTemperature;         // Temperature of hottest cell
    bool highCellAlarm;               // 
    bool lowCellAlarm;                // 
    bool highTempAlarm;               // 
    bool lowTempAlarm;                // 
    bool cellDeltaAlarm;              // 
    bool highCellWarn;                // 
    bool lowCellWarn;                 // 
    bool highTempWarn;                // 
    bool lowTempWarn;                 //   
} BMS;


// Charger

typedef struct {
    bool chargeInhibited;
} Charger;


// Station

typedef struct {

    clock_t lastHeartbeat;

    /*
     * Station capabilites message fields
     */

    bool weldDetectionSupported;
    uint16_t maximumVoltageAvailable;
    uint8_t availableCurrent;
    uint16_t thresholdVoltage; // evse reporting to car what it considers to be voltage to terminate charging

    /*
     * Station status message fields
     */

    uint8_t controlProtocolNumber;
    uint16_t outputVoltage;
    uint8_t outputCurrent;
    uint8_t timeRemainingSeconds;
    uint8_t timeRemainingMinutes;

    /* Indicates when station is outputting current.
     * false == no, true == yes.
     * False during insulation test.
     */
    bool stationStatus;

    /* Station reporting an error
     *   - Short circuit, earth fault in the charger
     *   - Malfunction of connector lock circuit
     *   - Emergency stop button pressed
     */
    bool stationMalfunction;

    // Status of whether plug is locked to vehicle
    bool vehicleConnectorLock;

    /* Voltage required by car higher than what the station can provide
     *   > we must disable CP3/OUT1 when true
     */
    bool batteryIncompatability;

    /* Fault which originates at the charger
     *   - DC short circuit, DC earth fault
     *   - Timeout
     *   - CAN reception error
     *   - Measured voltage < 50V after d2 on.
     *   - Car requesting more current than station can provide
     *   - On-board battery continues to apply voltage to the main circuit after the termination of charging.
     *   - DC voltage exceeds threshold voltage
     *   > we must disable CP3/OUT1 when true
     */
    bool chargingSystemMalfunction;

    /* Station wants to wind down the charging session.
     * true == don't charge, false == charge
     */
    bool chargerStopControl;
    
} Station;


// Termination condition

typedef enum {
    STOP_AT_SOC,
    STOP_AT_VOLTAGE,
    STOP_AT_TIME
} TerminationCondition;

// Chademo

typedef struct {
    /* This is the current (amps) we will request from the station. It will
     * vary dynamically throughout the charging process because of max ramp
     * rates, amps available at the charger, BMS limits, etc.
     */
    uint8_t chargingCurrentRequest;

    /*
     * The time we last changed chargingCurrentRequest. Used to manage ramping
     * request up/down.
     */
    clock_t lastCurrentRequestChange;

    // The battery voltage at which to stop charging
    float targetVoltage;

    /* This is the maximum battery voltage we communicate to the station in
     * message 100.4,5. Used by the station to set 'threshold voltage'. This
     * value should not be updated after switch k is turned on. I.e., after
     * vehicle gives go ahead signal. Value updated from BMS prior to that.
     */
    float maximumVoltage;

    // The SoC at which to stop charging.
    uint8_t targetSoc;

    // Flags that we send to the charget in the 0x102 message (vehicle status)
    bool vehicleChargingEnabled;     // 102.5.0
    bool vehicleNotInPark;           // 102.5.1
    bool vehicleChargingSystemFault; // 102.5.2
    bool vehicleRequestingStop;      // 102.5.3

    TerminationCondition terminationCondition;

    /* Use this during weld check to tell whether we're waiting for the
     * contactors to turn on or off.
     */
    bool weldCheckPendingSwitchOn;

    uint8_t weldCheckCycles;

    /* Track whether the "Present output current" and "Charging current request"
     * deviate by too much for too long.
     */
    bool currentDeviationError;
    clock_t currentDeviationErrorTimer;

    /* Track whether "Present output voltage" reported by the station and the
     * voltage measured by the bms (shunt) are within +/- 10V.
     */
    bool voltageDeviationError;
    clock_t voltageDeviationErrorTimer;

} Chademo;


// LED

typedef enum {
    STANDBY,
    DRIVE,
    CHARGING,
    FAULT
} LED_MODE;

typedef struct {
    bool on;
    int counter;
    int onDuration;
    int offDuration;
} StatusLED;




/*
// Web server

typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;
    bool complete;
    ip_addr_t gw;
    async_context_t *context;
} TCP_SERVER_T;


typedef struct TCP_CONNECT_STATE_T_ {
    struct tcp_pcb *pcb;
    int sent_len;
    char headers[128];
    char result[256];
    int header_len;
    int result_len;
    ip_addr_t *gw;
} TCP_CONNECT_STATE_T;
*/

#endif