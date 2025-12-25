#include "Tasks/Critical/independent_watchdog_task.h"

// Task: Independent Watchdog

void independent_watchdog_task(void *argument) {
    for (;;) {
        TickType_t start = xTaskGetTickCount();
        // TODO: Implement Independent Watchdog functionality
        vTaskDelayUntil(&start, pdMS_TO_TICKS(1000));
    }
}

task_entry_t create_independent_watchdog_task(void) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        independent_watchdog_task,            
        "Independent Watchdog",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &handle                  // Task handle
    );
    task_entry_t entry;
    entry.handle = handle;
    entry.name = "idwg";
    return entry;
}
