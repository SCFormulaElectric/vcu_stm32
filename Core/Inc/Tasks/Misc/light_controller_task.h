#ifndef LIGHT_CONTROLLER_TASK_H
#define LIGHT_CONTROLLER_TASK_H

#include "FreeRTOS.h"
#include "task.h"

// Task function
void light_controller_task(void *argument);

// Function to create the task and return its handle
TaskHandle_t create_light_controller_task(void);

#endif /* LIGHT_CONTROLLER_TASK_H */

