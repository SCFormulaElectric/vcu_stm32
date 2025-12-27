#ifndef LIGHT_CONTROLLER_TASK_H
#define LIGHT_CONTROLLER_TASK_H

#include "app.h"
#include "watchdog_tasks_defs.h"
void light_controller_task(void *argument);


task_entry_t create_light_controller_task(void);

#endif /* LIGHT_CONTROLLER_TASK_H */

