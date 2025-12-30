# drivers CMakeLists.txt
# All hardware drivers for STM32

# ==========================================
# Communication Drivers
# ==========================================

define_module(can
    SOURCES communication/can_driver.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/communication
)

define_module(esp8266
    SOURCES communication/esp8266.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/communication
    DEPENDS uart delay
)

define_module(ir_nec_exti
    SOURCES communication/ir_nec_exti.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/communication
    DEPENDS delay
)

define_module(ir_nec_tim
    SOURCES communication/ir_nec_tim.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/communication
    DEPENDS delay
)

define_module(nrf24l01
    SOURCES communication/nrf24l01.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/communication
    DEPENDS delay
)

define_module(rs485
    SOURCES communication/rs485.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/communication
    DEPENDS uart
)

define_module(uart
    SOURCES communication/uart.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/communication
)

define_module(usb_cdc
    SOURCES communication/usb_cdc.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/communication
)

define_module(w5500
    SOURCES communication/w5500.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/communication
)

# ==========================================
# Display Drivers
# ==========================================

define_module(ili9341
    SOURCES display/ili9341.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/display
)

define_module(ili9488
    SOURCES display/ili9488.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/display
)

define_module(ssd1306_afiskon
    SOURCES 
        display/afiskon-stm32-ssd1306/ssd1306.c
        display/afiskon-stm32-ssd1306/ssd1306_fonts.c
    INCLUDES 
        ${CMAKE_CURRENT_SOURCE_DIR}/display
        ${CMAKE_CURRENT_SOURCE_DIR}/display/afiskon-stm32-ssd1306
    DEPENDS i2c_soft
)

define_module(ssd1306_custom
    SOURCES 
        display/ssd1306_custom.c
        display/ssd1306_fonts_custom.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/display
    DEPENDS i2c_soft
)

define_module(st7735
    SOURCES display/st7735.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/display
)

define_module(st7789
    SOURCES display/st7789.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/display
)

# ==========================================
# Encoder Drivers
# ==========================================

define_module(encoder_knob
    SOURCES encoder/encoder_knob.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/encoder
)

define_module(encoder_motor
    SOURCES encoder/encoder_motor.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/encoder
)

# ==========================================
# Interface Drivers (Software I2C/SPI)
# ==========================================

define_module(i2c_soft
    SOURCES interface/soft_i2c.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/interface
    DEPENDS delay
)

define_module(spi_soft
    SOURCES interface/soft_spi.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/interface
    DEPENDS delay
)

# ==========================================
# IO Drivers
# ==========================================

define_module(buzzer
    SOURCES io/buzzer.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/io
)

define_module(led
    SOURCES io/led.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/io
)

define_module(key_single
    SOURCES io/key_single.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/io
)

define_module(key_matrix
    SOURCES io/key_matrix.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/io
)

define_module(ws2812
    SOURCES io/ws2812.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/io
)

define_module(xpt2046
    SOURCES io/xpt2046.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/io
)

# ==========================================
# Motor Drivers
# ==========================================

define_module(bldc_motor_esc_encoder
    SOURCES motor/bldc_motor_esc_encoder.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/motor
)

define_module(servo_motor
    SOURCES motor/servo_motor.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/motor
)

define_module(stepper_motor
    SOURCES motor/stepper_motor.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/motor
)

# ==========================================
# Sensor Drivers
# ==========================================

define_module(bh1750
    SOURCES sensor/bh1750.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/sensor
    DEPENDS i2c_soft delay
)

define_module(dht11
    SOURCES sensor/dht11.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/sensor
    DEPENDS delay
)

define_module(ds18b20
    SOURCES sensor/ds18b20.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/sensor
    DEPENDS delay
)

define_module(hc_sr04
    SOURCES sensor/hc_sr04.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/sensor
    DEPENDS delay
)

define_module(light_sensor
    SOURCES sensor/light_sensor.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/sensor
    DEPENDS moving_average
)

define_module(mpu6050
    SOURCES sensor/mpu6050.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/sensor
    DEPENDS i2c_soft
)

define_module(potentiometer
    SOURCES sensor/potentiometer.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/sensor
    DEPENDS moving_average
)

# ==========================================
# Storage Drivers
# ==========================================

define_module(at24cxx
    SOURCES storage/at24cxx.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/storage
    DEPENDS delay i2c_soft
)

define_module(internal_flash
    SOURCES storage/internal_flash.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/storage
)

define_module(sd_card_spi
    SOURCES storage/sd_card_spi.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/storage
    DEPENDS delay
)

define_module(w25qxx
    SOURCES storage/w25qxx.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/storage
    DEPENDS delay
)

# ==========================================
# System Drivers
# ==========================================

define_module(delay
    SOURCES system/delay.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/system
)

define_module(rtc
    SOURCES system/rtc_driver.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/system
)

define_module(watchdog
    SOURCES system/watchdog.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/system
)
