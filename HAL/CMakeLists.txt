cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME project)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Add User Drivers
add_subdirectory(User/Drivers)

# Test Case Selection
set(TEST_CASE "MOTOR" CACHE STRING "Select which test to run (CRSF, UART, BUZZER, DELAY, DHT11, KEY, LED, MOTOR, SSD1306, SSD1306_CUSTOM)")
message("Selected Test Case: ${TEST_CASE}")

if(TEST_CASE STREQUAL "CRSF")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE User/Drivers/Tests/crsf_protocol_test.c)
elseif(TEST_CASE STREQUAL "UART")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE User/Drivers/Tests/uart_driver_test.c)
elseif(TEST_CASE STREQUAL "BUZZER")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE User/Drivers/Tests/buzzer_driver_test.c)
elseif(TEST_CASE STREQUAL "DELAY")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE User/Drivers/Tests/delay_driver_test.c)
elseif(TEST_CASE STREQUAL "DHT11")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE User/Drivers/Tests/dht11_driver_test.c)
elseif(TEST_CASE STREQUAL "KEY")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE User/Drivers/Tests/key_driver_test.c)
elseif(TEST_CASE STREQUAL "LED")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE User/Drivers/Tests/led_driver_test.c)
elseif(TEST_CASE STREQUAL "MOTOR")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE User/Drivers/Tests/motor_driver_test.c)
elseif(TEST_CASE STREQUAL "SSD1306")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE User/Drivers/Tests/ssd1306_test.c)
elseif(TEST_CASE STREQUAL "SSD1306_CUSTOM")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE User/Drivers/Tests/ssd1306_custom_test.c)
else()
    # Default to CRSF if nothing matches, or you can leave it empty if you want to build nothing
    message(STATUS "No specific TEST_CASE matched, defaulting to CRSF")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE User/Drivers/Tests/crsf_protocol_test.c)
endif()

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
        User/App/Inc
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    UserDrivers
    # Add user defined libraries
)
