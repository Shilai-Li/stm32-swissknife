/**
 * @file moving_average.h
 * @brief Generic Moving Average Filter Implementation
 * @details Pure C implementation, decoupled from hardware.
 */

#ifndef MOVING_AVERAGE_H
#define MOVING_AVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Moving Average Filter Handle
 * @note  User must allocate the buffer memory externally to allow flexible sizing.
 */
typedef struct {
    uint16_t   *buffer;     /*!< Pointer to external buffer array */
    uint32_t    sum;        /*!< Running sum of buffer elements */
    uint16_t    size;       /*!< Total size of the buffer (Window Size) */
    uint16_t    index;      /*!< Current insertion index */
    bool        filled;     /*!< Flag: has the buffer filled up at least once? */
} MovingAverage_Handle_t;

/**
 * @brief  Initialize the filter
 * @param  h  Filter Handle
 * @param  buffer  Pointer to a uint16_t array allocated by user
 * @param  size    Size of the array (Window Size)
 */
void MovingAverage_Init(MovingAverage_Handle_t *h, uint16_t *buffer, uint16_t size);

/**
 * @brief  Update the filter with a new value
 * @param  h     Filter Handle
 * @param  input New raw value
 * @return Current filtered average
 */
uint16_t MovingAverage_Update(MovingAverage_Handle_t *h, uint16_t input);

/**
 * @brief  Reset the filter (clear history)
 */
void MovingAverage_Reset(MovingAverage_Handle_t *h);

/**
 * @brief  Get the current average without adding new data
 */
uint16_t MovingAverage_Get(MovingAverage_Handle_t *h);

#ifdef __cplusplus
}
#endif

#endif // MOVING_AVERAGE_H
