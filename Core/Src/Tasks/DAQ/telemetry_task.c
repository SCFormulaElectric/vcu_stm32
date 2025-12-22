#include "FreeRTOS.h"
#include "task.h"
#include "Tasks/DAQ/telemetry_task.h"

// Task: Telemetry

void telemetry_task(void *argument) {
    for (;;) {
        // TODO: Implement Telemetry functionality
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

TaskHandle_t create_telemetry_task(void) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        telemetry_task,            // Task function
        "Telemetry",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &handle                  // Task handle
    );
    return handle;
}
