#include "Tasks/Misc/default_task_task.h"

// Task: Default Task

void default_task_task(void *argument) {
    app_data_t *data = (app_data_t *) argument;
    
    for (;;) {
        TickType_t start = xTaskGetTickCount();
        // TODO: Implement Default Task functionality

        xEventGroupSetBits(data->idwg_group, WD_DEFAULT);
        vTaskDelayUntil(&start, pdMS_TO_TICKS(DEFAULT_TASK_DELAY_MS));
    }
}

task_entry_t create_default_task_task(app_data_t *data) {
    task_entry_t entry = {0};
    BaseType_t status = xTaskCreate(
        default_task_task,            
        "Default Task",               // Task name (string)
        DEFAULT_TASK_STACK_SIZE,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        default_task_prio,    // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    configASSERT(status == pdPASS);
    vTaskSuspend(entry.handle);
    entry.name = "default";
    return entry;
}
