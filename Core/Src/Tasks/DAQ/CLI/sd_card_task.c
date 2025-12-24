#include "Tasks/DAQ/CLI/sd_card_task.h"

// Task: SD Card

void sd_card_task(void *argument) {
    for (;;) {
        // TODO: Implement SD Card functionality
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

task_entry_t create_sd_card_task(void) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        sd_card_task,            
        "SD Card",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &handle                  // Task handle
    );
    task_entry_t entry;
    entry.handle = handle;
    entry.name = "sd"
    return entry;
}
