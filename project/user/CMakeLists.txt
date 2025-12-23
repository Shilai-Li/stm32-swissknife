# Test Case Selection
set(TEST_CASE "letter-shell" CACHE STRING "Select which test to run" FORCE)
message("Selected Test Case: ${TEST_CASE}")

# Initialize source list
# You can add common files here that are ALWAYS needed
set(USER_SOURCES
        # drivers/system/delay.c
        # drivers/communication/uart.c
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

            drivers/communication/uart.c
    )
    list(APPEND USER_INCLUDES
            ${CMAKE_CURRENT_SOURCE_DIR}/components/letter-shell
            ${CMAKE_CURRENT_SOURCE_DIR}/components/letter-shell/csrc

            ${CMAKE_CURRENT_SOURCE_DIR}/drivers/communication
    )

elseif(TEST_CASE STREQUAL "easy-logger")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE components/tests/easy-logger_tests.c)
    list(APPEND USER_SOURCES
            components/easy-logger/csrc/elog.c
            components/easy-logger/csrc/elog_async.c
            components/easy-logger/csrc/elog_buf.c
            components/easy-logger/csrc/elog_utils.c
            components/easy-logger/elog_port.c

            drivers/communication/uart.c
    )
    list(APPEND USER_INCLUDES
            ${CMAKE_CURRENT_SOURCE_DIR}/components/easy-logger
            ${CMAKE_CURRENT_SOURCE_DIR}/components/easy-logger/csrc

            ${CMAKE_CURRENT_SOURCE_DIR}/drivers/communication
    )

elseif(TEST_CASE STREQUAL "littlefs")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE components/tests/littlefs_tests.c)
    list(APPEND USER_SOURCES
            components/littlefs/csrc/lfs.c
            components/littlefs/csrc/lfs_util.c
            components/littlefs/littlefs_port.c
            drivers/storage/w25qxx.c
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
    # csrc/ 对应官方 LVGL 的 src/ 目录
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

elseif(TEST_CASE STREQUAL "cjson")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE components/tests/cjson_tests.c)
    list(APPEND USER_SOURCES
            components/cjson/csrc/cJSON.c
    )
    list(APPEND USER_INCLUDES
            ${CMAKE_CURRENT_SOURCE_DIR}/components/cjson
            ${CMAKE_CURRENT_SOURCE_DIR}/components/cjson/csrc
    )

elseif(TEST_CASE STREQUAL "coremqtt")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE components/tests/coremqtt_tests.c)
    list(APPEND USER_SOURCES
            components/coremqtt/csrc/core_mqtt.c
            components/coremqtt/csrc/core_mqtt_serializer.c
            components/coremqtt/csrc/core_mqtt_state.c
            components/coremqtt/coremqtt_port.c
    )
    list(APPEND USER_INCLUDES
            ${CMAKE_CURRENT_SOURCE_DIR}/components/coremqtt
            ${CMAKE_CURRENT_SOURCE_DIR}/components/coremqtt/csrc
    )

elseif(TEST_CASE STREQUAL "multitimer")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE components/tests/multitimer_tests.c)
    list(APPEND USER_SOURCES
            components/multitimer/csrc/MultiTimer.c
            components/multitimer/multitimer_port.c
    )
    list(APPEND USER_INCLUDES
            ${CMAKE_CURRENT_SOURCE_DIR}/components/multitimer
            ${CMAKE_CURRENT_SOURCE_DIR}/components/multitimer/csrc
    )

elseif(TEST_CASE STREQUAL "nanomodbus")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE components/tests/nanomodbus_tests.c)
    list(APPEND USER_SOURCES
            components/nanomodbus/csrc/nanomodbus.c
            components/nanomodbus/nanomodbus_port.c
    )
    list(APPEND USER_INCLUDES
            ${CMAKE_CURRENT_SOURCE_DIR}/components/nanomodbus
            ${CMAKE_CURRENT_SOURCE_DIR}/components/nanomodbus/csrc
    )

elseif(TEST_CASE STREQUAL "tinyframe")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE components/tests/tinyframe_tests.c)
    list(APPEND USER_SOURCES
            components/tinyframe/csrc/TinyFrame.c
            components/tinyframe/tinyframe_port.c
    )
    list(APPEND USER_INCLUDES
            ${CMAKE_CURRENT_SOURCE_DIR}/components/tinyframe
            ${CMAKE_CURRENT_SOURCE_DIR}/components/tinyframe/csrc
    )

elseif(TEST_CASE STREQUAL "lwrb")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE components/tests/lwrb_tests.c)
    list(APPEND USER_SOURCES
            components/lwrb/csrc/lwrb.c
    )
    list(APPEND USER_INCLUDES
            ${CMAKE_CURRENT_SOURCE_DIR}/components/lwrb
            ${CMAKE_CURRENT_SOURCE_DIR}/components/lwrb/csrc
    )

