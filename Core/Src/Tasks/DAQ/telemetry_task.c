#include "Tasks/DAQ/telemetry_task.h"

// Task: Telemetry

void telemetry_task(void *argument) {
    for (;;) {
        TickType_t start = xTaskGetTickCount();
        // TODO: Implement Telemetry functionality
        vTaskDelayUntil(&start, pdMS_TO_TICKS(1000));
    }
}

task_entry_t create_telemetry_task(void) {
    task_entry_t entry = {0};
    xTaskCreate(
        telemetry_task,            
        "Telemetry",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    
    vTaskSuspend(entry.handle);
    entry.name = "telemetry";
    return entry;
}
