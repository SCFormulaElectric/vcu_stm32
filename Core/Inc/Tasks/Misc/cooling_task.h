#ifndef COOLING_TASK_H
#define COOLING_TASK_H

#include "FreeRTOS.h"
#include "task.h"

// Task function
void cooling_task(void *argument);

// Function to create the task and return its handle
TaskHandle_t create_cooling_task(void);

#endif /* COOLING_TASK_H */

