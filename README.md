# CHAdeMO Implementation

This repo contains a car-side implementation of the CHAdeMO EV charging
standard including both the hardware and firmware.

## Features

* Supports CHAdeMO protocols v0.9, v1.0, and v2.0.
* Automatic sleep, wake on plug insertion.
* Separate manual wake button option.

## Chademo Plug Pinout

 1. Ground
 2. CP : evse => car, IN1, contactor +ve signal, goes high when active, sensed by 'f'
 3. nc
 4. CP3 : car => evse, OUT1, charge enable, sensed by 'k'
 5. HV-
 6. HV+
 7. CS : connection check, goes low when plug inserted, sensed by 'h'
 8. CAN H
 9. CAN L
10. CP2 : evse => car, IN2, contactor -ve signal, goes low when active, sensed by 'g'

OUT2, contactor override, pull low to allow CP2 signal through

## Charging start sequence

1. Car pulls CP high (12V) through R3 (1kΩ) when plug is inserted
2. User presses start button on station
3. Station pulls CP high (IN1), d1 closes, +ve contactor closes
4. Car and station start sending CAN messages
5. Car and station check each others params for compatability
6. Car pulls CP3 low and sets CAN vehicleChargingEnabled flag to true to
   indicate it is ready to go ahead with charging
7. Station locks plug to vehicle
8. Station performs insulation test
9. Station pulls CP2 low (IN2), d2 closes, -v3 contactor closes
10. Car ramps up current request. Energy transfer begins.

## Charging stop sequence

1. Car stops pulling CP3 low and sets CAN vehicleChargingEnabled to false.
2. Station deactivates CP and CP2
3. Station unlocks plug

## Questions

- [ ] When to use vehicleChargingEnabled and when to use vehicleRequestingStop?

