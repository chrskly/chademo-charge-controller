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

#ifndef WIFICONSOLE_H
#define WIFICONSOLE_H

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "picow_http/http.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"


class WifiConsole {
    private:
        struct server *srv;
        struct server_cfg cfg;
    public:
};

#endif