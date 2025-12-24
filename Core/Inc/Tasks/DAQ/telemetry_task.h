#ifndef TELEMETRY_TASK_H
#define TELEMETRY_TASK_H

#include "FreeRTOS.h"
#include "task.h"


void telemetry_task(void *argument);


task_entry_t create_telemetry_task(void);

#endif /* TELEMETRY_TASK_H */

