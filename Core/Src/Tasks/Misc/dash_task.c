#include "Tasks/Misc/dash_task.h"

// Task: Dash

void dash_task(void *argument) {
    for (;;) {
        TickType_t start = xTaskGetTickCount();
        // TODO: Implement Dash functionality
        vTaskDelayUntil(&start, pdMS_TO_TICKS(1000));
    }
}

task_entry_t create_dash_task(void) {
    task_entry_t entry = {0};
    xTaskCreate(
        dash_task,            
        "Dash",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    vTaskSuspend(entry.handle);
    entry.name = "dashboard";
    return entry;
}
