# Test Case Selection
set(TEST_CASE "servo" CACHE STRING "Select which test to run (BUZZER, CRSF, DELAY, DHT11, KEY, LED, MOTOR, MPU6050, SSD1306, SSD1306_CUSTOM, UART, W25QXX)" FORCE)
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
            drivers/dc_motor.c
            middlewares/algorithms/pid.c
    )

# Drivers
elseif(TEST_CASE STREQUAL "at24cxx")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/at24cxx_tests.c)
    list(APPEND USER_SOURCES drivers/at24cxx.c)
elseif(TEST_CASE STREQUAL "adc")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/adc_tests.c)
elseif(TEST_CASE STREQUAL "buzzer")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/buzzer_driver_test.c)
    list(APPEND USER_SOURCES drivers/buzzer.c)
elseif(TEST_CASE STREQUAL "can")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/can_tests.c)
elseif (TEST_CASE STREQUAL "CRSF")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/crsf_protocol_test.c)
    list(APPEND USER_SOURCES middlewares/protocols/crsf.c)
elseif(TEST_CASE STREQUAL "delay")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/delay_driver_test.c)
    # Delay driver is already in common sources
elseif(TEST_CASE STREQUAL "dht11")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/dht11_driver_test.c)
    list(APPEND USER_SOURCES drivers/dht11.c)
elseif(TEST_CASE STREQUAL "encoder")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/encoder_tests.c)
elseif(TEST_CASE STREQUAL "hc_sr04")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/hc_sr04_tests.c)
elseif(TEST_CASE STREQUAL "ili9341")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/ili9341_tests.c)
    list(APPEND USER_SOURCES drivers/ili9341.c)
elseif(TEST_CASE STREQUAL "ili9488")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/ili9488_tests.c)
    list(APPEND USER_SOURCES drivers/ili9488.c)
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
elseif(TEST_CASE STREQUAL "rs485")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/rs485_tests.c)
elseif(TEST_CASE STREQUAL "ssd1306_custom")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/ssd1306_custom_tests.c)
    list(APPEND USER_SOURCES
            drivers/afiskon-stm32-ssd1306/ssd1306.c
            drivers/afiskon-stm32-ssd1306/ssd1306_fonts.c
            drivers/ssd1306_custom.c
            drivers/ssd1306_fonts_custom.c
    )
elseif(TEST_CASE STREQUAL "sd_card_spi")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/sd_card_spi_tests.c)
    list(APPEND USER_SOURCES drivers/sd_card_spi.c)
elseif(TEST_CASE STREQUAL "servo_motor")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/servo_motor_tests.c)
elseif(TEST_CASE STREQUAL "soft_i2c")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/afiskon-stm32-ssd1306/ssd1306_tests.c)
    list(APPEND USER_SOURCES
            drivers/afiskon-stm32-ssd1306/ssd1306.c
            drivers/afiskon-stm32-ssd1306/ssd1306_fonts.c
    )
elseif(TEST_CASE STREQUAL "st7735")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/st7735_tests.c)
    list(APPEND USER_SOURCES drivers/st7735.c)
elseif(TEST_CASE STREQUAL "st7789")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/st7789_tests.c)
    list(APPEND USER_SOURCES drivers/st7789.c)
elseif(TEST_CASE STREQUAL "stepper")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/stepper_tests.c)
    list(APPEND USER_SOURCES drivers/stepper_motor.c)
elseif(TEST_CASE STREQUAL "UART")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/uart_driver_test.c)
    # UART driver is already in common sources
elseif(TEST_CASE STREQUAL "w25qxx")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/w25qxx_test.c)
    list(APPEND USER_SOURCES drivers/w25qxx.c)
elseif(TEST_CASE STREQUAL "ws2812")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/ws2812_tests.c)
elseif(TEST_CASE STREQUAL "xpt2046")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/xpt2046_tests.c)
elseif(TEST_CASE STREQUAL "esp8266")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/esp8266_tests.c)
    list(APPEND USER_SOURCES 
        drivers/esp8266.c
        drivers/uart.c
        drivers/delay.c
    )
elseif(TEST_CASE STREQUAL "flash")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/internal_flash_tests.c)
    list(APPEND USER_SOURCES drivers/internal_flash.c)
elseif(TEST_CASE STREQUAL "watchdog")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/watchdog_tests.c)
    list(APPEND USER_SOURCES drivers/watchdog.c)
elseif(TEST_CASE STREQUAL "rtc")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/rtc_driver_tests.c)
    list(APPEND USER_SOURCES drivers/rtc_driver.c)
elseif(TEST_CASE STREQUAL "nrf24")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/nrf24l01_tests.c)
    list(APPEND USER_SOURCES drivers/nrf24l01.c)
elseif(TEST_CASE STREQUAL "ds18b20")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/ds18b20_tests.c)
    list(APPEND USER_SOURCES drivers/ds18b20.c)
elseif(TEST_CASE STREQUAL "usb")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/usb_serial_tests.c)
    list(APPEND USER_SOURCES drivers/usb_serial.c)
elseif(TEST_CASE STREQUAL "light")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/light_sensor_tests.c)
    list(APPEND USER_SOURCES 
        drivers/light_sensor.c
        middlewares/algorithms/moving_average.c
    )
elseif(TEST_CASE STREQUAL "pot")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/potentiometer_tests.c)
    list(APPEND USER_SOURCES 
        drivers/potentiometer.c
        middlewares/algorithms/moving_average.c
    )
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
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(User PUBLIC stm32cubemx)
