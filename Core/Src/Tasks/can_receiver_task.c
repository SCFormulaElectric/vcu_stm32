#include "FreeRTOS.h"
#include "task.h"
#include "Tasks/can_receiver_task.h"

// Task: CAN Receiver

void can_receiver_task(void *argument) {
    for (;;) {
        // TODO: Implement CAN Receiver functionality
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

TaskHandle_t create_can_receiver_task(void) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        can_receiver_task,            // Task function
        "CAN Receiver",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &handle                  // Task handle
    );
    return handle;
}
