cmake_minimum_required(VERSION 3.27.1)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(pico_sdk_import.cmake)

project(charger CXX C ASM)

set(PICO_BOARD pico_w)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

pico_sdk_init()

#add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/picow_http)

add_executable(charger
        dhcpserver.h
        dhcpserver.c
        lwipopts.h
        battery.c
        battery.h
        chademo.c
        chademo.h
        chademocomms.cpp
        chademocomms.h
        chademostatemachine.c
        chademostatemachine.h
        chademostation.c
        chademostation.h
        charger.cpp
        charger.h
        comms.cpp
        comms.h
        inputs.c
        inputs.h
        led.c
        led.h
        settings.h
        util.c
        util.h
        mcp2515/mcp2515.cpp
        )

target_include_directories(charger PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        )

target_compile_definitions(charger PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
)

target_link_libraries(charger
        pico_cyw43_arch_lwip_poll
        pico_stdlib
        hardware_spi
        )

#picow_http_gen_handlers(charger
#        ${CMAKE_CURRENT_SOURCE_DIR}/www.yaml
#        ${CMAKE_CURRENT_SOURCE_DIR}/www
#        ${CMAKE_CURRENT_SOURCE_DIR}/www/index.html
#        )

pico_add_extra_outputs(charger)
