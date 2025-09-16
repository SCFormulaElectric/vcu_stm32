#include "FreeRTOS.h"
#include "task.h"
#include "Tasks/motor_controller_task.h"

// Task: Motor Controller

void motor_controller_task(void *argument) {
    for (;;) {
        // TODO: Implement Motor Controller functionality
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

TaskHandle_t create_motor_controller_task(void) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        motor_controller_task,            // Task function
        "Motor Controller",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &handle                  // Task handle
    );
    return handle;
}
