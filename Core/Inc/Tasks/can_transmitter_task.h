#ifndef can_transmitter_TASK_H
#define can_transmitter_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "app.h"

// Task function
void can_transmitter_task(void *argument);

// Function to create the task and return its handle
TaskHandle_t create_can_transmitter_task(void);

#endif /* can_transmitter_TASK_H */
