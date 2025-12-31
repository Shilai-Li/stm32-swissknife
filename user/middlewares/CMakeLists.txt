# middlewares CMakeLists.txt
# Middleware modules for algorithms and protocols

# ==========================================
# Algorithms
# ==========================================

define_module(moving_average
    SOURCES algorithms/moving_average.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/algorithms
)

define_module(pid
    SOURCES algorithms/pid.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/algorithms
)

# ==========================================
# Protocols
# ==========================================

define_module(crsf
    SOURCES protocols/crsf.c
    INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/protocols
)
