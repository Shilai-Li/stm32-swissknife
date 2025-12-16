/**
 * @file letter-shell_port.h
 * @brief Letter Shell Port for STM32 (UART via drivers/uart.h)
 */

#ifndef LETTER_SHELL_PORT_H
#define LETTER_SHELL_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "letter-shell/shell.h"

extern Shell shell;

/**
 * @brief Initialize Letter Shell
 */
void Shell_Port_Init(void);

/**
 * @brief Main Loop Task
 *        Polls UART RingBuffer and feeds Shell.
 *        Call this in your main while(1) loop.
 */
void Shell_Task(void);

#ifdef __cplusplus
}
#endif

#endif
