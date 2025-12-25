# Test Case Selection
set(TEST_CASE "freertos_test" CACHE STRING "Select which test to run" FORCE)
message("Selected Tesusb_serialt Case: ${TEST_CASE}")

# ==========================================
# 0. Helper Macros
# ==========================================

# Macro to easier define a component/driver
# Usage: define_module(NAME sources... INCLUDES includes... DEPENDS dependencies...)
macro(define_module name)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES INCLUDES DEPENDS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_library(${name} OBJECT ${ARG_SOURCES})
    target_include_directories(${name} PUBLIC ${ARG_INCLUDES})
    
    target_link_libraries(${name} PRIVATE stm32cubemx)

    # Store dependencies in a custom property to be resolved later
    set_target_properties(${name} PROPERTIES MODULE_DEPENDS "${ARG_DEPENDS}")
    
    set_property(GLOBAL APPEND PROPERTY ALL_USER_MODULES ${name})
endmacro()

# applications


# components

define_module(letter_shell
    SOURCES 
        components/letter-shell/csrc/shell.c 
        components/letter-shell/csrc/shell_ext.c
        components/letter-shell/letter-shell_port.c
    INCLUDES 
        components/letter-shell
        components/letter-shell/csrc
    DEPENDS uart
)

# drivers

# ==========================================
# communication
# ==========================================

define_module(uart
    SOURCES drivers/communication/uart.c
    INCLUDES drivers/communication
)

define_module(can
    SOURCES drivers/communication/can_driver.c
    INCLUDES drivers/communication
)

define_module(esp8266
    SOURCES drivers/communication/esp8266.c
    INCLUDES drivers/communication
    DEPENDS uart delay
)

define_module(ir_nec_exti
    SOURCES drivers/communication/ir_nec_exti.c
    INCLUDES drivers/communication
    DEPENDS delay
)

define_module(ir_nec_tim
    SOURCES drivers/communication/ir_nec_tim.c
    INCLUDES drivers/communication
    DEPENDS delay
)

define_module(nrf24l01
    SOURCES drivers/communication/nrf24l01.c
    INCLUDES drivers/communication
    DEPENDS delay
)

define_module(rs485
    SOURCES drivers/communication/rs485.c
    INCLUDES drivers/communication
    DEPENDS uart
)

define_module(usb_serial
    SOURCES drivers/communication/usb_serial.c
    INCLUDES drivers/communication
)

define_module(w5500
    SOURCES drivers/communication/w5500.c
    INCLUDES drivers/communication
)

# ==========================================
# display
# ==========================================

define_module(ili9341
    SOURCES drivers/display/ili9341.c
    INCLUDES drivers/display
)

define_module(ili9488
    SOURCES drivers/display/ili9488.c
    INCLUDES drivers/display
)

define_module(ssd1306_afiskon
    SOURCES 
        drivers/display/afiskon-stm32-ssd1306/ssd1306.c
        drivers/display/afiskon-stm32-ssd1306/ssd1306_fonts.c
    INCLUDES 
        drivers/display
        drivers/display/afiskon-stm32-ssd1306
    DEPENDS i2c_soft
)

define_module(ssd1306_custom
    SOURCES 
        drivers/display/ssd1306_custom.c
        drivers/display/ssd1306_fonts_custom.c
    INCLUDES drivers/display
    DEPENDS i2c_soft
)

define_module(st7735
    SOURCES drivers/display/st7735.c
    INCLUDES drivers/display
)

define_module(st7789
    SOURCES drivers/display/st7789.c
    INCLUDES drivers/display
)

# ==========================================
# encoder
# ==========================================

define_module(encoder_knob
    SOURCES drivers/encoder/encoder_knob.c
    INCLUDES drivers/encoder
)

define_module(encoder_motor
    SOURCES drivers/encoder/encoder_motor.c
    INCLUDES drivers/encoder
)

# ==========================================
# interface
# ==========================================

define_module(i2c_soft
    SOURCES drivers/interface/soft_i2c.c
    INCLUDES drivers/interface
    DEPENDS delay
)

define_module(spi_soft
    SOURCES drivers/interface/soft_spi.c
    INCLUDES drivers/interface
    DEPENDS delay
)

# ==========================================
# io
# ==========================================

define_module(buzzer
    SOURCES drivers/io/buzzer.c
    INCLUDES drivers/io
)

