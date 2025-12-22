#ifndef DEFAULT_TASK_TASK_H
#define DEFAULT_TASK_TASK_H

#include "FreeRTOS.h"
#include "task.h"

// Task function
void default_task_task(void *argument);

// Function to create the task and return its handle
TaskHandle_t create_default_task_task(void);

#endif /* DEFAULT_TASK_TASK_H */

