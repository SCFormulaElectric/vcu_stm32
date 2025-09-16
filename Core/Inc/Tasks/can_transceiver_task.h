#ifndef CAN_TRANSCEIVER_TASK_H
#define CAN_TRANSCEIVER_TASK_H

#include "FreeRTOS.h"
#include "task.h"

// Task function
void can_transceiver_task(void *argument);

// Function to create the task and return its handle
TaskHandle_t create_can_transceiver_task(void);

#endif /* CAN_TRANSCEIVER_TASK_H */
