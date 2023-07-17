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


#ifndef CHADEMO_H
#define CHADEMO_H

#define CC_CV_MARGIN 2

#include "chademostatemachine.h"
#include "chademostation.h"

class Chademo {

    private:

        // This is the current (amps) we will request from the station. It will
        // vary dynamically throughout the charging process because of max ramp
        // rates, amps available and charger, BMS limits, etc.
        uint8_t chargingCurrentRequest;

        // The time we last changed our current request we send to the station
        clock_t lastCurrentRequestChange;

        // The battery voltage at which to stop charging
        float targetVoltage;

        // The SoC at which to stop charging.
        uint8_t targetSoc;

        // Flags that are sent in the 0x102 message (vehicle status)
        bool vehicleChargingEnabled;
        bool vehicleNotInPark;
        bool vehicleChargingSystemFault;
        bool vehicleRequestingStop;

    public:

        ChademoState state;
        ChademoStation station;

        Chademo();

        void reinitialise();

        bool car_and_station_protocol_compatible();
        bool in_constant_current_window();
        bool in_constant_voltage_window();
        float get_target_voltage();
        bool station_voltage_sufficient();

        void recalculate_charging_current_request();
        void ramp_down_current_request();
        uint8_t get_charging_current_request();
        void recalculate_charging_time();

        uint8_t generate_battery_status_byte();
        uint8_t generate_vehicle_status_byte();

        bool plug_is_in();
        bool in1_is_active();
        bool in2_is_active();

        bool contactors_are_closed();

        void activate_out1();
        void deactivate_out1();
        bool out1_is_active();
        void activate_out2();
        void deactivate_out2();
        bool out2_is_active();

        void permit_contactor_close();
        void inhibit_contactor_close();

        void signal_charge_go_ahead();
        void initiate_shutdown();

        bool error_condition();

};

#endif