#ifndef STATE_MACHINE_TASK_H
#define STATE_MACHINE_TASK_H

#include "FreeRTOS.h"
#include "task.h"


void state_machine_task(void *argument);


TaskHandle_t create_state_machine_task(void);

#endif /* STATE_MACHINE_TASK_H */

