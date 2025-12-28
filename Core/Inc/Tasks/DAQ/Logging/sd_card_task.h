#ifndef SD_CARD_TASK_H
#define SD_CARD_TASK_H

#include <stdint.h>
#include "app.h"
#include "Tasks/Task_Helper/watchdog_tasks_defs.h"
#include <stdio.h>
#include <string.h>

#define SD_CARD_STACK_SIZE          8*KILOBYTE
#define SD_CARD_DELAY_MS            200
#define SD_CARD_BUFFER_SIZE         1024
#define SD_CARD_SYNC_PERIOD_MS      5000
#define BITS_CLEARED_BEFORE_READ    0x0
#define BITS_CLEARED_AFTER_READ     0xFFFFFFFF
void sd_card_task(void *argument);
uint32_t find_next_log_index(void);


task_entry_t create_sd_card_task(void);

#endif /* SD_CARD_TASK_H */

