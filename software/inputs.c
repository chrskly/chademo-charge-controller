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


#include <stdio.h>

#include "hardware/gpio.h"

#include "chademostatemachine.h"
#include "settings.h"


/*
 * IN1 (a.k.a 'f'), d1 enable signal, plug pin 2
 * IN2 (a.k.a 'g'), d2 enable signal, plug pin 10
 * CS  (a.k.a 'h'), pilot signal, plug pin 7
 */

void gpio_callback(uint gpio, uint32_t events) {
    extern ChademoState chademoState;

    if ( gpio == CHADEMO_IN1_PIN ) {
        if ( gpio_get(CHADEMO_IN1_PIN) == 1 ) {
            chademoState(E_IN1_ACTIVATED);
        } else {
            chademoState(E_IN1_DEACTIVATED);
        }
    }

    if ( gpio == CHADEMO_IN2_PIN ) {
        if ( gpio_get(CHADEMO_IN2_PIN) == 0 ) {
            chademoState(E_IN2_ACTIVATED);
        } else {
            chademoState(E_IN2_DEACTIVATED);
        }
    }

    if ( gpio == CHADEMO_CS_PIN ) {
        if ( gpio_get(CHADEMO_CS_PIN) == 0 ) {
            chademoState(E_PLUG_INSERTED);
        } else {
            chademoState(E_PLUG_REMOVED);
        }
    }

    if ( gpio == CHARGE_INHIBIT_PIN ) {
        if ( gpio_get(CHARGE_INHIBIT_PIN) == 0 ) {
            chademoState(E_CHARGE_INHIBIT_ENABLED);
        } else {
            chademoState(E_CHARGE_INHIBIT_DISABLED);
        }
    }

}

void enable_listen_for_IN1_signal() {
    gpio_set_irq_enabled_with_callback(CHADEMO_IN1_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
}

void enable_listen_for_IN2_signal() {
    gpio_set_irq_enabled(CHADEMO_IN2_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}

void enable_listen_for_CS_signal() {
    gpio_set_irq_enabled(CHADEMO_CS_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
}

