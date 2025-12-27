#include "Tasks/Misc/light_controller_task.h"

// Task: Light Controller

void light_controller_task(void *argument) {
    app_data_t *data = (app_data_t *) argument;

    for (;;) {
        TickType_t start = xTaskGetTickCount();
        // TODO: Implement Light Controller functionality
        xEventGroupSetBits(data->idwg_group, WD_LIGHT_CONTROLLER);
        vTaskDelayUntil(&start, pdMS_TO_TICKS(1000));
    }
}

task_entry_t create_light_controller_task(app_data_t *data) {
    task_entry_t entry = {0};
    BaseType_t status = xTaskCreate(
        light_controller_task,            
        "Light Controller",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        LC_PRIO,    // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    configASSERT(status == pdPASS);
    vTaskSuspend(entry.handle);
    entry.name = "light_controller";
    return entry;
}