define_module(led
    SOURCES drivers/io/led.c
    INCLUDES drivers/io
)

define_module(key
    SOURCES drivers/io/key.c
    INCLUDES drivers/io
)

define_module(ws2812
    SOURCES drivers/io/ws2812.c
    INCLUDES drivers/io
)

define_module(xpt2046
    SOURCES drivers/io/xpt2046.c
    INCLUDES drivers/io
)

# ==========================================
# motor
# ==========================================

define_module(bldc_motor_esc_encoder
    SOURCES drivers/motor/bldc_motor_esc_encoder.c
    INCLUDES drivers/motor
)

define_module(servo_motor
    SOURCES drivers/motor/servo_motor.c
    INCLUDES drivers/motor
)

define_module(stepper_motor
    SOURCES drivers/motor/stepper_motor.c
    INCLUDES drivers/motor
)

# ==========================================
# sensor
# ==========================================

define_module(bh1750
    SOURCES drivers/sensor/bh1750.c
    INCLUDES drivers/sensor
    DEPENDS i2c_soft delay
)

define_module(dht11
    SOURCES drivers/sensor/dht11.c
    INCLUDES drivers/sensor
    DEPENDS delay
)

define_module(ds18b20
    SOURCES drivers/sensor/ds18b20.c
    INCLUDES drivers/sensor
    DEPENDS delay
)

define_module(hc_sr04
    SOURCES drivers/sensor/hc_sr04.c
    INCLUDES drivers/sensor
    DEPENDS delay
)

define_module(light_sensor
    SOURCES drivers/sensor/light_sensor.c
    INCLUDES drivers/sensor
    DEPENDS moving_average
)

define_module(mpu6050
    SOURCES drivers/sensor/mpu6050.c
    INCLUDES drivers/sensor
    DEPENDS i2c_soft
)

define_module(potentiometer
    SOURCES drivers/sensor/potentiometer.c
    INCLUDES drivers/sensor
    DEPENDS moving_average
)

# ==========================================
# storage
# ==========================================

define_module(at24cxx
    SOURCES drivers/storage/at24cxx.c
    INCLUDES drivers/storage
    DEPENDS delay i2c_soft
)

define_module(internal_flash
    SOURCES drivers/storage/internal_flash.c
    INCLUDES drivers/storage
)

define_module(sd_card_spi
    SOURCES drivers/storage/sd_card_spi.c
    INCLUDES drivers/storage
    DEPENDS delay
)

define_module(w25qxx
    SOURCES drivers/storage/w25qxx.c
    INCLUDES drivers/storage
    DEPENDS delay
)

# ==========================================
# system
# ==========================================

define_module(delay
    SOURCES drivers/system/delay.c
    INCLUDES drivers/system
)

define_module(rtc
    SOURCES drivers/system/rtc_driver.c
    INCLUDES drivers/system
)

define_module(watchdog
    SOURCES drivers/system/watchdog.c
    INCLUDES drivers/system
)

# middlewares

# algorithms
define_module(moving_average
    SOURCES middlewares/algorithms/moving_average.c
    INCLUDES middlewares/algorithms
)

define_module(pid
    SOURCES middlewares/algorithms/pid.c
    INCLUDES middlewares/algorithms
)

# protocols
define_module(crsf
    SOURCES middlewares/protocols/crsf.c
    INCLUDES middlewares/protocols
)

# ==========================================
# 6. Deferred Linking (Connect modules regardless of definition order)
# ==========================================
get_property(all_modules GLOBAL PROPERTY ALL_USER_MODULES)
foreach(mod ${all_modules})
    get_target_property(deps ${mod} MODULE_DEPENDS)
    if(deps)
        foreach(dep ${deps})
            if(TARGET ${dep})
                # 现在所有 Target 都一定已经定义了，可以安全 Link
                target_link_libraries(${mod} PRIVATE ${dep})
            else()
                message(WARNING "Module ${mod} depends on ${dep}, but ${dep} is not defined!")
            endif()
        endforeach()
    endif()
endforeach()

# ==========================================
# 7. Dependency Resolution Logic
# ==========================================

