#ifndef __KEY_MATRIX_H
#define __KEY_MATRIX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdbool.h>

#define MATRIX_MAX_ROWS 16  // Support up to 16 Rows
#define MATRIX_MAX_COLS 32  // Support up to 32 Columns (using uint32_t bitmask)

// Event Data
typedef struct {
    uint8_t row;
    uint8_t col;
    bool    pressed; // True=Press, False=Release
} MatrixEvent_t;

/**
 * @brief Initialize Matrix Keyboard
 * @param row_ports Array of GPIO Ports for Rows
 * @param row_pins  Array of GPIO Pins for Rows
 * @param num_rows  Number of Rows
 * @param col_ports Array of GPIO Ports for Cols
 * @param col_pins  Array of GPIO Pins for Cols
 * @param num_cols  Number of Cols
 */
void KeyMatrix_Init(GPIO_TypeDef** row_ports, uint16_t* row_pins, uint8_t num_rows,
                    GPIO_TypeDef** col_ports, uint16_t* col_pins, uint8_t num_cols);

/**
 * @brief Scan the matrix. Call this periodically (e.g., 20ms).
 * @return True if a new event occurred
 */
bool KeyMatrix_Scan(void);

/**
 * @brief Get the latest event.
 * @return Event struct. If no event, .pressed will likely be false (but check return of Scan)
 */
MatrixEvent_t KeyMatrix_GetEvent(void);

/**
 * @brief  Helper: Map (row,col) to a character
 * @param  map: 1D array representing the matrix symbols
 * @return Character at current event coordinates
 */
char KeyMatrix_MapChar(const char *map, MatrixEvent_t evt);

#ifdef __cplusplus
}
#endif

#endif
