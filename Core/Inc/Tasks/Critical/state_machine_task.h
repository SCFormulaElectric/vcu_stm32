#ifndef STATE_MACHINE_TASK_H
#define STATE_MACHINE_TASK_H

#include "FreeRTOS.h"
#include "task.h"

// Task function
void state_machine_task(void *argument);

// Function to create the task and return its handle
TaskHandle_t create_state_machine_task(void);

#endif /* STATE_MACHINE_TASK_H */

