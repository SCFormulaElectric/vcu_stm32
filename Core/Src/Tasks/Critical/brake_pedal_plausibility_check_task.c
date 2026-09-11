#include "Tasks/Critical/brake_pedal_plausibility_check_task.h"

void brake_pedal_plausibility_check_task(void *argument) {
    app_data_t *data = (app_data_t *) argument;
    pedal_persistence_timer_t disagreement_timer = {0};
    uint8_t brake_sensor_fault = 0U;
    uint8_t overlap_latched = 0U;
    
    for (;;) {
        TickType_t start = xTaskGetTickCount();
        adc_snapshot_t adc_snapshot = {0};
        const adc_snapshot_status_t adc_status = adc_acquisition_read(
            &adc_snapshot, HAL_GetTick());
        uint16_t input_1 = adc_snapshot.channels[BRAKE_PIN1];
        uint16_t input_2 = adc_snapshot.channels[BRAKE_PIN2];
        uint16_t brake_1 = pedal_normalize(input_1, BRAKE_PIN1_MIN, BRAKE_PIN1_MAX);
        uint16_t brake_2 = pedal_normalize(input_2, BRAKE_PIN2_MIN, BRAKE_PIN2_MAX);
        
        uint16_t throttle_level = data->throttle_level;
        uint16_t brake_level = (brake_1 + brake_2) / 2U;

        const uint8_t brake_inputs_valid =
            (adc_status == ADC_SNAPSHOT_OK &&
             input_1 >= BRAKE_PIN1_MIN && input_1 <= BRAKE_PIN1_MAX &&
             input_2 >= BRAKE_PIN2_MIN && input_2 <= BRAKE_PIN2_MAX);

        if (brake_inputs_valid == 0U) {
            brake_sensor_fault = brake_sensor_fault_update(
                brake_sensor_fault, 0U, 0U, 0U);
            (void)pedal_condition_persisted(&disagreement_timer, 0U, 0U, 0U);
            data->brake_level = 0U;
        } else {
            const uint8_t sensors_disagree = pedal_values_disagree(
                brake_1, brake_2, BSE_NORMALIZED_MAX_DIFFERENCE);
            const uint8_t disagreement_persisted = pedal_condition_persisted(
                &disagreement_timer, sensors_disagree, (uint32_t)start,
                (uint32_t)pdMS_TO_TICKS(BSE_DISAGREEMENT_PERSISTENCE_MS));
            brake_sensor_fault = brake_sensor_fault_update(
                brake_sensor_fault, 1U, sensors_disagree,
                disagreement_persisted);
            data->brake_level = brake_level;
        }

        /* EV.4.7 overlap is an immediate latch; it is not debounced. Once set,
         * only APPS below 5% releases it, regardless of later brake state. */
        overlap_latched = brake_throttle_overlap_latch_update(
            overlap_latched, throttle_level,
            (brake_inputs_valid != 0U) ? brake_level : 0U,
            BPPS_THROTTLE_ENABLED, BPPS_THROTTLE_DISABLED,
            BPPS_BRAKE_THRESHOLD);

        data->motorControl.input_faults.bpps_fault =
            (brake_sensor_fault != 0U || overlap_latched != 0U) ? 1U : 0U;
        if (data->motorControl.input_faults.bpps_fault != 0U) {
            (void)can_bus_request_motor_inhibit(&data->can_bus);
        }
        xEventGroupSetBits(data->idwg_group, WD_BPPS);
        vTaskDelayUntil(&start, pdMS_TO_TICKS(BPPS_DELAY_MS));
    }
}

task_entry_t create_brake_pedal_plausibility_check_task(app_data_t *data) {
    task_entry_t entry = {0};
    
    BaseType_t status = xTaskCreate(
        brake_pedal_plausibility_check_task,            
        "Brake Pedal Plausibility Check",               // Task name (string)
        BPPS_STACK_SIZE,
        data,                    // Task parameters
        BPPS_PRIO,    // Priority (adjust as needed)
        &entry.handle
    );

    configASSERT(status == pdPASS);
    vTaskSuspend(entry.handle);
    entry.name = "bpps";
    return entry;
}
