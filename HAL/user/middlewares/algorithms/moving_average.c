/**
 * @file moving_average.c
 * @brief Generic Moving Average Filter Implementation
 */

#include "moving_average.h"
#include <string.h>

void MovingAverage_Init(MovingAverage_Handle_t *h, uint16_t *buffer, uint16_t size) {
    if (!h || !buffer || size == 0) return;
    
    h->buffer = buffer;
    h->size = size;
    h->index = 0;
    h->sum = 0;
    h->filled = false;
    
    // Clear buffer
    memset(h->buffer, 0, size * sizeof(uint16_t));
}

uint16_t MovingAverage_Update(MovingAverage_Handle_t *h, uint16_t input) {
    if (!h || !h->buffer || h->size == 0) return input;

    // Subtract the oldest value from sum
    // (If not filled yet, buffer usually contains 0s or old garbage, 
    // strictly we should track count. But for simplicity, we use RingOverwrite model)
    h->sum -= h->buffer[h->index];

    // Store new value
    h->buffer[h->index] = input;
    
    // Add new value to sum
    h->sum += input;

    // Advance index
    h->index++;
    if (h->index >= h->size) {
        h->index = 0;
        h->filled = true; // Once we wrap around, the buffer is full of valid data
    }

    // Calculate Average
    if (h->filled) {
        return (uint16_t)(h->sum / h->size);
    } else {
        // Optimization: If not filled, average only over valid samples?
        // Or just return Sum / Index?
        if (h->index == 0) return input; // Should happen only immediately after init
        return (uint16_t)(h->sum / h->index);
    }
}

void MovingAverage_Reset(MovingAverage_Handle_t *h) {
    if (!h) return;
    h->index = 0;
    h->sum = 0;
    h->filled = false;
    memset(h->buffer, 0, h->size * sizeof(uint16_t));
}

uint16_t MovingAverage_Get(MovingAverage_Handle_t *h) {
    if (!h || h->size == 0) return 0;
    if (h->filled) {
        return (uint16_t)(h->sum / h->size);
    } else {
        if (h->index == 0) return 0;
        return (uint16_t)(h->sum / h->index);
    }
}
