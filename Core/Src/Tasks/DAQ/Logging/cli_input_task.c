#include "Tasks/DAQ/Logging/cli_input_task.h"


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
                    serial_print("Command too long! Please refresh the buffer");
                    index = 0;
                }
            }
        }
        xEventGroupSetBits(data->idwg_group, WD_CLI_INPUT);
        vTaskDelayUntil(&start, pdMS_TO_TICKS(CLI_TASK_DELAY_MS));
    }
}

void process_cmd(app_data_t *app, const char *cmd) {
    char task_name[32];
    int value = -1;

    if (sscanf(cmd, "%31[^=]=%d", task_name, &value) != 2) {
        serial_print("Invalid command format: %s\n", cmd);
        return;
    }

    start_stop_task(app, task_name, value);

    serial_print("Unknown task: %s\n", task_name);
}

void start_stop_task(app_data_t *app, const char* task_name, int value) {
    for (size_t i = 0; i < NUM_TASKS; i++) {
        if (strcmp(task_name, app->task_entries[i].name) == 0) {
            TaskHandle_t handle = app->task_entries[i].handle;
            if (value == 0) {
                vTaskSuspend(handle);
                serial_print("%s suspended\n", task_name);
            } else if (value == 1) {
                vTaskResume(handle);
                serial_print("%s resumed\n", task_name);
            } else {
                serial_print("Invalid value %d for %s, use 0 or 1\n", value, task_name);
            }
            return;
        }
    }
}

void send_task_help(app_data_t *app) {
    char buf[128];

    serial_print("Task names:\r\n{\r\n");

    for (size_t i = 0; i < NUM_TASKS; i++) {
        snprintf(buf, sizeof(buf), "  %s\r\n", app->task_entries[i].name);
        serial_print(buf);
    }

    serial_print("}\r\nUse: <task_name>=1 to resume, <task_name>=0 to stop\r\n");
}


task_entry_t create_cli_input_task(app_data_t *data) {
    task_entry_t entry = {0};
    BaseType_t status = xTaskCreate(
        cli_input_task,            
        "CLI Input",               // Task name (string)
        CLI_STACK_SIZE_WORDS,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        cli_input_PRIO,    // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    
    configASSERT(status == pdPASS);
    vTaskSuspend(entry.handle);
    entry.name = "cli";
    return entry;
}