elseif(TEST_CASE STREQUAL "sfud")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE components/tests/sfud_tests.c)
    list(APPEND USER_SOURCES
            components/sfud/csrc/sfud.c
            components/sfud/csrc/sfud_sfdp.c
            components/sfud/sfud_port.c
    )
    list(APPEND USER_INCLUDES
            ${CMAKE_CURRENT_SOURCE_DIR}/components/sfud
            ${CMAKE_CURRENT_SOURCE_DIR}/components/sfud/csrc
    )

elseif (TEST_CASE STREQUAL "servo")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE components/tests/servo_test.c)
    list(APPEND USER_SOURCES
            components/servo/csrc/servo.c
            components/servo/servo_port.c
            drivers/motor/dc_motor.c
            middlewares/algorithms/pid.c
    )

elseif(TEST_CASE STREQUAL "flexible_button")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE components/tests/flexible_button_tests.c)
    list(APPEND USER_SOURCES
            components/flexible_button/csrc/flexible_button.c
            components/flexible_button/flexible_button_port.c
    )
    list(APPEND USER_INCLUDES
            ${CMAKE_CURRENT_SOURCE_DIR}/components/flexible_button
            ${CMAKE_CURRENT_SOURCE_DIR}/components/flexible_button/csrc
    )

elseif(TEST_CASE STREQUAL "freertos")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE components/tests/freertos_tests.c)
    list(APPEND USER_SOURCES
            components/freertos/csrc/tasks.c
            components/freertos/csrc/queue.c
            components/freertos/csrc/list.c
            components/freertos/csrc/timers.c
            components/freertos/csrc/event_groups.c
            components/freertos/csrc/stream_buffer.c
            components/freertos/csrc/portable/GCC/ARM_CM3/port.c
            components/freertos/csrc/portable/MemMang/heap_4.c
            components/freertos/freertos_port.c
    )
    list(APPEND USER_INCLUDES
            ${CMAKE_CURRENT_SOURCE_DIR}/components/freertos
            ${CMAKE_CURRENT_SOURCE_DIR}/components/freertos/csrc/include
            ${CMAKE_CURRENT_SOURCE_DIR}/components/freertos/csrc/portable/GCC/ARM_CM3

            ${CMAKE_CURRENT_SOURCE_DIR}/drivers/communication
    )


# Drivers
elseif(TEST_CASE STREQUAL "at24cxx")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/at24cxx_tests.c)
    list(APPEND USER_SOURCES drivers/storage/at24cxx.c)
elseif(TEST_CASE STREQUAL "adc")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/adc_tests.c)
elseif(TEST_CASE STREQUAL "buzzer")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/buzzer_driver_test.c)
    list(APPEND USER_SOURCES drivers/io/buzzer.c)
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
    list(APPEND USER_SOURCES drivers/sensor/dht11.c)
elseif(TEST_CASE STREQUAL "encoder")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/encoder_tests.c)
elseif(TEST_CASE STREQUAL "hc_sr04")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/hc_sr04_tests.c)
elseif(TEST_CASE STREQUAL "ili9341")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/ili9341_tests.c)
    list(APPEND USER_SOURCES drivers/display/ili9341.c)
elseif(TEST_CASE STREQUAL "ili9488")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/ili9488_tests.c)
    list(APPEND USER_SOURCES drivers/display/ili9488.c)
elseif(TEST_CASE STREQUAL "key")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/key_driver_test.c)
    list(APPEND USER_SOURCES drivers/io/key.c)
elseif(TEST_CASE STREQUAL "led")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/led_driver_tests.c)
    list(APPEND USER_SOURCES drivers/io/led.c)
elseif(TEST_CASE STREQUAL "motor")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/motor_tests.c)
elseif(TEST_CASE STREQUAL "mpu6050")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/mpu6050_tests.c)
    list(APPEND USER_SOURCES drivers/sensor/mpu6050.c)
    list(APPEND USER_INCLUDES
            ${CMAKE_CURRENT_SOURCE_DIR}/drivers/sensor
            ${CMAKE_CURRENT_SOURCE_DIR}/drivers/communication
    )
elseif(TEST_CASE STREQUAL "rs485")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/rs485_tests.c)
elseif(TEST_CASE STREQUAL "ssd1306_custom")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/ssd1306_custom_tests.c)
    list(APPEND USER_SOURCES
            drivers/display/afiskon-stm32-ssd1306/ssd1306.c
            drivers/display/afiskon-stm32-ssd1306/ssd1306_fonts.c
            drivers/display/ssd1306_custom.c
            drivers/display/ssd1306_fonts_custom.c
    )
