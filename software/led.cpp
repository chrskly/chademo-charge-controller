/*
 * This file is part of the ev mustang bms project.
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

#include "led.h"


// Switch status light to a different mode
void StatusLight::led_set_mode(LED_MODE newMode) {
    printf("Setting LED mode %d\n", newMode);
    switch( newMode ) {
        case STANDBY:
            printf("Switch status light to mode STANDBY\n");
            LEDonDuration = 1;
            LEDoffDuration = 39;
            break;
        case DRIVE:
            printf("Switch status light to mode DRIVE\n");
            LEDonDuration = 20;
            LEDoffDuration = 0;
            break;
        case CHARGING:
            printf("Switch status light to mode CHARGING\n");
            LEDonDuration = 10;
            LEDoffDuration = 10;
            break;
        case FAULT:
            printf("Switch status light to mode FAULT\n");
            LEDonDuration = 1;
            LEDoffDuration = 1;
            break;
    }

}

void StatusLight::led_blink() {
    ++LEDcounter;

    if ( LEDon ) {
        if ( LEDcounter > LEDonDuration ) {
            LEDcounter = 0;
            if ( LEDoffDuration > 0 ) {
                gpio_put(PICO_DEFAULT_LED_PIN, 0);                
                LEDon = false;
            }
        }
    } else {
        if ( LEDcounter > LEDoffDuration ) {
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            LEDcounter = 0;
            LEDon = true;
        }
    }
}

bool process_led_blink_step(struct repeating_timer *t) {
    extern StatusLight statusLight;
    statusLight.led_blink();
    return true;
}

struct repeating_timer ledBlinkTimer;

void enable_led_blink() {
    add_repeating_timer_ms(100, process_led_blink_step, NULL, &ledBlinkTimer);
}

