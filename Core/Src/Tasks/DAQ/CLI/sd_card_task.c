#include "Tasks/DAQ/CLI/sd_card_task.h"

// Task: SD Card

void sd_card_task(void *argument) {
    app_data_t *data = (app_data_t *) argument;
    for (;;) {
        TickType_t start = xTaskGetTickCount();
        // TODO: Implement SD Card functionality
        xEventGroupSetBits(data->idwg_group, WD_SD_CARD);
        vTaskDelayUntil(&start, pdMS_TO_TICKS(1000));
    }
}

task_entry_t create_sd_card_task(app_data_t *data) {
    task_entry_t entry = {0};
    BaseType_t status = xTaskCreate(
        sd_card_task,            
        "SD Card",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        sd_card_PRIO,    // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    
    configASSERT(status == pdPASS);
    vTaskSuspend(entry.handle);
    entry.name = "sd";
    return entry;
}
