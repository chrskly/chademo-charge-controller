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

using namespace std;

#include <time.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"

#include "mcp2515/mcp2515.h"
#include "settings.h"
#include "chademo.h"
#include "chademostatemachine.h"
#include "chademocomms.h"
#include "comms.h"
#include "led.h"
#include "battery.h"
#include "charger.h"

MCP2515 mainCAN(SPI_PORT, MAIN_CAN_CS, SPI_MISO, SPI_MOSI, SPI_CLK, 500000);
MCP2515 chademoCAN(SPI_PORT, CHADEMO_CAN_CS, SPI_MISO, SPI_MOSI, SPI_CLK, 500000);
MCP2515 cssCAN(SPI_PORT, CCS_CAN_CS, SPI_MISO, SPI_MOSI, SPI_CLK, 500000);

Charger charger;
StatusLight statusLight;

Charger::Charger(){
    chargeInhibited = true;
}


int main() {
    stdio_init_all();

    set_sys_clock_khz(80000, true);

    // set up the serial port
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    printf("Charger starting up ...\n");

    // Set up blinky LED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    statusLight.led_set_mode(STANDBY);
    enable_led_blink();

    // 8MHz clock for CAN oscillator
    clock_gpio_init(CAN_CLK_PIN, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_SYS, 10);

    printf("Setting up main CAN port (BITRATE:%d:%d)\n", CAN_500KBPS, MCP_8MHZ);
    mainCAN.reset();
    mainCAN.setBitrate(CAN_500KBPS, MCP_8MHZ);
    mainCAN.setNormalMode();
    printf("Enabling handling of inbound CAN messages on main bus\n");
    enable_handle_main_CAN_messages();

    printf("Setting up Chademo CAN port (BITRATE:%d:%d)\n", CAN_500KBPS, MCP_8MHZ);
    chademoCAN.reset();
    chademoCAN.setBitrate(CAN_500KBPS, MCP_8MHZ);
    chademoCAN.setNormalMode();
    printf("Enabling handling of inbound CAN messages on chademo bus\n");
    enable_handle_chademo_CAN_messages();

    while(true) {
    }

    return 0;
}

