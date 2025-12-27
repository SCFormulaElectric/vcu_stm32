#ifndef LIGHT_CONTROLLER_TASK_H
#define LIGHT_CONTROLLER_TASK_H

#include "app.h"
#include "watchdog_tasks_defs.h"
// I'm including the statemachine.h because it has the necessary defines 
// for thresholds and includes
#include "state_machine_task.h"

#define LIGHT_CONTROLLER_DELAY_MS           100
#define LIGHT_CONTROLLER_STACK_SIZE         KILOBYTE
void light_controller_task(void *argument);


task_entry_t create_light_controller_task(void);

#endif /* LIGHT_CONTROLLER_TASK_H */

