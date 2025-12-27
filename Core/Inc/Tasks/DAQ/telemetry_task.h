#ifndef TELEMETRY_TASK_H
#define TELEMETRY_TASK_H

#include "app.h"
#include "watchdog_tasks_defs.h"

#define TELEMETRY_STACK_SIZE            2*KILOBYTE
void telemetry_task(void *argument);


task_entry_t create_telemetry_task(void);

#endif /* TELEMETRY_TASK_H */

