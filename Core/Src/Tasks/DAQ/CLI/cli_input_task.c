#include "Tasks/DAQ/CLI/cli_input_task.h"

// Task: CLI Input

void cli_input_task(void *argument) {
    for (;;) {
        // TODO: Implement CLI Input functionality
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

TaskHandle_t create_cli_input_task(void) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        cli_input_task,            
        "CLI Input",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &handle                  // Task handle
    );
    return handle;
}
