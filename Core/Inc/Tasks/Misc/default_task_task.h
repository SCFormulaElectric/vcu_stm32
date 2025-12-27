#ifndef DEFAULT_TASK_TASK_H
#define DEFAULT_TASK_TASK_H

#include "app.h"
#include "watchdog_tasks_defs.h"

#define DEFAULT_TASK_DELAY_MS 1000
#define DEFAULT_TASK_STACK_SIZE_WORDS 256
void default_task_task(void *argument);


task_entry_t create_default_task_task(void);

#endif /* DEFAULT_TASK_TASK_H */

