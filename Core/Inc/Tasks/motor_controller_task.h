#ifndef MOTOR_CONTROLLER_TASK_H
#define MOTOR_CONTROLLER_TASK_H

#include "FreeRTOS.h"
#include "task.h"

// Task function
void motor_controller_task(void *argument);

// Function to create the task and return its handle
TaskHandle_t create_motor_controller_task(void);

#endif /* MOTOR_CONTROLLER_TASK_H */
