#ifndef INDEPENDENT_WATCHDOG_TASK_H
#define INDEPENDENT_WATCHDOG_TASK_H

#include "app.h"
#include "watchdog_tasks_defs.h"

#define IDWG_WINDOW_MS 3000
#define IDWG_STACK_SIZE KILOBYTE
void independent_watchdog_task(void *argument);
task_entry_t create_independent_watchdog_task(void);

#endif /* INDEPENDENT_WATCHDOG_TASK_H */

