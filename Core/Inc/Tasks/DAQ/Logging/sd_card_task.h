#ifndef SD_CARD_TASK_H
#define SD_CARD_TASK_H

#include <stdint.h>
#include "app.h"
#include "Tasks/Task_Helper/watchdog_tasks_defs.h"
#include <stdio.h>
#include <string.h>

#define SD_CARD_STACK_SIZE      8*KILOBYTE

void sd_card_task(void *argument);
uint32_t find_next_log_index(void);


task_entry_t create_sd_card_task(void);

#endif /* SD_CARD_TASK_H */

