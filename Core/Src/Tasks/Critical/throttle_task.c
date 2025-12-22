#include "FreeRTOS.h"
#include "task.h"
#include "Tasks/Critical/throttle_task.h"
#include "app.h"

void throttle_task(void *argument) {
    volatile app_data_t *data = (app_data_t *) argument;
    MotorControl_t *motorControl = &data->motorControl;

    for (;;) {
        // TODO: add the ADC reading for the throttle pins
        uint16_t throttle_1 = map_to_percentage(input_1); 
        uint16_t throttle_2 = map_to_percentage(input_2);

        //If are signals are valid
        if (validate_signal(throttle_1, throttle_2)) {
            if (motorControl->opState == throttle_error) {
                motorControl->opState = enabled;
            }
            
            // Todo: Maybe add a check to see if the prev readings were the same as the current reading so you don't need to take the mutex
            throttle_level = (throttle_1 + throttle_2) / 2;
        }
        else {
            if (motorControl->opState == enabled) {
                motorControl->opState = throttle_error;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

TaskHandle_t create_throttle_task(app_data_t *data) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        throttle_task,            // Task function
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
bool validate_signal(uint16_t input_1, uint16_t input_2) {

    // TODO: fill in the function parameters for the raw HAL input, min and max ADC reading of the throttle sensor
    // notes: old pedal sensor was 1 volt == 818 because of 12 bit ADC with range of 0 - 5 volts

    
    bool bound_valid = within_bounds(input_1, input_2);
    //Failed to be within the min and max range of the pedal
    if (!bound_valid) {
        return false;
    }
    
    //Failed to be within the allowed percentage of the two pedals
    bool apps_valid = apps_check(input_1, input_2);
    if (!apps_valid) {
        return false;
    }

    return true;
}

// TODO: Add some sort of logging here to see what is actually throwing errors
bool within_bounds(uint16_t level1, uint16_t level2) {
    if (level1 < 0 - THROTTLE_FAULT_TOLERANCE) {

        return false;
    }
    // level 1 to high 
    if (level1 > 1000 + THROTTLE_FAULT_TOLERANCE) {
        return false;
    }

    // level 2 to low
    if (level2 < 0 - THROTTLE_FAULT_TOLERANCE) {
        return false;
    }

    // level 2 to high 
    if (level2 > 1000 + THROTTLE_FAULT_TOLERANCE) {
        return false;
    }
    
    // Now that we know that it is within fault tolerance, we can constrain it between 0 - 1000
    if (level1 > 1000) {
        level1 = 1000;
    }
    if (level2 > 1000) {
        level2 = 1000;
    }

    return true;
}


// TODO: Add some sort of logging here to see what is actually throwing errors
bool apps_check(uint16_t level1, uint16_t level2) {
    // level1 is > MAX_ERROR% more than level2
    if (level1 > level2 + MAX_PEDAL_DIFFERENCE) {
        return false;
    }
    
    // level2 is > MAX_ERROR% more than level1
    if (level2 > level1 + MAX_PEDAL_DIFFERENCE) {
        return false;
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