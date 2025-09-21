#include "FreeRTOS.h"
#include "task.h"
#include "Tasks/can_receiver_task.h"

// Task: CAN Receiver

void can_receiver_task(void *argument) {
    for (;;) {
        if (xQueueReceive(canRxQueue, &msg, portMAX_DELAY))
        {
        // TODO: Implement CAN Receiver functionality

        }
    }
}

TaskHandle_t create_can_receiver_task(app_data *data) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        can_receiver_task,            // Task function
        "CAN Receiver",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        CAN_receiver_task_PRIO + 1,    // Priority (adjust as needed)
        &handle                  // Task handle
    );
    return handle;
}
