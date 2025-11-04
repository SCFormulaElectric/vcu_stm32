#ifndef throttle_task_H
#define throttle_task_H

#include "FreeRTOS.h"
#include "task.h"
#include "app.h"
#include <stdint.h>
#include "semphr.h"
#define MAX_PEDAL_DIFFERENCE 100
#define THROTTLE_FAULT_TOLERANCE 150

typedef struct {
    uint16_t throttle_1;
    uint16_t throttle_2;
} throttle_t;

// Allow other tasks to read throtle levels
extern volatile throttle_t throttle_levels; 
extern SemaphoreHandle_t throttle_mutex;

// Task function
void throttle_task(void *argument);

// Function to create the task and return its handle
TaskHandle_t create_throttle_task(void);
bool apps_check(uint16_t level1, uint16_t level2);
bool validate_signal(uint16_t input_1, uint16_t input_2);
bool within_bounds(uint16_t level1, uint16_t level2);

#endif /* throttle_task_H */
