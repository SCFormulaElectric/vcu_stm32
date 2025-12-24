#include "Tasks/Misc/cooling_task.h"

// Task: Cooling

void cooling_task(void *argument) {
    for (;;) {
        // TODO: Implement Cooling functionality
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

TaskHandle_t create_cooling_task(void) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        cooling_task,            
        "Cooling",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &handle                  // Task handle
    );
    return handle;
}
