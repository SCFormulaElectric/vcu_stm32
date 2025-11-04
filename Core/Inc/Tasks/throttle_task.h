#ifndef throttle_task_H
#define throttle_task_H

#include "FreeRTOS.h"
#include "task.h"
#include "app.h"
#include <stdint.h>
#include "semphr.h"
#define MAX_PEDAL_DIFFERENCE 100
#define THROTTLE_FAULT_TOLERANCE 150

// Global variable for other tasks to read
extern volatile uint16_t throttle_level; 

// Task function
void throttle_task(void *argument);

// Function to create the task and return its handle
TaskHandle_t create_throttle_task(void);
bool apps_check(uint16_t level1, uint16_t level2);
bool validate_signal(uint16_t input_1, uint16_t input_2);
bool within_bounds(uint16_t level1, uint16_t level2);

#endif /* throttle_task_H */
