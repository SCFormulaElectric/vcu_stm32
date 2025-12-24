#include "Tasks/Critical/throttle_task.h"

void throttle_task(void *argument) {
    app_data_t *data = (app_data_t *) argument;

    for (;;) {
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
        
        vTaskDelay(pdMS_TO_TICKS(THROTTLE_TASK_DELAY_MS));
    }
}

TaskHandle_t create_throttle_task(app_data_t *data) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        throttle_task,            
        "APPS Implausibility Check",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        APPS_PRIO,               // Priority (adjust as needed)
        &handle                  // Task handle
    );
    return handle;
}

/**
 * @param 
 *  input_1: raw ADC reading of sensor 1 from pedal
 *  input_2: raw ADC reading of sensor 2 from pedal
 */
uint_8 invalid_signal_check(uint16_t input_1, uint16_t input_2) {
    
    //Failed to be within the min and max range of the pedal
    uint_8 bound_invalid = out_of_bounds(input_1, input_2);
    if (1 == bound_invalid) {
        return 1;
    }
    
    //Failed to be within the allowed percentage of the two pedals
    uint_8 apps_invalid = apps_faulted(input_1, input_2);
    if (1 == apps_invalid) {
        return 1;
    }

    return 0;
}

// TODO: Add some sort of logging here to see what is actually throwing errors
uint_8 out_of_bounds(uint16_t level1, uint16_t level2) {
    if (level1 < 0 - THROTTLE_FAULT_TOLERANCE) {
        return 1;
    }
    // level 1 to high 
    if (level1 > 1000 + THROTTLE_FAULT_TOLERANCE) {
        return 1;
    }

    // level 2 to low
    if (level2 < 0 - THROTTLE_FAULT_TOLERANCE) {
        return 1;
    }

    // level 2 to high 
    if (level2 > 1000 + THROTTLE_FAULT_TOLERANCE) {
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


// TODO: Add some sort of logging here to see what is actually throwing errors
uint_8 apps_faulted(uint16_t level1, uint16_t level2) {
    // level1 is > MAX_ERROR% more than level2
    if (level1 > level2 + MAX_PEDAL_DIFFERENCE) {
        return 1;
    }
    
    // level2 is > MAX_ERROR% more than level1
    if (level2 > level1 + MAX_PEDAL_DIFFERENCE) {
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