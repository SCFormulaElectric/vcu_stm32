#ifndef INDEPENDENT_WATCHDOG_TASK_H
#define INDEPENDENT_WATCHDOG_TASK_H

#include "FreeRTOS.h"
#include "task.h"


void independent_watchdog_task(void *argument);


task_entry_t create_independent_watchdog_task(void);

#endif /* INDEPENDENT_WATCHDOG_TASK_H */

