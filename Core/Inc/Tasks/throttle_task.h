#ifndef throttle_task_H
#define throttle_task_H

#include "FreeRTOS.h"
#include "task.h"
#include "app.h"

// Task function
void throttle_task(void *argument);

// Function to create the task and return its handle
TaskHandle_t create_throttle_task(void);

bool validate_signal(uint16_t input_1, uint16_t input_2);
bool bounds_check();
bool apps_check();

#endif /* throttle_task_H */
