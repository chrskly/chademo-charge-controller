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

## Charging sequence

- station pulls CS high?low? to indicate plug insertion
- station pulls CP high (IN1), d1 closes, +ve contactor closes
- Start CAN
- Station/car send params, compatability check
- car pulls CP3 low to indicate start permission
- connector lock
- station runs insulation test
- station pulls CP2 low (IN2), d2 closes
- << charging >>
- car sends current request constantly
- battery fills up, car sends zero current request
- car deactivates CP3, stop permission
- station deactivates CP and CP2
- weld detection?
- unlock


## Questions
target voltage in car status, should it be 'full' voltage? should it be voltage at the soc we hope to reach?


1 - Charger announces itself to vehicle via discrete signal
2 - Vehicle starts CAN communications with charger
3 - Vehicle announces it is ready to charge (CAN and discrete signal)
4 - Charger locks the connector and performs an insulation test
5 - Charger signals it is ready to receive current request (CAN and discrete signal)
6 - Vehicle closes battery contactors and requests charging current
7 - Vehicle sends a charge stop request (CAN and discrete signal)
8 - Vehicle opens battery contactors
9 - Charger unlocks the connector
10 - CAN communication is halted

