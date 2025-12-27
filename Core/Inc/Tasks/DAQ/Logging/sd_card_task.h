#ifndef SD_CARD_TASK_H
#define SD_CARD_TASK_H

#include "app.h"
#include "watchdog_tasks_defs.h"

#define SD_CARD_STACK_SIZE      8*KILOBYTE

void sd_card_task(void *argument);


task_entry_t create_sd_card_task(void);

#endif /* SD_CARD_TASK_H */

