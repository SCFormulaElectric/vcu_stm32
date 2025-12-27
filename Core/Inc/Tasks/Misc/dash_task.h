#ifndef DASH_TASK_H
#define DASH_TASK_H

#include "app.h"
#include "watchdog_tasks_defs.h"

#define DASHBOARD_STACK_SIZE    KILOBYTE
void dash_task(void *argument);


task_entry_t create_dash_task(void);

#endif /* DASH_TASK_H */

