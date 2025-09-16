#ifndef TELEMETRY_TASK_H
#define TELEMETRY_TASK_H

#include "FreeRTOS.h"
#include "task.h"

// Task function
void telemetry_task(void *argument);

// Function to create the task and return its handle
TaskHandle_t create_telemetry_task(void);

#endif /* TELEMETRY_TASK_H */
