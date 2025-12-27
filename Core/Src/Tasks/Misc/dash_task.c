#include "Tasks/Misc/dash_task.h"

// Task: Dash

void dash_task(void *argument) {
    app_data_t *data = (app_data_t *) argument;

    for (;;) {
        TickType_t start = xTaskGetTickCount();
        // TODO: Implement Dash functionality

        xEventGroupSetBits(data->idwg_group, WD_DASH);
        vTaskDelayUntil(&start, pdMS_TO_TICKS(1000));
    }
}

task_entry_t create_dash_task(app_data_t *data) {
    task_entry_t entry = {0};
    BaseType_t status = xTaskCreate(
        dash_task,            
        "Dash",               // Task name (string)
        DASHBOARD_STACK_SIZE,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        dash_PRIO,    // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    configASSERT(status == pdPASS);
    vTaskSuspend(entry.handle);
    entry.name = "dashboard";
    return entry;
}
