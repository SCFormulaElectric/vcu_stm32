#include "FreeRTOS.h"
#include "task.h"
#include "Tasks/independent_watchdog_task.h"

// Task: Independent Watchdog

void independent_watchdog_task(void *argument) {
    for (;;) {
        // TODO: Implement Independent Watchdog functionality
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

TaskHandle_t create_independent_watchdog_task(void) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        independent_watchdog_task,            // Task function
        "Independent Watchdog",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &handle                  // Task handle
    );
    return handle;
}
