#ifndef MOTOR_CONTROLLER_TASK_H
#define MOTOR_CONTROLLER_TASK_H

#include "app.h"
#include "watchdog_tasks_defs.h"
#include "throttle_task.h"
#include "can_bus.h"
#include "motor_control.h"

#define Forward  1
#define Backward 0
#define MC_QUEUE_WAIT_MS 5
#define THROTTLE_DEADZONE 20
#define motor_control_interval 25
#define MAX_TORQUE 300 

#define MC_STACK_SIZE 2*KILOBYTE

typedef enum {
    STATE_DISABLE,
    STATE_ENABLE,
    STATE_WAIT,
} state_t;


void motor_controller_task(void *argument);
can_tx_message_t create_motor_controller_command(uint16_t torque, uint16_t speed, uint16_t torque_limit, 
    uint8_t direction, uint8_t inverter_en, uint8_t inverter_discharge, uint8_t speed_mode_enable);


task_entry_t create_motor_controller_task(void);

#endif /* MOTOR_CONTROLLER_TASK_H */

