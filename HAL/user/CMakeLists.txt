# Test Case Selection
set(TEST_CASE "easy-logger" CACHE STRING "Select which test to run (BUZZER, CRSF, DELAY, DHT11, KEY, LED, MOTOR, MPU6050, SSD1306, SSD1306_CUSTOM, UART, W25QXX)" FORCE)
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
elseif(TEST_CASE STREQUAL "letter-shell")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE components/tests/letter-shell_tests.c)
    list(APPEND USER_SOURCES
            components/letter-shell/csrc/shell.c
            components/letter-shell/csrc/shell_ext.c
            components/letter-shell/letter-shell_port.c
    )
    list(APPEND USER_INCLUDES
            ${CMAKE_CURRENT_SOURCE_DIR}/components/letter-shell
            ${CMAKE_CURRENT_SOURCE_DIR}/components/letter-shell/csrc
    )

elseif(TEST_CASE STREQUAL "easy-logger")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE components/tests/easy-logger_tests.c)
    list(APPEND USER_SOURCES
            components/easy-logger/csrc/elog.c
            components/easy-logger/csrc/elog_async.c
            components/easy-logger/csrc/elog_buf.c
            components/easy-logger/csrc/elog_utils.c
            components/easy-logger/elog_port.c
    )
    list(APPEND USER_INCLUDES
            ${CMAKE_CURRENT_SOURCE_DIR}/components/easy-logger
            ${CMAKE_CURRENT_SOURCE_DIR}/components/easy-logger/csrc
    )

elseif(TEST_CASE STREQUAL "littlefs")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE components/tests/littlefs_tests.c)
    list(APPEND USER_SOURCES
            components/littlefs/csrc/lfs.c
            components/littlefs/csrc/lfs_util.c
            components/littlefs/littlefs_port.c
            drivers/w25qxx.c
    )
    list(APPEND USER_INCLUDES
            ${CMAKE_CURRENT_SOURCE_DIR}/components/littlefs
            ${CMAKE_CURRENT_SOURCE_DIR}/components/littlefs/csrc
    )

elseif(TEST_CASE STREQUAL "u8g2")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE components/tests/u8g2_tests.c)

    # === U8G2 Core Files (Minimal Set) ===
    list(APPEND U8G2_SOURCES
            components/u8g2/u8g2_bitmap.c
            components/u8g2/u8g2_box.c
            components/u8g2/u8g2_buffer.c
            components/u8g2/u8g2_circle.c
            components/u8g2/u8g2_cleardisplay.c
            components/u8g2/u8g2_font.c
            components/u8g2/u8g2_fonts.c
            components/u8g2/u8g2_hvline.c
            components/u8g2/u8g2_input_value.c
            components/u8g2/u8g2_intersection.c
            components/u8g2/u8g2_kerning.c
            components/u8g2/u8g2_line.c
            components/u8g2/u8g2_ll_hvline.c
            components/u8g2/u8g2_message.c
            components/u8g2/u8g2_polygon.c
            components/u8g2/u8g2_selection_list.c
            components/u8g2/u8g2_setup.c
            components/u8g2/u8x8_8x8.c
            components/u8g2/u8x8_byte.c
            components/u8g2/u8x8_cad.c
            components/u8g2/u8x8_debounce.c
            components/u8g2/u8x8_display.c
            components/u8g2/u8x8_gpio.c
            components/u8g2/u8x8_input_value.c
            components/u8g2/u8x8_message.c
            components/u8g2/u8x8_selection_list.c
            components/u8g2/u8x8_setup.c
            components/u8g2/u8x8_string.c
            components/u8g2/u8x8_u8toa.c
            components/u8g2/u8x8_u16toa.c

            # === Porting Layer ===
            components/u8g2/u8g2_port.c

            # === Selected Display Drivers (Uncomment as needed) ===
            components/u8g2/u8x8_d_ssd1306_128x64_noname.c
            # components/u8g2/u8x8_d_pcd8544_84x48.c
    )

    list(APPEND USER_SOURCES ${U8G2_SOURCES})
    list(APPEND USER_INCLUDES
            ${CMAKE_CURRENT_SOURCE_DIR}/components/u8g2
    )

elseif(TEST_CASE STREQUAL "lvgl")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE components/tests/lvgl_tests.c)

    # === LVGL Core Files (Auto-collected) ===
    # Use GLOB_RECURSE to collect all LVGL source files
    file(GLOB_RECURSE LVGL_CORE_SOURCES
            components/lvgl/csrc/core/*.c
            components/lvgl/csrc/display/*.c
            components/lvgl/csrc/draw/sw/*.c        # Software renderer
            components/lvgl/csrc/draw/lv_draw*.c    # Draw core
            components/lvgl/csrc/font/*.c
            components/lvgl/csrc/indev/*.c
            components/lvgl/csrc/layouts/*.c
            components/lvgl/csrc/misc/*.c
            components/lvgl/csrc/osal/*.c
            components/lvgl/csrc/stdlib/*.c
            components/lvgl/csrc/themes/*.c
            components/lvgl/csrc/tick/*.c
            components/lvgl/csrc/widgets/*.c
    )

    # === LVGL Port Layer ===
    list(APPEND LVGL_SOURCES
            ${LVGL_CORE_SOURCES}
            components/lvgl/lvgl_port.c
    )

    list(APPEND USER_SOURCES ${LVGL_SOURCES})
    list(APPEND USER_INCLUDES
            ${CMAKE_CURRENT_SOURCE_DIR}/components/lvgl
            ${CMAKE_CURRENT_SOURCE_DIR}/components/lvgl/csrc
    )

    # LVGL specific compile definitions
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
            LV_CONF_INCLUDE_SIMPLE=1
    )

elseif (TEST_CASE STREQUAL "servo")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE components/tests/servo_test.c)
    list(APPEND USER_SOURCES
            components/servo/csrc/servo.c
            components/servo/servo_port.c
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
elseif(TEST_CASE STREQUAL "buzzer")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/buzzer_tests.c)
    list(APPEND USER_SOURCES drivers/buzzer.c)
elseif(TEST_CASE STREQUAL "ir")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/ir_tests.c)
    list(APPEND USER_SOURCES 
        drivers/ir_nec_exti.c
        drivers/ir_nec_tim.c
        drivers/delay.c
    )
else()
    # Default to CRSF if nothing matches
    message(STATUS "No specific TEST_CASE matched, defaulting to CRSF")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/Tests/crsf_protocol_test.c)
    list(APPEND USER_SOURCES middlewares/protocols/crsf.c)
endif()

# Create the library with the selected sources
add_library(User OBJECT ${USER_SOURCES})

# Include directories - 基础路径（全局）
target_include_directories(User PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}

        ${CMAKE_CURRENT_SOURCE_DIR}/applications
        ${CMAKE_CURRENT_SOURCE_DIR}/components
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers
        ${CMAKE_CURRENT_SOURCE_DIR}/middlewares
        
        ${USER_INCLUDES}  # 组件特定的 include 路径（条件化）
)

target_link_libraries(User PUBLIC stm32cubemx)
