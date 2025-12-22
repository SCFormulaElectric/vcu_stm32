#include "FreeRTOS.h"
#include "task.h"
#include "Tasks/Critical/state_machine_task.h"

// Task: State Machine

void state_machine_task(void *argument) {
    for (;;) {
        // TODO: Implement State Machine functionality
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

TaskHandle_t create_state_machine_task(void) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        state_machine_task,            // Task function
        "State Machine",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &handle                  // Task handle
    );
    return handle;
}
