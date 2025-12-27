#include "Tasks/DAQ/telemetry_task.h"

// Task: Telemetry

void telemetry_task(void *argument) {
    app_data_t *data = (app_data_t *) argument;

    for (;;) {
        TickType_t start = xTaskGetTickCount();
        
        // TODO: Implement Telemetry functionality
        xEventGroupSetBits(data->idwg_group, WD_TELEMETRY);
        vTaskDelayUntil(&start, pdMS_TO_TICKS(1000));
    }
}

task_entry_t create_telemetry_task(app_data_t *data) {
    task_entry_t entry = {0};
    BaseType_t status = xTaskCreate(
        telemetry_task,            
        "Telemetry",               // Task name (string)
        TELEMETRY_STACK_SIZE,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        telemetry_task_PRIO,    // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    
    configASSERT(status == pdPASS);
    vTaskSuspend(entry.handle);
    entry.name = "telemetry";
    return entry;
}
