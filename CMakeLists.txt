# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Datalogger_IMU C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

include_directories(${CMAKE_SOURCE_DIR}/lib/include) # Inclui os files .h na pasta lib
include_directories(${CMAKE_SOURCE_DIR}/lib/include/ssd1306) # Inclui os files .h na pasta ssd1306

add_subdirectory(lib/FatFs_SPI)    

add_executable(${PROJECT_NAME}  
        Datalogger_IMU.c
        lib/source/buzzer.c
        lib/source/ssd1306.c
        lib/source/hw_config.c
        lib/source/sdcard_fatFS.c
        lib/source/mpu6050.c
        )

# Adiciona o diretório raiz como include privado apenas para o target atual
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})  

# Add the standard library to the build
target_link_libraries(${PROJECT_NAME} 
        pico_stdlib
        FatFs_SPI
        hardware_gpio
        hardware_adc
        hardware_clocks
        hardware_i2c
        hardware_pwm
        hardware_irq
        hardware_pio
        hardware_timer
        pico_bootrom
        )

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${PROJECT_NAME}  1)
pico_enable_stdio_usb(${PROJECT_NAME}  1)

pico_add_extra_outputs(${PROJECT_NAME} )