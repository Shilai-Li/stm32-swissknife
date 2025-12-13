# Test Case Selection
set(TEST_CASE "UART" CACHE STRING "Select which test to run (BUZZER, CRSF, DELAY, DHT11, KEY, LED, MOTOR, MPU6050, SSD1306, SSD1306_CUSTOM, UART, W25QXX)" FORCE)
message("Selected Test Case: ${TEST_CASE}")

# Initialize source list
# You can add common files here that are ALWAYS needed
set(USER_SOURCES
        drivers/delay.c
        drivers/uart.c
)

# Applications
if (TEST_CASE STREQUAL "stm32f429HAL步进电机驱动")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE applications/2024-03/stm32f429HAL步进电机驱动/main.c)
    list(APPEND USER_SOURCES
            HARDWARE/KEY/key.c
            HARDWARE/KEY/led.c
            HARDWARE/TIMER/timer.c

            SYSTEM/delay/delay.c
            SYSTEM/sys/sys.c
            SYSTEM/usart/usart.c
    )

# Components
elseif (TEST_CASE STREQUAL "servo")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE components/tests/servo_test.c)
    list(APPEND USER_SOURCES
            components/servo.c
            components/servo_port.c
            drivers/motor.c
            middlewares/algorithms/pid.c
    )

# Drivers
elseif(TEST_CASE STREQUAL "buzzer")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/buzzer_driver_test.c)
    list(APPEND USER_SOURCES drivers/buzzer.c)
elseif (TEST_CASE STREQUAL "CRSF")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/crsf_protocol_test.c)
    list(APPEND USER_SOURCES middlewares/protocols/crsf.c)
elseif(TEST_CASE STREQUAL "delay")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/delay_driver_test.c)
    # Delay driver is already in common sources
elseif(TEST_CASE STREQUAL "dht11")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/dht11_driver_test.c)
    list(APPEND USER_SOURCES drivers/dht11.c)
elseif(TEST_CASE STREQUAL "key")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/key_driver_test.c)
    list(APPEND USER_SOURCES drivers/key.c)
elseif(TEST_CASE STREQUAL "led")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/led_driver_test.c)
    list(APPEND USER_SOURCES drivers/led.c)
elseif(TEST_CASE STREQUAL "motor")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/motor_driver_test.c)
elseif(TEST_CASE STREQUAL "mpu6050")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/mpu6050_driver_test.c)
    list(APPEND USER_SOURCES drivers/mpu6050.c)
elseif(TEST_CASE STREQUAL "SSD1306_CUSTOM")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/ssd1306_custom_tests.c)
    list(APPEND USER_SOURCES
            drivers/afiskon-stm32-ssd1306/ssd1306.c
            drivers/afiskon-stm32-ssd1306/ssd1306_fonts.c
            drivers/afiskon-stm32-ssd1306/ssd1306_custom.c
            drivers/afiskon-stm32-ssd1306/ssd1306_fonts_custom.c
    )
elseif(TEST_CASE STREQUAL "SSD1306")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/ssd1306_tests.c)
    list(APPEND USER_SOURCES
            drivers/afiskon-stm32-ssd1306/ssd1306.c
            drivers/afiskon-stm32-ssd1306/ssd1306_fonts.c
    )
elseif(TEST_CASE STREQUAL "UART")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/uart_driver_test.c)
    # UART driver is already in common sources
elseif(TEST_CASE STREQUAL "w25qxx")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/w25qxx_test.c)
    list(APPEND USER_SOURCES drivers/w25qxx.c)
else()
    # Default to CRSF if nothing matches
    message(STATUS "No specific TEST_CASE matched, defaulting to CRSF")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/Tests/crsf_protocol_test.c)
    list(APPEND USER_SOURCES middlewares/protocols/crsf.c)
endif()

# Create the library with the selected sources
add_library(User OBJECT ${USER_SOURCES})

# Include directories (kept global for simplicity, or can be conditional too)
target_include_directories(User PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/applications
        ${CMAKE_CURRENT_SOURCE_DIR}/components
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers
        ${CMAKE_CURRENT_SOURCE_DIR}/middlewares
)

target_link_libraries(User PUBLIC stm32cubemx)
