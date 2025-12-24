#ifndef CLI_INPUT_TASK_H
#define CLI_INPUT_TASK_H

#include "FreeRTOS.h"
#include "task.h"


void cli_input_task(void *argument);


TaskHandle_t create_cli_input_task(void);

#endif /* CLI_INPUT_TASK_H */

