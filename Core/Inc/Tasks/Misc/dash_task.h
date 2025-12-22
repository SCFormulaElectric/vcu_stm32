#ifndef DASH_TASK_H
#define DASH_TASK_H

#include "FreeRTOS.h"
#include "task.h"

// Task function
void dash_task(void *argument);

// Function to create the task and return its handle
TaskHandle_t create_dash_task(void);

#endif /* DASH_TASK_H */

