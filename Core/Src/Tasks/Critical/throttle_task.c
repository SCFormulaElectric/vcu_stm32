#include "Tasks/Critical/throttle_task.h"

void throttle_task(void *argument) {
    app_data_t *data = (app_data_t *) argument;

    for (;;) {
        TickType_t start = xTaskGetTickCount();
        uint16_t input_1 = adc_buffer[THROTTLE_PIN1];
        uint16_t input_2 = adc_buffer[THROTTLE_PIN2];
        uint16_t throttle_1 = map_to_percentage(input_1, THROTTLE_PIN1_MIN, THROTTLE_PIN1_MAX); 
        uint16_t throttle_2 = map_to_percentage(input_2, THROTTLE_PIN2_MIN, THROTTLE_PIN2_MAX);

        //If are signals are valid
        if (0 == invalid_signal_check(throttle_1, throttle_2)) {
            data->motorControl.input_faults.apps_fault = 0;
            data->throttle_level = (throttle_1 + throttle_2) / 2;
        }
        else {
            data->motorControl.input_faults.apps_fault = 1;
            data->throttle_level = 0;
        }
        xEventGroupSetBits(data->idwg_group, WD_THROTTLE);
        vTaskDelayUntil(&start, pdMS_TO_TICKS(THROTTLE_TASK_DELAY_MS));
    }
}

task_entry_t create_throttle_task(app_data_t *data) {
    task_entry_t entry = {0};
    BaseType_t status = xTaskCreate(
        throttle_task,            
        "APPS Implausibility Check",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        APPS_PRIO,               // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    
    configASSERT(status == pdPASS);
    vTaskSuspend(entry.handle);
    entry.name = "throttle";
    return entry;
}

/**
 * @param 
 *  input_1: raw ADC reading of sensor 1 from pedal
 *  input_2: raw ADC reading of sensor 2 from pedal
 */
uint8_t invalid_signal_check(uint16_t input_1, uint16_t input_2) {
    
    //Failed to be within the min and max range of the pedal
    uint8_t bound_invalid = out_of_bounds(input_1, input_2);
    if (1 == bound_invalid) {
        return 1;
    }
    
    //Failed to be within the allowed percentage of the two pedals
    uint8_t apps_invalid = apps_faulted(input_1, input_2);
    if (1 == apps_invalid) {
        return 1;
    }

    return 0;
}

uint8_t out_of_bounds(uint16_t level1, uint16_t level2) {
    if (level1 < 0 - THROTTLE_FAULT_TOLERANCE) {
        serial_print("Throttle 1 too small: %d", level1);
        return 1;
    }
    if (level1 > 1000 + THROTTLE_FAULT_TOLERANCE) {
        serial_print("Throttle 1 too large: %d", level1);
        return 1;
    }

    if (level2 < 0 - THROTTLE_FAULT_TOLERANCE) {
        serial_print("Throttle 2 too small: %d", level2);
        return 1;
    }

    if (level2 > 1000 + THROTTLE_FAULT_TOLERANCE) {
        serial_print("Throttle 2 too large: %d", level2);
        return 1;
    }
    
    // Now that we know that it is within fault tolerance, we can constrain it between 0 - 1000
    if (level1 > 1000) {
        level1 = 1000;
    }
    if (level2 > 1000) {
        level2 = 1000;
    }

    return 0;
}


uint8_t apps_faulted(uint16_t level1, uint16_t level2) {
    if (level1 > level2 + MAX_PEDAL_DIFFERENCE) {
        serial_print("Throttle 1 too large for Throttle 2. %d >> %d", level1, level2);
        return 1;
    }
    
    if (level2 > level1 + MAX_PEDAL_DIFFERENCE) {
        serial_print("Throttle 1 too small for Throttle 2. %d << %d", level1, level2);
        return 1;
    }
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