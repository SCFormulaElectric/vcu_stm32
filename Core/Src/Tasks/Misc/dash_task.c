#include "FreeRTOS.h"
#include "task.h"
#include "Tasks/Misc/dash_task.h"

// Task: Dash

void dash_task(void *argument) {
    for (;;) {
        // TODO: Implement Dash functionality
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

TaskHandle_t create_dash_task(void) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        dash_task,            // Task function
        "Dash",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &handle                  // Task handle
    );
    return handle;
}
