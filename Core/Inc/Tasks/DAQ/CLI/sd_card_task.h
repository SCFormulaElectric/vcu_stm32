#ifndef SD_CARD_TASK_H
#define SD_CARD_TASK_H

#include "FreeRTOS.h"
#include "task.h"

// Task function
void sd_card_task(void *argument);

// Function to create the task and return its handle
TaskHandle_t create_sd_card_task(void);

#endif /* SD_CARD_TASK_H */

