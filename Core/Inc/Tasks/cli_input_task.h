#ifndef CLI_INPUT_TASK_H
#define CLI_INPUT_TASK_H

#include "FreeRTOS.h"
#include "task.h"

// Task function
void cli_input_task(void *argument);

// Function to create the task and return its handle
TaskHandle_t create_cli_input_task(void);

#endif /* CLI_INPUT_TASK_H */
