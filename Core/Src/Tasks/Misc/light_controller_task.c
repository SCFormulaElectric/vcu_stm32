#include "Tasks/Misc/light_controller_task.h"

// Task: Light Controller

void light_controller_task(void *argument) {
    for (;;) {
        TickType_t start = xTaskGetTickCount();
        // TODO: Implement Light Controller functionality
        vTaskDelayUntil(&start, pdMS_TO_TICKS(1000));
    }
}

task_entry_t create_light_controller_task(void) {
    task_entry_t entry = {0};
    xTaskCreate(
        light_controller_task,            
        "Light Controller",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    vTaskSuspend(entry.handle);
    entry.name = "light_controller";
    return entry;
}
