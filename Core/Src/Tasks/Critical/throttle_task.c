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
        if (input_1 >= THROTTLE_PIN1_MIN && input_1 <= THROTTLE_PIN1_MAX &&
            input_2 >= THROTTLE_PIN2_MIN && input_2 <= THROTTLE_PIN2_MAX &&
            0 == invalid_signal_check(throttle_1, throttle_2)) {
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
        THROTTLE_STACK_SIZE,                     // Stack size (words, adjust as needed)
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
    if (level1 > 1000U) {
        serial_log("Throttle 1 too large: %u", level1);
        return 1;
    }

    if (level2 > 1000U) {
        serial_log("Throttle 2 too large: %u", level2);
        return 1;
    }
    
    // Now that we know that it is within fault tolerance, we can constrain it between 0 - 1000
    return 0;
}


uint8_t apps_faulted(uint16_t level1, uint16_t level2) {
    if (level1 > level2 + MAX_PEDAL_DIFFERENCE) {
        serial_log("Throttle 1 too large for Throttle 2. %u >> %u", level1, level2);
        return 1;
    }
    
    if (level2 > level1 + MAX_PEDAL_DIFFERENCE) {
        serial_log("Throttle 1 too small for Throttle 2. %u << %u", level1, level2);
        return 1;
    }
    return 0;
}

uint16_t map_to_percentage(uint16_t input, uint16_t min_val, uint16_t max_val) {
    // invalid case
    if (min_val >= max_val) {
        return 0;
    }
    if (input < min_val) {
        return 0;
    }
    if (input >= max_val) {
        return 1000;
    }
    return (uint16_t)(((uint32_t)(input - min_val) * 1000U) / (max_val - min_val));
}

const char *pedal_response_mode_name(pedal_response_mode_t mode) {
    switch (mode) {
        case PEDAL_RESPONSE_LINEAR:      return "linear";
        case PEDAL_RESPONSE_EARLY:       return "early";
        case PEDAL_RESPONSE_BALANCED:    return "balanced";
        case PEDAL_RESPONSE_PROGRESSIVE: return "progressive";
        default:                         return "invalid";
    }
}

/*
 * The response tables are normalized to 0..1000 at 100-count intervals.
 * EARLY reaches more command at low pedal and tapers toward full pedal.
 * BALANCED gives a moderate S-shaped response. PROGRESSIVE gives finer
 * low-pedal control. The selected table is blended with linear according to
 * strength, so the response can be tuned without floating point math.
 */
uint16_t pedal_response_apply(const pedal_response_config_t *config, uint16_t pedal_level) {
    static const uint16_t early_table[11] =
        {0U, 240U, 430U, 580U, 700U, 790U, 860U, 910U, 950U, 980U, 1000U};
    static const uint16_t balanced_table[11] =
        {0U, 60U, 150U, 260U, 390U, 520U, 650U, 770U, 870U, 950U, 1000U};
    static const uint16_t progressive_table[11] =
        {0U, 30U, 90U, 170U, 280U, 400U, 530U, 660U, 780U, 900U, 1000U};

    if (pedal_level > 1000U) {
        pedal_level = 1000U;
    }
    if (config == NULL || config->mode == PEDAL_RESPONSE_LINEAR) {
        return pedal_level;
    }

    const uint16_t *table = NULL;
    switch (config->mode) {
        case PEDAL_RESPONSE_EARLY:       table = early_table; break;
        case PEDAL_RESPONSE_BALANCED:    table = balanced_table; break;
        case PEDAL_RESPONSE_PROGRESSIVE: table = progressive_table; break;
        default:                         return pedal_level;
    }

    uint32_t strength = config->strength;
    if (strength > 1000U) {
        strength = 1000U;
    }

    const uint16_t segment = pedal_level / 100U;
    const uint16_t remainder = pedal_level % 100U;
    uint16_t selected = 1000U;
    if (segment < 10U) {
        const uint16_t low = table[segment];
        const uint16_t high = table[segment + 1U];
        selected = (uint16_t)(low + (((uint32_t)(high - low) * remainder) / 100U));
    }

    if (selected >= pedal_level) {
        return (uint16_t)(pedal_level +
            (((uint32_t)(selected - pedal_level) * strength) / 1000U));
    }
    return (uint16_t)(pedal_level -
        (((uint32_t)(pedal_level - selected) * strength) / 1000U));
}
