#include "FreeRTOS.h"
#include "task.h"
#include "Tasks/can_transceiver_task.h"

// Task: CAN Transceiver

void can_transceiver_task(void *argument) {
    for (;;) {
        // TODO: Implement CAN Transceiver functionality
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

TaskHandle_t create_can_transceiver_task(void) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        can_transceiver_task,            // Task function
        "CAN Transceiver",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &handle                  // Task handle
    );
    return handle;
}
