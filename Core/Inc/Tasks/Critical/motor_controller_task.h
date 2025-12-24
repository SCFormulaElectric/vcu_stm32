#ifndef MOTOR_CONTROLLER_TASK_H
#define MOTOR_CONTROLLER_TASK_H

#include "app.h"
#include "throttle_task.h"
#include "can_bus.h"

#define Forward  1
#define Backward 0

typedef enum {
    STATE_DISABLE,
    STATE_ENABLE,
    STATE_WAIT,
} state_t;


void motor_controller_task(void *argument);

can_message_t create_motor_controller_command(uint16_t torque, uint16_t speed, uint16_t torque_limit, 
    uint_8 direction, uint_8 inverter_en, uint_8 inverter_discharge, uint_8 speed_mode_enable);


TaskHandle_t create_motor_controller_task(void);

#endif /* MOTOR_CONTROLLER_TASK_H */

