cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(pico_sdk_import.cmake)

project(charger)

set(PICO_CXX_ENABLE_EXCEPTIONS 1)

pico_sdk_init()

add_executable(charger
        battery.cpp
        battery.h
        chademo.cpp
        chademo.h
        chademocomms.cpp
        chademocomms.h
        chademostatemachine.cpp
        chademostatemachine.h
        chademostation.cpp
        chademostation.h
        charger.cpp
        charger.h
        comms.cpp
        comms.h
        inputs.cpp
        inputs.h
        led.cpp
        led.h
        settings.h
        util.cpp
        util.h
        mcp2515/mcp2515.cpp
        )

pico_add_extra_outputs(charger)

target_link_libraries(charger pico_stdlib hardware_spi)
