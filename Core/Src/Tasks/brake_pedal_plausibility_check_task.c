#include "FreeRTOS.h"
#include "task.h"
#include "Tasks/brake_pedal_plausibility_check_task.h"

// Task: Brake Pedal Plausibility Check

void brake_pedal_plausibility_check_task(void *argument) {
    for (;;) {
        // TODO: Implement Brake Pedal Plausibility Check functionality
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

TaskHandle_t create_brake_pedal_plausibility_check_task(void) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        brake_pedal_plausibility_check_task,            // Task function
        "Brake Pedal Plausibility Check",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &handle                  // Task handle
    );
    return handle;
}
