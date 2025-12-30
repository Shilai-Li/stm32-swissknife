# components CMakeLists.txt
# Third-party components and libraries

define_module(letter_shell
    SOURCES 
        letter-shell/csrc/shell.c 
        letter-shell/csrc/shell_ext.c
        letter-shell/letter-shell_port.c
    INCLUDES 
        ${CMAKE_CURRENT_SOURCE_DIR}/letter-shell
        ${CMAKE_CURRENT_SOURCE_DIR}/letter-shell/csrc
    DEPENDS uart
)