# If ENABLED_MODULES is not set by parent project, determine it from TEST_CASE
if (NOT DEFINED ENABLED_MODULES)
    set(ENABLED_MODULES "")
    set(TEST_SRC "")

    # applications
    if (TEST_CASE STREQUAL "freertos_test")
        list(APPEND ENABLED_MODULES led uart) 
        set(TEST_SRC applications/freertos_test.c)
        
    elseif (TEST_CASE STREQUAL "i2c_master")
        list(APPEND ENABLED_MODULES uart)
        set(TEST_SRC applications/i2c_master_slave.c)
        target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE DEMO_ROLE=1) # 1 = MASTER
        
    elseif (TEST_CASE STREQUAL "i2c_slave")
        list(APPEND ENABLED_MODULES uart)
        set(TEST_SRC applications/i2c_master_slave.c)
        target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE DEMO_ROLE=0) # 0 = SLAVE

    # components
    elseif (TEST_CASE STREQUAL "shell")
        list(APPEND ENABLED_MODULES letter_shell)
        set(TEST_SRC components/tests/letter-shell_tests.c)
  

    # drivers
    # communication
    elseif (TEST_CASE STREQUAL "uart")
        list(APPEND ENABLED_MODULES uart)
        set(TEST_SRC drivers/tests/uart_tests.c)

    elseif (TEST_CASE STREQUAL "usb_serial")
        list(APPEND ENABLED_MODULES usb_serial delay)
        set(TEST_SRC drivers/tests/usb_serial_tests.c)
    elseif (TEST_CASE STREQUAL "can")
        list(APPEND ENABLED_MODULES can)
        set(TEST_SRC drivers/tests/can_tests.c)
    elseif (TEST_CASE STREQUAL "esp8266")
        list(APPEND ENABLED_MODULES esp8266)
        set(TEST_SRC drivers/tests/esp8266_tests.c)
    elseif (TEST_CASE STREQUAL "ir_nec_exti")
        list(APPEND ENABLED_MODULES ir_nec_exti)
        set(TEST_SRC drivers/tests/ir_nec_exti_tests.c)
    elseif (TEST_CASE STREQUAL "ir_nec_tim")
        list(APPEND ENABLED_MODULES ir_nec_tim)
        set(TEST_SRC drivers/tests/ir_nec_tim_tests.c)
    elseif (TEST_CASE STREQUAL "nrf24l01")
        list(APPEND ENABLED_MODULES nrf24l01)
        set(TEST_SRC drivers/tests/nrf24l01_tests.c)
    elseif (TEST_CASE STREQUAL "rs485")
        list(APPEND ENABLED_MODULES rs485)
        set(TEST_SRC drivers/tests/rs485_tests.c)
    elseif (TEST_CASE STREQUAL "w5500")
        list(APPEND ENABLED_MODULES w5500)
        set(TEST_SRC drivers/tests/w5500_tests.c)
    # display

    # encoder

    # io
    elseif (TEST_CASE STREQUAL "buzzer")
        list(APPEND ENABLED_MODULES buzzer uart)
        set(TEST_SRC drivers/tests/buzzer_tests.c)
    elseif (TEST_CASE STREQUAL "led")
        list(APPEND ENABLED_MODULES led)
        set(TEST_SRC drivers/tests/led_tests.c)
    elseif (TEST_CASE STREQUAL "ws2812")
        list(APPEND ENABLED_MODULES ws2812 uart)
        set(TEST_SRC drivers/tests/ws2812_tests.c)
    elseif (TEST_CASE STREQUAL "xpt2046")
        list(APPEND ENABLED_MODULES xpt2046 uart)
        set(TEST_SRC drivers/tests/xpt2046_tests.c)
    
    # motor
    elseif (TEST_CASE STREQUAL "bldc_motor_esc_encoder")
        list(APPEND ENABLED_MODULES bldc_motor_esc_encoder uart delay)
        set(TEST_SRC drivers/tests/bldc_motor_esc_encoder_tests.c)

    elseif (TEST_CASE STREQUAL "servo_motor")
        list(APPEND ENABLED_MODULES servo_motor uart)
        set(TEST_SRC drivers/tests/servo_motor_tests.c)

    elseif (TEST_CASE STREQUAL "stepper_motor")
        list(APPEND ENABLED_MODULES stepper_motor uart delay)
        set(TEST_SRC drivers/tests/stepper_motor_tests.c)

    # sensor
    elseif (TEST_CASE STREQUAL "bh1750")
        list(APPEND ENABLED_MODULES bh1750 uart delay)
        set(TEST_SRC drivers/tests/bh1750_tests.c)

    elseif (TEST_CASE STREQUAL "dht11")
        list(APPEND ENABLED_MODULES delay dht11 uart) # UART for printing
        set(TEST_SRC drivers/tests/dht11_tests.c)

    elseif (TEST_CASE STREQUAL "ds18b20")
        list(APPEND ENABLED_MODULES ds18b20 uart delay)
        set(TEST_SRC drivers/tests/ds18b20_tests.c)

    elseif (TEST_CASE STREQUAL "hc_sr04")
        list(APPEND ENABLED_MODULES hc_sr04 uart delay)
        set(TEST_SRC drivers/tests/hc_sr04_tests.c)

    elseif (TEST_CASE STREQUAL "light_sensor")
        list(APPEND ENABLED_MODULES light_sensor uart delay)
        set(TEST_SRC drivers/tests/light_sensor_tests.c)

    elseif (TEST_CASE STREQUAL "mpu6050")
        list(APPEND ENABLED_MODULES mpu6050 uart delay)
        set(TEST_SRC drivers/tests/mpu6050_tests.c)

    elseif (TEST_CASE STREQUAL "potentiometer")
        list(APPEND ENABLED_MODULES potentiometer uart delay)
        set(TEST_SRC drivers/tests/potentiometer_tests.c)

    # storage
    elseif (TEST_CASE STREQUAL "at24cxx")
        list(APPEND ENABLED_MODULES at24cxx uart delay)
        set(TEST_SRC drivers/tests/at24cxx_tests.c)

    elseif (TEST_CASE STREQUAL "internal_flash")
        list(APPEND ENABLED_MODULES internal_flash uart)
        set(TEST_SRC drivers/tests/internal_flash_tests.c)

    elseif (TEST_CASE STREQUAL "sd_card_spi")
        list(APPEND ENABLED_MODULES sd_card_spi uart delay)
        set(TEST_SRC drivers/tests/sd_card_spi_tests.c)

    elseif (TEST_CASE STREQUAL "w25qxx")
        list(APPEND ENABLED_MODULES w25qxx uart delay)
        set(TEST_SRC drivers/tests/w25qxx_tests.c)

    # system
    elseif (TEST_CASE STREQUAL "delay")
        list(APPEND ENABLED_MODULES delay uart)
        set(TEST_SRC drivers/tests/delay_tests.c)
    elseif (TEST_CASE STREQUAL "rtc")
        list(APPEND ENABLED_MODULES rtc uart)
        set(TEST_SRC drivers/tests/rtc_tests.c)
    elseif (TEST_CASE STREQUAL "watchdog")
        list(APPEND ENABLED_MODULES watchdog uart)
        set(TEST_SRC drivers/tests/watchdog_tests.c)



    else()
        message(STATUS "TEST_CASE ${TEST_CASE} not migrated to new system yet.")
    endif()

    if (TEST_SRC)
        target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${TEST_SRC})
    endif()
