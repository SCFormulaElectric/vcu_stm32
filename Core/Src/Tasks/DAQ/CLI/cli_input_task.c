#include "Tasks/DAQ/CLI/cli_input_task.h"

// Task: CLI Input

void cli_input_task(void *argument) {
    app_data_t *data = (app_data_t *) argument;
    char cli_buffer[CLI_BUFFER_SIZE] = {0};
    uint8_t index = 0;
    for (;;) {
        TickType_t start = xTaskGetTickCount();
        uint8_t ch;
        if (xQueueReceive(data->cli_queue, &ch, CLI_TICKS_TO_WAIT))
        {
            if (ch == '\n' || ch == '\r') {
                cli_buffer[index] = '\0';
                process_cmd(data, cli_buffer);
                index = 0;
            } else {
                cli_buffer[index] = ch;
                index++;

                if (index >= CLI_BUFFER_SIZE) {
                    // todo: Log buffer overflow error
                    index = 0;
                }
            }
        }
        vTaskDelayUntil(&start, pdMS_TO_TICKS(CLI_TASK_DELAY_MS));
    }
}

void process_cmd(app_data_t *app, const char *cmd) {
    char task_name[32];
    int value = -1;

    if (sscanf(cmd, "%31[^=]=%d", task_name, &value) != 2) {
        // printf("Invalid command format: %s\n", cmd);
        return;
    }

    // Look up task by name
    for (size_t i = 0; i < NUM_TASKS; i++) {
        if (strcmp(task_name, app->task_entires[i].name) == 0) {
            TaskHandle_t handle = *(app->task_entires[i].handle);
            if (value == 0) {
                vTaskSuspend(handle);
                // printf("%s suspended\n", task_name);
            } else if (value == 1) {
                vTaskResume(handle);
                // printf("%s resumed\n", task_name);
            } else {
                // printf("Invalid value %d for %s, use 0 or 1\n", value, task_name);
            }
            return;
        }
    }
    // printf("Unknown task: %s\n", task_name);
}
task_entry_t create_cli_input_task(void) {
    task_entry_t entry = {0};
    xTaskCreate(
        cli_input_task,            
        "CLI Input",               // Task name (string)
        CLI_STACK_SIZE_WORDS,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    
    vTaskSuspend(entry.handle);
    entry.name = "cli";
    return entry;
}
