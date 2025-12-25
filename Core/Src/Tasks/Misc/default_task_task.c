#include "Tasks/Misc/default_task_task.h"

// Task: Default Task

void default_task_task(void *argument) {
    for (;;) {
        TickType_t start = xTaskGetTickCount();
        // TODO: Implement Default Task functionality
        vTaskDelayUntil(&start, pdMS_TO_TICKS(1000));
    }
}

task_entry_t create_default_task_task(void) {
    task_entry_t entry = {0};
    xTaskCreate(
        default_task_task,            
        "Default Task",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    vTaskSuspend(entry.handle);
    entry.name = "default";
    return entry;
}
