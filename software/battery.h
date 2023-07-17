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

class Battery {

    private:
        // Voltage to charge up to
        float targetVoltage;

        // Estimated time remaining until charging is complete. We calculate
        // this as we go along. Sent to station.
        uint8_t chargingTimeMinutes;
        uint8_t chargingTimeMinutesMax;

        // BMS
        clock_t bmsLastUpdate;

    public:
        // Below are all received from BMS
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

        // BMS
        void bms_heartbeat();
        bool bms_is_alive();
        float get_target_voltage();

        // This is the current (amps) to request from the station. This value
        // only factors in any limits placed on us by the BMS.
        uint8_t targetChargingCurrent;

        // Rated capacity of the battery.
        uint16_t batteryCapacityWH;
        uint16_t batteryCapacityAH;

        Battery();
        void recalculate_charging_time_minutes(uint8_t current, uint8_t targetSoc);
        uint8_t get_voltage_from_soc(uint8_t soc);
        uint8_t get_charging_time_minutes();
        uint8_t get_charging_time_minutes_max();

        bool is_full();

};

#endif