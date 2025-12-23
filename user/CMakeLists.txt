# Test Case Selection
set(TEST_CASE "buzzer" CACHE STRING "Select which test to run" FORCE)
message("Selected Test Case: ${TEST_CASE}")

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
endmacro()

# ==========================================
# 1. System Drivers (Base)
# ==========================================

define_module(delay 
    SOURCES drivers/system/delay.c
    INCLUDES drivers/system
)

define_module(uart
    SOURCES drivers/communication/uart.c
    INCLUDES drivers/communication
)

define_module(i2c_soft
    SOURCES drivers/interface/soft_i2c.c
    INCLUDES drivers/interface
)

# ==========================================
# 2. io
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
# 3. Sensor Drivers
# ==========================================

define_module(dht11
    SOURCES drivers/sensor/dht11.c
    INCLUDES drivers/sensor
    DEPENDS delay
)

define_module(mpu6050
    SOURCES drivers/sensor/mpu6050.c
    INCLUDES drivers/sensor
    DEPENDS i2c_soft
)

define_module(bh1750
    SOURCES drivers/sensor/light_sensor.c
    INCLUDES drivers/sensor
    DEPENDS i2c_soft
)

# ==========================================
# 4. Display Drivers
# ==========================================

define_module(ssd1306
    SOURCES 
        drivers/display/afiskon-stm32-ssd1306/ssd1306.c
        drivers/display/afiskon-stm32-ssd1306/ssd1306_fonts.c
    INCLUDES 
        drivers/display/afiskon-stm32-ssd1306
    DEPENDS i2c_soft
)

# ==========================================
# 5. Components
# ==========================================

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

define_module(freertos_os
    SOURCES
        components/freertos/csrc/tasks.c
        components/freertos/csrc/queue.c
        components/freertos/csrc/list.c
        components/freertos/csrc/timers.c
        components/freertos/csrc/event_groups.c
        components/freertos/csrc/stream_buffer.c
        components/freertos/csrc/portable/GCC/ARM_CM3/port.c
        components/freertos/csrc/portable/MemMang/heap_4.c
        components/freertos/freertos_port.c
    INCLUDES
        components/freertos
        components/freertos/csrc/include
        components/freertos/csrc/portable/GCC/ARM_CM3
)

# ==========================================
# 6. Dependency Resolution Logic
# ==========================================

# If ENABLED_MODULES is not set by parent project, determine it from TEST_CASE
if (NOT DEFINED ENABLED_MODULES)
    set(ENABLED_MODULES "")
    set(TEST_SRC "")

    if (TEST_CASE STREQUAL "led")
        list(APPEND ENABLED_MODULES led)
        set(TEST_SRC drivers/tests/led_tests.c)

    elseif (TEST_CASE STREQUAL "buzzer")
        list(APPEND ENABLED_MODULES buzzer uart)
        set(TEST_SRC drivers/tests/buzzer_tests.c)

    elseif (TEST_CASE STREQUAL "dht11")
        list(APPEND ENABLED_MODULES dht11 uart) # UART for printing
        set(TEST_SRC drivers/tests/dht11_test.c)

    elseif (TEST_CASE STREQUAL "mpu6050")
        list(APPEND ENABLED_MODULES mpu6050 uart)
        set(TEST_SRC drivers/tests/mpu6050_tests.c)

    elseif (TEST_CASE STREQUAL "shell")
        list(APPEND ENABLED_MODULES letter_shell)
        set(TEST_SRC components/tests/letter-shell_tests.c)
    
    elseif (TEST_CASE STREQUAL "freertos")
        # NOTE: If CubeMX provides FreeRTOS, don't include freertos_os here!
        list(APPEND ENABLED_MODULES freertos_os led uart) 
        set(TEST_SRC components/tests/freertos_tests.c)

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
