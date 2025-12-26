#include "key_matrix.h"

// Hardware Config Storage
static GPIO_TypeDef** _row_ports;
static uint16_t*      _row_pins;
static uint8_t        _R;

static GPIO_TypeDef** _col_ports;
static uint16_t*      _col_pins;
static uint8_t        _C;

// State Bitmap (1 = Pressed, 0 = Released)
// Support up to 16 rows * 32 cols bitmask.
static uint32_t _state_matrix[MATRIX_MAX_ROWS]; 
// static uint32_t _last_matrix[MATRIX_MAX_ROWS]; // Not used currently

// Pending Event
static MatrixEvent_t _last_event = {0};

void KeyMatrix_Init(GPIO_TypeDef** row_ports, uint16_t* row_pins, uint8_t num_rows,
                    GPIO_TypeDef** col_ports, uint16_t* col_pins, uint8_t num_cols)
{
    _row_ports = row_ports;
    _row_pins  = row_pins;
    _R         = (num_rows > MATRIX_MAX_ROWS) ? MATRIX_MAX_ROWS : num_rows;
    
    _col_ports = col_ports;
    _col_pins  = col_pins;
    _C         = (num_cols > MATRIX_MAX_COLS) ? MATRIX_MAX_COLS : num_cols;
    
    // Init Rows as OUTPUT Open-Drain (or Push-Pull, but OD is safer)
    // Actually, Input PU on Cols + Output PP on Rows is standard.
    // Row Idle = HIGH. Active Scan = LOW.
    GPIO_InitTypeDef w = {0};
    w.Mode = GPIO_MODE_OUTPUT_PP;
    w.Pull = GPIO_NOPULL;
    w.Speed = GPIO_SPEED_FREQ_LOW;
    
    for(int i=0; i<_R; i++) {
         HAL_GPIO_Init(_row_ports[i], &w);
         HAL_GPIO_WritePin(_row_ports[i], _row_pins[i], GPIO_PIN_SET); // Idle High
    }
    
    // Init Cols as INPUT PULLUP
    GPIO_InitTypeDef r = {0};
    r.Mode = GPIO_MODE_INPUT;
    r.Pull = GPIO_PULLUP;
    r.Speed = GPIO_SPEED_FREQ_LOW;
    
    for(int i=0; i<_C; i++) {
        HAL_GPIO_Init(_col_ports[i], &r);
    }
}

bool KeyMatrix_Scan(void)
{
    bool event_detected = false;
    
    for(int r=0; r<_R; r++) {
        // 1. Activate Row (Pull Low)
        HAL_GPIO_WritePin(_row_ports[r], _row_pins[r], GPIO_PIN_RESET);
        
        // 2. Short Delay for signal stabilization
        // for(volatile int d=0; d<10; d++); 
        
        // 3. Read Cols
        for(int c=0; c<_C; c++) {
            // Active Low (PullUp) -> Low means Pressed
            bool is_pressed = (HAL_GPIO_ReadPin(_col_ports[c], _col_pins[c]) == GPIO_PIN_RESET);
            
            // Check changes against last state
            bool was_pressed = (_state_matrix[r] >> c) & 0x01;
            
            if (is_pressed != was_pressed) {
                // Update State
                if (is_pressed) _state_matrix[r] |= (1UL << c);
                else            _state_matrix[r] &= ~(1UL << c);
                
                // Register Event (Only latest one strictly recorded)
                _last_event.row = r;
                _last_event.col = c;
                _last_event.pressed = is_pressed;
                event_detected = true;
            }
        }
        
        // 4. Deactivate Row (Return High)
        HAL_GPIO_WritePin(_row_ports[r], _row_pins[r], GPIO_PIN_SET);
    }
    
    return event_detected;
}

MatrixEvent_t KeyMatrix_GetEvent(void)
{
    return _last_event;
}

char KeyMatrix_MapChar(const char *map, MatrixEvent_t evt)
{
    if (!map) return 0;
    // Map index = row * TotalCols + col
    // Assuming map is flat array properly sized
    return map[evt.row * _C + evt.col];
}
