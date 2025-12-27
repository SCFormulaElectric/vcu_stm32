#ifndef CLI_INPUT_TASK_H
#define CLI_INPUT_TASK_H

#include "app.h"
#include "watchdog_tasks_defs.h"
#include <string.h>
#include <stdio.h>

#define CLI_STACK_SIZE_WORDS 1024
#define CLI_TASK_DELAY_MS 100
#define CLI_TICKS_TO_WAIT 0
#define CLI_BUFFER_SIZE 50

void cli_input_task(void *argument);
void process_cmd(app_data_t *app, const char *cmd);

task_entry_t create_cli_input_task(void);

#endif /* CLI_INPUT_TASK_H */

