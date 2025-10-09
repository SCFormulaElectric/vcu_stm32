#include "FreeRTOS.h"
#include "task.h"
#include "Tasks/throttle_task.h"
#include "app.h"

void throttle_task(void *argument) {
    volatile app_data *data = (app_data *) argument;
    MotorControl_t *motorControl = &data->motorControl;
    for (;;) {
        // TODO: Implement Throttle Pedal Plausibility Check functionality
        uint16_t throttle_level = data->throttle_level;
        uint16_t brake_level = data->throttle_level;

        if (brake_level > )

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

TaskHandle_t create_throttle_task(app_data *data) {
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
 *  disagree_tolerance: value for how much the two sensors can disagree, 10% based on 2024-2025 rules
 *  fault_tolerance: value for which a single sensor can go over or under the constraints
 */
bool validate_signal(uint16_t input_1, uint16_t input_2, uint16_t disagree_tolerance, uint16_t fault_tolerance) {

    // TODO: fill in the function parameters for the raw HAL input, min and max ADC reading of the throttle sensor
    // notes: old pedal sensor was 1 volt == 818 because of 12 bit ADC with range of 0 - 5 volts
    uint16_t throttle_1 = map_to_percentage(); 
    uint16_t throttle_2 = map_to_percentage();
    bool within_bound = within_bounds(throttle_1, throttle_2, fault_tolerance);
    bool apps_valid = apps_check()

}

// TODO: Add some sort of logging here to see what is actually throwing errors
bool within_bounds(uint16_t& level1, uint16_t& level2, uint16_t fault_tolerance) {
    // TODO: we will never have it below 0 because in the map to percentage, we return 0 if input < min_value.
    // we have to because otherwise the mapped value will be < 0 and a uint can not store a negative number.
    // level 1 to low
    if (level1 < 0 - fault_tolerance) {
        return false;
    }
    // level 1 to high 
    if (level1 > 1000 + fault_tolerance) {
        return false;
    }

    // level 2 to low
    if (level2 < 0 - fault_tolerance) {
        return false;
    }

    // level 2 to high 
    if (level2 > 1000 + fault_tolerance) {
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

bool apps_check() {
    
}

uint16_t map_to_percentage(uint16_t input, uint16_t min_val, uint16_t max_val) {
    // invalid case
    if (min_val >= max_val) {
        return 0;
    }
    // mapped value from 0 - 1000
    return (uint16_t)(((uint32_t)(input - min_val) * 1000) / (max_val - min_val));
}