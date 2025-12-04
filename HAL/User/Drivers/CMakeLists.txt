add_library(UserDrivers OBJECT
    Src/buzzer_driver.c
    Src/crsf.c
    Src/delay_driver.c
    Src/dht11_driver.c
    Src/key_driver.c
    Src/led_driver.c
    Src/motor_driver.c
    Src/ssd1306.c
    Src/ssd1306_custom.c
    Src/ssd1306_fonts.c
    Src/ssd1306_fonts_custom.c
    Src/ssd1306_tests.c
    Src/uart_driver.c
)

target_include_directories(UserDrivers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Inc)

target_link_libraries(UserDrivers PUBLIC stm32cubemx)
