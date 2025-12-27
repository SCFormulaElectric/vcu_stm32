#include "Tasks/DAQ/Logging/cli_input_task.h"

// Task: CLI Input
static cli_output_entry_t cli_output[NUM_PERIODIC_OUTPUTS] = {0};

void cli_input_task(void *argument) {
    app_data_t *data = (app_data_t *) argument;
    char cli_buffer[CLI_BUFFER_SIZE] = {0};
    uint8_t index = 0;

    populate_array();

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

        print_toggled_outputs();
        xEventGroupSetBits(data->idwg_group, WD_CLI_INPUT);
        vTaskDelayUntil(&start, pdMS_TO_TICKS(CLI_TASK_DELAY_MS));
    }
}

void populate_array() {
    cli_output[IO_output_index].flag = 0;
    cli_output[IO_output_index].last_tick_time = 0;
}

void print_toggled_outputs() {
    TickType_t now = xTaskGetTickCount();
    if (cli_output[IO_output_index].flag && (now - cli_output[IO_output_index].last_tick_time) >= pdMS_TO_TICKS(PRINT_IO_PERIOD_MS)) {
        cli_output[IO_output_index].last_tick_time = now;

        GPIO_PinState dpin1 = HAL_GPIO_ReadPin(GPIOA, PIN1);
        GPIO_PinState dpin2 = HAL_GPIO_ReadPin(GPIOA, PIN2);
        uint16_t a_value1 = adc_buffer[0];
        uint16_t a_value2 = adc_buffer[1];
        uint16_t a_value3 = adc_buffer[2];
        uint16_t a_value4 = adc_buffer[3];
        uint16_t a_value5 = adc_buffer[4];
        uint16_t a_value6 = adc_buffer[5];
        serial_print("ANO1: %u, ANO2: %u, ANO3: %u, ANO4: %u, ANO5: %u, ANO6: %u\r\n",
                    a_value1, a_value2, a_value3, a_value4, a_value5, a_value6);
        serial_print("DIN1: %d, DIN2: %d\r\n", dpin1, dpin2);
    }
}

void process_cmd(app_data_t *app, const char *cmd) {
    char task_name[32];
    int value = -1;

    if (strcmp(cmd, "h") == 0 || strcmp(cmd, "H") == 0) {
        send_task_help(app);
        return;
    }

    if (strcmp(cmd, "t") == 0 || strcmp(cmd, "T") == 0) {
        cli_output[IO_output_index].flag ^= 1;
        return;
    }

    if (sscanf(cmd, "%31[^=]=%d", task_name, &value) == 2) {
        start_stop_task(app, task_name, value);
        return;
    }
    
    serial_print("Invalid command format: %s\n", cmd);
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
    for (size_t i = 0; i < NUM_TASKS; i++) {
        eTaskState state = eTaskGetState(app->task_entries[i].handle);
        const char *state_str;
        switch (state) {
            case eRunning:   state_str = "Running"; break;
            case eReady:     state_str = "Ready"; break;
            case eBlocked:   state_str = "Blocked"; break;
            case eSuspended: state_str = "Suspended"; break;
            case eDeleted:   state_str = "Deleted"; break;
            default:         state_str = "Unknown"; break;
        }
        serial_print("%s: %s\r\n", app->task_entries[i].name, state_str);
    }

    serial_print("\r\nUse: <task_name>=1 to resume, <task_name>=0 to stop\r\n");
    serial_print("Enter: h for help\r\n");
    serial_print("Enter: t to toggle printing IO pins\r\n");
}


task_entry_t create_cli_input_task(app_data_t *data) {
    task_entry_t entry = {0};
    BaseType_t status = xTaskCreate(
        cli_input_task,            
        "CLI Input",               // Task name (string)
        CLI_STACK_SIZE,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        cli_input_PRIO,    // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    
    configASSERT(status == pdPASS);
    vTaskSuspend(entry.handle);
    entry.name = "cli";
    return entry;
}
