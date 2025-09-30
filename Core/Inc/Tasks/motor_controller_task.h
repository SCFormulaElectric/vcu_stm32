#ifndef MOTOR_CONTROLLER_TASK_H
#define MOTOR_CONTROLLER_TASK_H
#include "../../Inc/app.h"
#include "FreeRTOS.h"
#include "task.h"

#define Forward  1
#define Backward 0

enum {
    STATE_DISABLE,
    STATE_ENABLE,
    STATE_WAIT,
} state_t;

// Task function
void motor_controller_task(void *argument);

can_message_t create_motor_controller_command(uint16_t torque, uint16_t speed, uint16_t torque_limit, 
    bool direction, bool inverter_en, bool inverter_discharge, bool speed_mode_enable);

// Function to create the task and return its handle
TaskHandle_t create_motor_controller_task(void);

#endif /* MOTOR_CONTROLLER_TASK_H */
