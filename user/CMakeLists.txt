# Test Case Selection
if (NOT DEFINED TEST_CASE)
    set(TEST_CASE "freertos_test" CACHE STRING "Select which test to run")
endif()
message("User Module - Current Test Case: ${TEST_CASE}")

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

# ==========================================
# 1. Include Subdirectories
# ==========================================

# Applications
add_subdirectory(applications)

# Components (third-party libraries)
add_subdirectory(components)

# Drivers (all hardware drivers)
add_subdirectory(drivers)

# Middlewares (algorithms and protocols)
add_subdirectory(middlewares)

# ==========================================
# 2. Deferred Linking (Connect modules regardless of definition order)
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
# 3. Test Case Configuration
# ==========================================

# If ENABLED_MODULES is not set by parent project, determine it from TEST_CASE
if (NOT DEFINED ENABLED_MODULES)
    set(ENABLED_MODULES "")
    set(TEST_SRC "")

    # applications
    if (TEST_CASE STREQUAL "freertos_producer_consumer")
        list(APPEND ENABLED_MODULES freertos_producer_consumer)
        
    elseif (TEST_CASE STREQUAL "i2c_master")
        list(APPEND ENABLED_MODULES uart)
        set(TEST_SRC applications/i2c_master_slave/i2c_master_slave.c)
        target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE DEMO_ROLE=1) # 1 = MASTER
        
    elseif (TEST_CASE STREQUAL "i2c_slave")
        list(APPEND ENABLED_MODULES uart)
        set(TEST_SRC applications/i2c_master_slave/i2c_master_slave.c)
        target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE DEMO_ROLE=0) # 0 = SLAVE

    # components
    elseif (TEST_CASE STREQUAL "shell_tests")
        list(APPEND ENABLED_MODULES letter_shell)
        set(TEST_SRC components/tests/letter-shell_tests.c)
  

    # drivers
    # communication
    elseif (TEST_CASE STREQUAL "can_tests")
        list(APPEND ENABLED_MODULES can)
        set(TEST_SRC drivers/tests/can_tests.c)

    elseif (TEST_CASE STREQUAL "esp8266_tests")
        list(APPEND ENABLED_MODULES esp8266)
        set(TEST_SRC drivers/tests/esp8266_tests.c)

    elseif (TEST_CASE STREQUAL "ir_nec_exti_tests")
        list(APPEND ENABLED_MODULES ir_nec_exti)
        set(TEST_SRC drivers/tests/ir_nec_exti_tests.c)

    elseif (TEST_CASE STREQUAL "ir_nec_tim_tests")
        list(APPEND ENABLED_MODULES ir_nec_tim)
        set(TEST_SRC drivers/tests/ir_nec_tim_tests.c)

    elseif (TEST_CASE STREQUAL "nrf24l01_tests")
        list(APPEND ENABLED_MODULES nrf24l01)
        set(TEST_SRC drivers/tests/nrf24l01_tests.c)

    elseif (TEST_CASE STREQUAL "rs485_tests")
        list(APPEND ENABLED_MODULES rs485)
        set(TEST_SRC drivers/tests/rs485_tests.c)

    elseif (TEST_CASE STREQUAL "uart_tests")
        list(APPEND ENABLED_MODULES uart usb_cdc)
        set(TEST_SRC drivers/communication/uart_tests.c)

    elseif (TEST_CASE STREQUAL "usb_cdc_tests")
        list(APPEND ENABLED_MODULES usb_cdc)
        set(TEST_SRC drivers/communication/usb_cdc_tests.c)

    elseif (TEST_CASE STREQUAL "w5500_tests")
        list(APPEND ENABLED_MODULES w5500)
        set(TEST_SRC drivers/tests/w5500_tests.c)

        
    # display

    # encoder

    # io
    elseif (TEST_CASE STREQUAL "buzzer_tests")
        list(APPEND ENABLED_MODULES buzzer uart usb_cdc)
        set(TEST_SRC drivers/io/buzzer_tests.c)

    elseif (TEST_CASE STREQUAL "led_tests")
        list(APPEND ENABLED_MODULES led usb_cdc)
        set(TEST_SRC drivers/io/led_tests.c)

    elseif (TEST_CASE STREQUAL "key_single_tests")
        list(APPEND ENABLED_MODULES key_single uart usb_cdc)
        set(TEST_SRC drivers/io/key_single_tests.c)

    elseif (TEST_CASE STREQUAL "key_matrix_tests")
        list(APPEND ENABLED_MODULES key_matrix uart usb_cdc)
        set(TEST_SRC drivers/io/key_matrix_tests.c)

    elseif (TEST_CASE STREQUAL "ws2812_tests")
        list(APPEND ENABLED_MODULES ws2812 uart usb_cdc)
        set(TEST_SRC drivers/io/ws2812_tests.c)

    elseif (TEST_CASE STREQUAL "xpt2046_tests")
        list(APPEND ENABLED_MODULES xpt2046 spi_soft uart usb_cdc)
        set(TEST_SRC drivers/io/xpt2046_tests.c)
    
    # motor
    elseif (TEST_CASE STREQUAL "bldc_motor_esc_encoder_tests")
        list(APPEND ENABLED_MODULES bldc_motor_esc_encoder uart delay)
        set(TEST_SRC drivers/tests/bldc_motor_esc_encoder_tests.c)

    elseif (TEST_CASE STREQUAL "servo_motor_tests")
        list(APPEND ENABLED_MODULES servo_motor uart)
        set(TEST_SRC drivers/tests/servo_motor_tests.c)

    elseif (TEST_CASE STREQUAL "stepper_motor_tests")
        list(APPEND ENABLED_MODULES stepper_motor uart delay)
        set(TEST_SRC drivers/tests/stepper_motor_tests.c)

    # sensor
    elseif (TEST_CASE STREQUAL "bh1750_tests")
        list(APPEND ENABLED_MODULES bh1750 uart delay usb_cdc)
        set(TEST_SRC drivers/sensor/bh1750_tests.c)

    elseif (TEST_CASE STREQUAL "dht11_tests")
        list(APPEND ENABLED_MODULES delay dht11 uart usb_cdc) # UART for printing
        set(TEST_SRC drivers/sensor/dht11_tests.c)

    elseif (TEST_CASE STREQUAL "ds18b20_tests")
        list(APPEND ENABLED_MODULES ds18b20 uart delay usb_cdc)
        set(TEST_SRC drivers/sensor/ds18b20_tests.c)

    elseif (TEST_CASE STREQUAL "hc_sr04_tests")
        list(APPEND ENABLED_MODULES hc_sr04 uart delay usb_cdc)
        set(TEST_SRC drivers/sensor/hc_sr04_tests.c)

    elseif (TEST_CASE STREQUAL "light_sensor_tests")
        list(APPEND ENABLED_MODULES light_sensor uart delay usb_cdc)
        set(TEST_SRC drivers/sensor/light_sensor_tests.c)

    elseif (TEST_CASE STREQUAL "mpu6050_tests")
        list(APPEND ENABLED_MODULES mpu6050 uart delay usb_cdc)
        set(TEST_SRC drivers/sensor/mpu6050_tests.c)

    elseif (TEST_CASE STREQUAL "potentiometer_tests")
        list(APPEND ENABLED_MODULES potentiometer uart delay usb_cdc)
        set(TEST_SRC drivers/sensor/potentiometer_tests.c)

    # storage
    elseif (TEST_CASE STREQUAL "at24cxx_tests")
        list(APPEND ENABLED_MODULES at24cxx uart delay usb_cdc)
        set(TEST_SRC drivers/storage/at24cxx_tests.c)

    elseif (TEST_CASE STREQUAL "internal_flash_tests")
        list(APPEND ENABLED_MODULES internal_flash uart delay usb_cdc)
        set(TEST_SRC drivers/storage/internal_flash_tests.c)

    elseif (TEST_CASE STREQUAL "sd_card_spi_tests")
        list(APPEND ENABLED_MODULES sd_card_spi uart delay usb_cdc)
        set(TEST_SRC drivers/storage/sd_card_spi_tests.c)

    elseif (TEST_CASE STREQUAL "w25qxx_tests")
        list(APPEND ENABLED_MODULES w25qxx uart delay usb_cdc)
        set(TEST_SRC drivers/storage/w25qxx_tests.c)

    # system
    elseif (TEST_CASE STREQUAL "delay_tests")
        list(APPEND ENABLED_MODULES delay uart usb_cdc)
        set(TEST_SRC drivers/system/delay_tests.c)
    elseif (TEST_CASE STREQUAL "rtc_driver_tests")
        list(APPEND ENABLED_MODULES rtc uart usb_cdc)
        set(TEST_SRC drivers/system/rtc_driver_tests.c)
    elseif (TEST_CASE STREQUAL "watchdog_tests")
        list(APPEND ENABLED_MODULES watchdog uart usb_cdc)
        set(TEST_SRC drivers/system/watchdog_tests.c)



    else()
        message(STATUS "TEST_CASE ${TEST_CASE} not migrated to new system yet.")
    endif()

    if (TEST_SRC)
        target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${TEST_SRC})
    endif()
endif()

# ==========================================
# 4. Dependency Resolution Logic
# ==========================================

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
# 5. Create Final User Library
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