endif()

# Recursive function to resolve dependencies
function(resolve_dependencies modules_list out_result)
    set(visited ${${out_result}})
    set(queue ${modules_list})
    
    foreach(mod ${queue})
        if (NOT "${mod}" IN_LIST visited)
            list(APPEND visited ${mod})
            get_target_property(deps ${mod} MODULE_DEPENDS)
            if (deps)
                resolve_dependencies("${deps}" visited)
            endif()
        endif()
    endforeach()
    set(${out_result} ${visited} PARENT_SCOPE)
endfunction()

# Resolve all dependencies
set(FINAL_MODULES "")
resolve_dependencies("${ENABLED_MODULES}" FINAL_MODULES)
message(STATUS "Enabled User Modules: ${FINAL_MODULES}")

# ==========================================
# 7. Create Final User Library
# ==========================================

add_library(user INTERFACE)

foreach(mod ${FINAL_MODULES})
    # Check if target exists (in case of typo)
    if (TARGET ${mod})
        target_sources(user INTERFACE $<TARGET_OBJECTS:${mod}>)
        target_include_directories(user INTERFACE $<TARGET_PROPERTY:${mod},INTERFACE_INCLUDE_DIRECTORIES>)
    else()
        message(WARNING "Module ${mod} is required but not defined!")
    endif()
endforeach()

# Add global include paths if necessary (e.g. for user_main.h)
target_include_directories(user INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/drivers
    ${CMAKE_CURRENT_SOURCE_DIR}/components
)

target_link_libraries(user INTERFACE stm32cubemx)
