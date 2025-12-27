#ifndef CLI_INPUT_TASK_H
#define CLI_INPUT_TASK_H

#include "app.h"
#include "Tasks/Task_Helper/watchdog_tasks_defs.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#define CLI_STACK_SIZE          2*KILOBYTE
#define CLI_TASK_DELAY_MS       50
#define CLI_TICKS_TO_WAIT       0
#define CLI_BUFFER_SIZE         50

#define NUM_PERIODIC_OUTPUTS    1
#define IO_output_index         0

#define PRINT_IO_PERIOD_MS      2000

typedef struct {
    char flag;
    TickType_t last_tick_time;
} cli_output_entry_t;

cli_output_entry_t cli_output[NUM_PERIODIC_OUTPUTS];

void cli_input_task(void *argument);
void process_cmd(app_data_t *app, const char *cmd);

task_entry_t create_cli_input_task(void);

#endif /* CLI_INPUT_TASK_H */

