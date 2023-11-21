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

#include <string.h>
#include <time.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "hardware/watchdog.h"

extern "C" {
    #include "led.h"
    #include "battery.h"
    #include "settings.h"
    #include "chademo.h"
    #include "statemachine.h"
    #include "dhcpserver.h"
    #include "dnsserver.h"
    #include "wifi.h"
}

#include "mcp2515/mcp2515.h"
#include "comms.h"
#include "chademocomms.h"

#include "charger.h"
#include "types.h"



MCP2515 mainCAN(SPI_PORT, MAIN_CAN_CS, SPI_MISO, SPI_MOSI, SPI_CLK, 500000);
MCP2515 chademoCAN(SPI_PORT, CHADEMO_CAN_CS, SPI_MISO, SPI_MOSI, SPI_CLK, 500000);


Charger charger;
State state;
Station station;
BMS bms;
Battery battery;
StatusLED led;
Chademo chademo;


// Watchdog

struct repeating_timer watchdogKeepaliveTimer;

bool watchdog_keepalive(struct repeating_timer *t) {
    watchdog_update();
    return true;
}

void enable_watchdog_keepalive() {
    add_repeating_timer_ms(5000, watchdog_keepalive, NULL, &watchdogKeepaliveTimer);
}


int main() {
    stdio_init_all();

    set_sys_clock_khz(80000, true);

    // set up the serial port
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    printf("Charger starting up ...\n");

    chademo_reinitialise();

    // Set up blinky LED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    led_set_mode(STANDBY);
    enable_led_blink();

    // 8MHz clock for CAN oscillator
    clock_gpio_init(CAN_CLK_PIN, CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_VALUE_CLK_SYS, 10);

    printf("Setting up main CAN port (BITRATE:%d:%d)\n", CAN_500KBPS, MCP_8MHZ);
    mainCAN.reset();
    mainCAN.setBitrate(CAN_500KBPS, MCP_8MHZ);
    mainCAN.setNormalMode();
    printf("Enabling handling of inbound CAN messages on main bus\n");
    enable_handle_main_CAN_messages();

    enable_bms_liveness_check();

    printf("Setting up Chademo CAN port (BITRATE:%d:%d)\n", CAN_500KBPS, MCP_8MHZ);
    chademoCAN.reset();
    chademoCAN.setBitrate(CAN_500KBPS, MCP_8MHZ);
    chademoCAN.setNormalMode();
    printf("Enabling handling of inbound CAN messages on chademo bus\n");
    enable_handle_chademo_CAN_messages();

    printf("Starting web server\n");
    TCP_SERVER_T *tcpState = new TCP_SERVER_T;

    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }

    const char *ap_name = "picow_test";
    const char *password = "password";

    cyw43_arch_enable_ap_mode(ap_name, password, CYW43_AUTH_WPA2_AES_PSK);

    ip4_addr_t mask;
    IP4_ADDR(ip_2_ip4(&tcpState->gw), 192, 168, 4, 1);
    IP4_ADDR(ip_2_ip4(&mask), 255, 255, 255, 0);

    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &tcpState->gw, &mask);

    if (!tcp_server_open(tcpState, ap_name)) {
        printf("failed to open server\n");
        return 1;
    }

    tcpState->complete = false;

    while(!tcpState->complete) {
        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));
    }

    return 0;
}