elseif(TEST_CASE STREQUAL "sd_card_spi")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/sd_card_spi_tests.c)
    list(APPEND USER_SOURCES drivers/storage/sd_card_spi.c)
elseif(TEST_CASE STREQUAL "servo_motor")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/servo_motor_tests.c)
elseif(TEST_CASE STREQUAL "soft_i2c")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/afiskon-stm32-ssd1306/ssd1306_tests.c)
    list(APPEND USER_SOURCES
            drivers/display/afiskon-stm32-ssd1306/ssd1306.c
            drivers/display/afiskon-stm32-ssd1306/ssd1306_fonts.c
    )
elseif(TEST_CASE STREQUAL "st7735")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/st7735_tests.c)
    list(APPEND USER_SOURCES drivers/display/st7735.c)
elseif(TEST_CASE STREQUAL "st7789")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/st7789_tests.c)
    list(APPEND USER_SOURCES drivers/display/st7789.c)
elseif(TEST_CASE STREQUAL "stepper")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/stepper_tests.c)
    list(APPEND USER_SOURCES drivers/motor/stepper_motor.c)
elseif(TEST_CASE STREQUAL "UART")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/uart_driver_test.c)
    # UART driver is already in common sources
elseif(TEST_CASE STREQUAL "w25qxx")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/w25qxx_test.c)
    list(APPEND USER_SOURCES drivers/storage/w25qxx.c)
elseif(TEST_CASE STREQUAL "ws2812")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/ws2812_tests.c)
elseif(TEST_CASE STREQUAL "xpt2046")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/xpt2046_tests.c)
elseif(TEST_CASE STREQUAL "esp8266")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/esp8266_tests.c)
    list(APPEND USER_SOURCES 
        drivers/communication/esp8266.c
        drivers/communication/uart.c
        drivers/system/delay.c
    )
elseif(TEST_CASE STREQUAL "esp8266_mqtt")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/esp8266_mqtt_tests.c)
    list(APPEND USER_SOURCES 
        drivers/communication/esp8266.c
        drivers/communication/uart.c
        drivers/system/delay.c
    )
elseif(TEST_CASE STREQUAL "flash")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/internal_flash_tests.c)
    list(APPEND USER_SOURCES drivers/storage/internal_flash.c)
elseif(TEST_CASE STREQUAL "watchdog")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/watchdog_tests.c)
    list(APPEND USER_SOURCES drivers/system/watchdog.c)
elseif(TEST_CASE STREQUAL "rtc")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/rtc_driver_tests.c)
    list(APPEND USER_SOURCES drivers/system/rtc_driver.c)
elseif(TEST_CASE STREQUAL "nrf24")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/nrf24l01_tests.c)
    list(APPEND USER_SOURCES drivers/communication/nrf24l01.c)
elseif(TEST_CASE STREQUAL "ds18b20")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/ds18b20_tests.c)
    list(APPEND USER_SOURCES drivers/sensor/ds18b20.c)
elseif(TEST_CASE STREQUAL "usb")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/usb_serial_tests.c)
    list(APPEND USER_SOURCES drivers/communication/usb_serial.c)
elseif(TEST_CASE STREQUAL "light")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/light_sensor_tests.c)
    list(APPEND USER_SOURCES 
        drivers/sensor/light_sensor.c
        middlewares/algorithms/moving_average.c
    )
elseif(TEST_CASE STREQUAL "pot")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/potentiometer_tests.c)
    list(APPEND USER_SOURCES 
        drivers/sensor/potentiometer.c
        middlewares/algorithms/moving_average.c
    )
elseif(TEST_CASE STREQUAL "buzzer")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/buzzer_tests.c)
    list(APPEND USER_SOURCES drivers/io/buzzer.c)
elseif(TEST_CASE STREQUAL "ir")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/tests/ir_tests.c)
    list(APPEND USER_SOURCES 
        drivers/communication/ir_nec_exti.c
        drivers/communication/ir_nec_tim.c
        drivers/system/delay.c
    )
else()
    # Default to CRSF if nothing matches
    message(STATUS "No specific TEST_CASE matched, defaulting to CRSF")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE drivers/Tests/crsf_protocol_test.c)
    list(APPEND USER_SOURCES middlewares/protocols/crsf.c)
endif()

# Create the library with the selected sources
add_library(user OBJECT ${USER_SOURCES})

# Include directories
target_include_directories(user PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}

        ${CMAKE_CURRENT_SOURCE_DIR}/applications
        ${CMAKE_CURRENT_SOURCE_DIR}/components
        ${CMAKE_CURRENT_SOURCE_DIR}/drivers
        ${CMAKE_CURRENT_SOURCE_DIR}/middlewares
        
        ${USER_INCLUDES}
)

target_link_libraries(user PUBLIC stm32cubemx)
