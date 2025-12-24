#ifndef SD_CARD_TASK_H
#define SD_CARD_TASK_H

#include "FreeRTOS.h"
#include "task.h"


void sd_card_task(void *argument);


task_entry_t create_sd_card_task(void);

#endif /* SD_CARD_TASK_H */

