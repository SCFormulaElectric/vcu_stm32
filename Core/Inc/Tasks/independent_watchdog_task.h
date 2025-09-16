#ifndef INDEPENDENT_WATCHDOG_TASK_H
#define INDEPENDENT_WATCHDOG_TASK_H

#include "FreeRTOS.h"
#include "task.h"

// Task function
void independent_watchdog_task(void *argument);

// Function to create the task and return its handle
TaskHandle_t create_independent_watchdog_task(void);

#endif /* INDEPENDENT_WATCHDOG_TASK_H */
