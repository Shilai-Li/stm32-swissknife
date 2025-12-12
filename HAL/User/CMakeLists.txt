add_library(User OBJECT
        Components/servo.c

        Drivers/delay_driver.c
        Drivers/dht11_driver.c
        Drivers/key_driver.c
        Drivers/led_driver.c
        Drivers/motor_driver.c
        Drivers/buzzer_driver.c
        Drivers/ssd1306.c
        Drivers/ssd1306_custom.c
        Drivers/ssd1306_fonts.c
        Drivers/ssd1306_fonts_custom.c
        Drivers/ssd1306_tests.c
        Drivers/uart_driver.c

        Middlewares/Algorithms/pid.c

        Middlewares/Protocols/crsf.c
)

target_include_directories(User PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Components)

target_include_directories(User PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Drivers)

target_include_directories(User PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/algorithms)

target_include_directories(User PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/Protocols)

target_link_libraries(User PUBLIC stm32cubemx)
