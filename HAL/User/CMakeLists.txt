# Test Case Selection
set(TEST_CASE "servo_test" CACHE STRING "Select which test to run (CRSF, UART, BUZZER, DELAY, DHT11, KEY, LED, MOTOR, SSD1306, SSD1306_CUSTOM)")
message("Selected Test Case: ${TEST_CASE}")

# Initialize source list
# You can add common files here that are ALWAYS needed
set(USER_SOURCES 
    Drivers/delay_driver.c
    Drivers/uart_driver.c
)

# Applications
if(TEST_CASE STREQUAL "dfaf")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE Applications/)
endif ()

# Components
if (TEST_CASE STREQUAL "servo_test")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE Components/Tests/servo_test.c)
    list(APPEND USER_SOURCES Components/servo.c)
elseif (TEST_CASE STREQUAL "asdfas")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE Components/Tests)
endif ()

# Drivers
if (TEST_CASE STREQUAL "CRSF")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE Drivers/Tests/crsf_protocol_test.c)
    list(APPEND USER_SOURCES Middlewares/Protocols/crsf.c)
elseif(TEST_CASE STREQUAL "UART")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE Drivers/Tests/uart_driver_test.c)
    # UART driver is already in common sources
elseif(TEST_CASE STREQUAL "BUZZER")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE Drivers/Tests/buzzer_driver_test.c)
    list(APPEND USER_SOURCES Drivers/buzzer_driver.c)
elseif(TEST_CASE STREQUAL "DELAY")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE Drivers/Tests/delay_driver_test.c)
    # Delay driver is already in common sources
elseif(TEST_CASE STREQUAL "DHT11")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE Drivers/Tests/dht11_driver_test.c)
    list(APPEND USER_SOURCES Drivers/dht11_driver.c)
elseif(TEST_CASE STREQUAL "KEY")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE Drivers/Tests/key_driver_test.c)
    list(APPEND USER_SOURCES Drivers/key_driver.c)
elseif(TEST_CASE STREQUAL "LED")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE Drivers/Tests/led_driver_test.c)
    list(APPEND USER_SOURCES Drivers/led_driver.c)
elseif(TEST_CASE STREQUAL "MOTOR")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE Drivers/Tests/motor_driver_test.c)
    list(APPEND USER_SOURCES 
        Drivers/motor_driver.c
        Middlewares/Algorithms/pid.c
    )
elseif(TEST_CASE STREQUAL "SSD1306")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE Drivers/Tests/ssd1306_test.c)
    list(APPEND USER_SOURCES 
        Drivers/ssd1306.c
        Drivers/ssd1306_fonts.c
    )
elseif(TEST_CASE STREQUAL "SSD1306_CUSTOM")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE Drivers/Tests/ssd1306_custom_test.c)
    list(APPEND USER_SOURCES 
        Drivers/ssd1306.c
        Drivers/ssd1306_fonts.c
        Drivers/ssd1306_custom.c
        Drivers/ssd1306_fonts_custom.c
    )
else()
    # Default to CRSF if nothing matches
    message(STATUS "No specific TEST_CASE matched, defaulting to CRSF")
    target_sources(${CMAKE_PROJECT_NAME} PRIVATE Drivers/Tests/crsf_protocol_test.c)
    list(APPEND USER_SOURCES Middlewares/Protocols/crsf.c)
endif()

# Create the library with the selected sources
add_library(User OBJECT ${USER_SOURCES})

# Include directories (kept global for simplicity, or can be conditional too)
target_include_directories(User PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Components
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/algorithms
    ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Protocols
)

target_link_libraries(User PUBLIC stm32cubemx)
