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


#include "led.h"

#include <stdio.h>
#include "pico/stdlib.h"

#include "types.h"

#include "settings.h"

int statusLEDcounter;

extern StatusLED led;


// Switch status light to a different mode
void led_set_mode(LED_MODE newMode) {
    printf("Setting LED mode %d\n", newMode);
    switch( newMode ) {
        case STANDBY:
            printf("Switch status light to mode STANDBY\n");
            led.onDuration = 1;
            led.offDuration = 39;
            break;
        case DRIVE:
            printf("Switch status light to mode DRIVE\n");
            led.onDuration = 20;
            led.offDuration = 0;
            break;
        case CHARGING:
            printf("Switch status light to mode CHARGING\n");
            led.onDuration = 10;
            led.offDuration = 10;
            break;
        case FAULT:
            printf("Switch status light to mode FAULT\n");
            led.onDuration = 1;
            led.offDuration = 1;
            break;
    }

}

void led_blink() {
    ++led.counter;

    if ( led.on ) {
        if ( led.counter > led.onDuration ) {
            led.counter = 0;
            if ( led.offDuration > 0 ) {
                gpio_put(PICO_DEFAULT_LED_PIN, 0);                
                led.on = false;
            }
        }
    } else {
        if ( led.counter > led.offDuration ) {
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            led.counter = 0;
            led.on = true;
        }
    }
}

bool process_led_blink_step(struct repeating_timer *t) {
    led_blink();
    return true;
}

struct repeating_timer ledBlinkTimer;

void enable_led_blink() {
    add_repeating_timer_ms(100, process_led_blink_step, NULL, &ledBlinkTimer);
}

