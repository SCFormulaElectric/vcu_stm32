#include "Tasks/Critical/brake_pedal_plausibility_check_task.h"

void brake_pedal_plausibility_check_task(void *argument) {
    app_data_t *data = (app_data_t *) argument;
    TickType_t start = xTaskGetTickCount();

    for (;;) {
        if (data->throttle_level > BPPS_THROTTLE_ENABLED && data->brake_level > BPPS_BRAKE_TRESH) {
            data->motorControl.input_faults.bpps_fault = 1;
        } else if (data->throttle_level < BPPS_THROTTLE_DISABLED) {
            data->motorControl.input_faults.bpps_fault = 0;
        }     
        vTaskDelay(pdMS_TO_TICKS(BPPS_DELAY_MS));
    }
}

task_entry_t create_brake_pedal_plausibility_check_task(void) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        brake_pedal_plausibility_check_task,            
        "Brake Pedal Plausibility Check",               // Task name (string)
        BPPS_STACK_SIZE_WORDS,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &handle                  // Task handle
    );
    task_entry_t entry;
    entry.handle = handle;
    entry.name = "bpps"
    return entry;
}
