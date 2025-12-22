#include "FreeRTOS.h"
#include "task.h"
#include "Tasks/Misc/light_controller_task.h"

// Task: Light Controller

void light_controller_task(void *argument) {
    for (;;) {
        // TODO: Implement Light Controller functionality
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

TaskHandle_t create_light_controller_task(void) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        light_controller_task,            // Task function
        "Light Controller",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &handle                  // Task handle
    );
    return handle;
}
