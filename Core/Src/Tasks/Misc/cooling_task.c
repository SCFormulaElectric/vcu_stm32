#include "Tasks/Misc/cooling_task.h"

// Task: Cooling

void cooling_task(void *argument) {
    for (;;) {
        TickType_t start = xTaskGetTickCount();
        // TODO: Implement Cooling functionality
        vTaskDelayUntil(&start, pdMS_TO_TICKS(1000));
    }
}

task_entry_t create_cooling_task(void) {
    task_entry_t entry = {0};
    xTaskCreate(
        cooling_task,            
        "Cooling",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    vTaskSuspend(entry.handle);
    entry.name = "cooling";
    return entry;
}
