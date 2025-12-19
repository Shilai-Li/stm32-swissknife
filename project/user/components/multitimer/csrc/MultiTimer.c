/**
 * @file MultiTimer.c
 * @brief MultiTimer library implementation
 * @author 0x1abin
 * @license MIT
 */
#include "MultiTimer.h"

// Timer list head
static MultiTimer* head_handle = NULL;

/**
 * @brief Initialize timer
 */
void MultiTimerInit(MultiTimer* timer, uint32_t period, MultiTimerCallback_t callback, void* userData) {
    if (!timer) return;
    timer->next = NULL;
    timer->period = period;
    timer->callback = callback;
    timer->userData = userData;
    timer->deadline = 0;
}

/**
 * @brief Start timer
 */
int MultiTimerStart(MultiTimer* timer, uint32_t startTime, uint32_t period) {
    if (!timer) return -1;
    
    // Stop if already running to re-insert correctly
    MultiTimerStop(timer);
    
    if (period > 0) {
        timer->period = period;
    }
    
    timer->deadline = startTime + timer->period;
    
    // Insert into linked list (sorted by deadline could be better, but head insert is O(1))
    // simpler implementation: insert at head
    timer->next = head_handle;
    head_handle = timer;
    
    return 0;
}

/**
 * @brief Stop timer
 */
void MultiTimerStop(MultiTimer* timer) {
    if (!timer || !head_handle) return;
    
    MultiTimer** curr = &head_handle;
    while (*curr) {
        if (*curr == timer) {
            *curr = timer->next;
            timer->next = NULL;
            return;
        }
        curr = &(*curr)->next;
    }
}

/**
 * @brief Check if active
 */
int MultiTimerIsActive(MultiTimer* timer) {
    if (!timer) return 0;
    MultiTimer* curr = head_handle;
    while (curr) {
        if (curr == timer) return 1;
        curr = curr->next;
    }
    return 0;
}

/**
 * @brief Timer yield function
 */
void MultiTimerYield(void) {
    MultiTimer* target;
    uint32_t now = MultiTimerTicks();

    // Iterate through list
    // Note: If callback removes/adds timers, we need to be careful.
    // Safe way: restart list scan is simple but inefficient. 
    // Or just be careful.
    
    MultiTimer** curr = &head_handle;
    while (*curr) {
        target = *curr;
        // Check for timeout
        // Handle wrapping: (now - deadline) < 2^31
        if ((now - target->deadline) < 0x80000000) {
            
            // Remove from list if one-shot
            if (target->period == 0) {
                *curr = target->next; // Remove
                target->next = NULL;
            } else {
                // Periodic: Update deadline
                target->deadline = now + target->period;
                // Keep in list, move to next
                curr = &target->next;
            }

            // Call callback
            if (target->callback) {
                target->callback(target, target->userData);
            }
        } else {
             curr = &target->next;
        }
    }
}
