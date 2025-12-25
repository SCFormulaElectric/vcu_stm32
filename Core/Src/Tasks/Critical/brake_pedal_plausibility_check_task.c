#include "Tasks/Critical/brake_pedal_plausibility_check_task.h"

void brake_pedal_plausibility_check_task(void *argument) {
    app_data_t *data = (app_data_t *) argument;
    
    for (;;) {
        TickType_t start = xTaskGetTickCount();
        uint16_t input_1 = adc_buffer[BRAKE_PIN1];
        uint16_t input_2 = adc_buffer[BRAKE_PIN2];
        uint16_t brake_1 = map_to_percentage(input_1, BRAKE_PIN1_MIN, BRAKE_PIN1_MAX); 
        uint16_t brake_2 = map_to_percentage(input_2, BRAKE_PIN2_MIN, BRAKE_PIN2_MAX);
        
        uint16_t throttle_level = data->throttle_level;
        uint16_t brake_level = (brake_1 + brake_2) / 2; 
        data->brake_level = brake_level;

        if (throttle_level > BPPS_THROTTLE_ENABLED && brake_level > BPPS_BRAKE_TRESH) {
            data->motorControl.input_faults.bpps_fault = 1;
        } else if (throttle_level < BPPS_THROTTLE_DISABLED) {
            data->motorControl.input_faults.bpps_fault = 0;
        }     
        vTaskDelayUntil(&start, pdMS_TO_TICKS(BPPS_DELAY_MS));
    }
}

task_entry_t create_brake_pedal_plausibility_check_task(app_data_t *data) {
    task_entry_t entry = {0};
    
    xTaskCreate(
        brake_pedal_plausibility_check_task,            
        "Brake Pedal Plausibility Check",               // Task name (string)
        BPPS_STACK_SIZE_WORDS,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &entry.handle
    );

    vTaskSuspend(entry.handle);
    entry.name = "bpps";
    return entry;
}

uint16_t map_to_percentage(uint16_t input, uint16_t min_val, uint16_t max_val) {
    // invalid case
    if (min_val >= max_val) {
        return 0;
    }
    if (input < min_val) {
        return 0;
    }
    return (uint16_t)(((uint32_t)(input - min_val) * 1000) / (max_val - min_val));
}