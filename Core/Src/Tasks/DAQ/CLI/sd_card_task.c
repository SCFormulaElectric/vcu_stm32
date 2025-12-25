#include "Tasks/DAQ/CLI/sd_card_task.h"

// Task: SD Card

void sd_card_task(void *argument) {
    for (;;) {
        TickType_t start = xTaskGetTickCount();
        // TODO: Implement SD Card functionality
        vTaskDelayUntil(&start, pdMS_TO_TICKS(1000));
    }
}

task_entry_t create_sd_card_task(void) {
    task_entry_t entry = {0};
    xTaskCreate(
        sd_card_task,            
        "SD Card",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    
    vTaskSuspend(entry.handle);
    entry.name = "sd";
    return entry;
}
