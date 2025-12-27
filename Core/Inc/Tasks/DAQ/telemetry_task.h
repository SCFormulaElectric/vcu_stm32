#ifndef TELEMETRY_TASK_H
#define TELEMETRY_TASK_H

#include "app.h"
#include "watchdog_tasks_defs.h"

void telemetry_task(void *argument);


task_entry_t create_telemetry_task(void);

#endif /* TELEMETRY_TASK_H */

