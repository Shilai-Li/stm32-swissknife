# applications CMakeLists.txt
# Application modules

define_module(freertos_producer_consumer
    SOURCES 
        freertos_producer_consumer/app_main.c
        freertos_producer_consumer/producer_task.c
        freertos_producer_consumer/consumer_task.c
    INCLUDES 
        ${CMAKE_CURRENT_SOURCE_DIR}/freertos_producer_consumer
    DEPENDS 
        uart led usb_cdc
)
