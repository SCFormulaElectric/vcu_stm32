#ifndef BRAKE_PEDAL_PLAUSIBILITY_CHECK_TASK_H
#define BRAKE_PEDAL_PLAUSIBILITY_CHECK_TASK_H

#include "FreeRTOS.h"
#include "task.h"

// Task function
void brake_pedal_plausibility_check_task(void *argument);

// Function to create the task and return its handle
TaskHandle_t create_brake_pedal_plausibility_check_task(void);

#endif /* BRAKE_PEDAL_PLAUSIBILITY_CHECK_TASK_H */

