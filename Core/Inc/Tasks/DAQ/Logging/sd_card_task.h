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
#define SD_CARD_MESSAGES_PER_ACTIVATION 8U
#define SD_CARD_MAX_LOG_INDEX       9999U
#define SD_CARD_NAME_CHECKS_PER_ACTIVATION 8U
#define BITS_CLEARED_BEFORE_READ    0x0
#define BITS_CLEARED_AFTER_READ     0xFFFFFFFF
void sd_card_task(void *argument);


FRESULT find_next_log_index(uint32_t *cursor, uint32_t *index_out,
    uint8_t *found);
task_entry_t create_sd_card_task(app_data_t *data);

#endif /* SD_CARD_TASK_H */

