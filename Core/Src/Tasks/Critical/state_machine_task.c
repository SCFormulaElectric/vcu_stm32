#include "Tasks/Critical/state_machine_task.h"

// Task: State Machine

void state_machine_task(void *argument) {
    app_data_t *data = (app_data_t *)argument;
    TickType_t prepare_started = 0;
    for (;;) {
        TickType_t start = xTaskGetTickCount();
        const GPIO_PinState tsms = HAL_GPIO_ReadPin(GPIOA, TSMS_PIN);
        const GPIO_PinState r2d = HAL_GPIO_ReadPin(GPIOA, R2D_PIN);
        const GPIO_PinState bms = HAL_GPIO_ReadPin(GPIOA, BMS_PIN);
        inverter_fault_status_snapshot_t inverter_status = {0};
        motor_control_read_fault_status(&data->motorControl, &inverter_status);
        const uint8_t inverter_status_fresh =
            inverter_fault_status_snapshot_is_fresh(&inverter_status,
                (uint32_t)start,
                (uint32_t)pdMS_TO_TICKS(CAN_INVERTER_FAULT_STATUS_TIMEOUT_MS));
        adc_snapshot_t adc_snapshot = {0};
        const adc_snapshot_status_t adc_status = adc_acquisition_read(
            &adc_snapshot, HAL_GetTick());
        const uint32_t faults =
            ((VCU_TORQUE_ENABLE_COMMISSIONED == 0U) ?
                SAFETY_FAULT_COMMISSIONING : 0U) |
            ((data->boot_healthy == 0U) ? SAFETY_FAULT_BOOT : 0U) |
            ((tsms != GPIO_PIN_SET) ? SAFETY_FAULT_TSMS : 0U) |
            ((bms != GPIO_PIN_SET) ? SAFETY_FAULT_BMS : 0U) |
            ((data->motorControl.input_faults.apps_fault != 0U) ? SAFETY_FAULT_APPS : 0U) |
             ((data->motorControl.input_faults.bpps_fault != 0U) ? SAFETY_FAULT_BPPS : 0U) |
            ((adc_status != ADC_SNAPSHOT_OK) ? SAFETY_FAULT_ADC : 0U) |
            ((!inverter_status_fresh ||
              data->can_bus.tx_safety.fault_latched != 0U ||
              data->can_bus.rx_fault_latched != 0U) ?
                SAFETY_FAULT_CAN : 0U) |
            (is_fault(&inverter_status.fault_codes) ? SAFETY_FAULT_INVERTER : 0U);
        uint8_t forced_disable = 0U;
        taskENTER_CRITICAL();
        data->safety_faults = faults;
        /* Exit the globally visible enabled state before any lower-priority
         * switch processing. A higher-priority motor task that preempts after
         * this store sees IDLE and can publish only the inhibited zero frame. */
        if (data->car_state == CAR_ENABLE &&
            (faults != SAFETY_FAULT_NONE || r2d != GPIO_PIN_SET)) {
            data->car_state = CAR_IDLE;
            data->ready_to_drive = 0U;
            data->rtd_sound_active = 0U;
            forced_disable = 1U;
        }
        taskEXIT_CRITICAL();
        if (forced_disable != 0U) {
            (void)can_bus_request_motor_inhibit(&data->can_bus);
        }
        car_state_t car_state = data->car_state;
        switch (car_state) {
            case CAR_IDLE:{
                data->ready_to_drive = 0U;
                data->rtd_sound_active = 0U;
                prepare_started = 0;
                if (faults == SAFETY_FAULT_NONE && r2d == GPIO_PIN_SET &&
                    data->brake_level > BRAKE_THRESHOLD) {
                    car_state = CAR_PREPARE;
                    prepare_started = start;
                }
                break;
            }
            case CAR_PREPARE:{
                data->rtd_sound_active = 1U;
                if (faults != SAFETY_FAULT_NONE || r2d != GPIO_PIN_SET ||
                    data->brake_level <= BRAKE_THRESHOLD) {
                    car_state = CAR_IDLE;
                    data->rtd_sound_active = 0U;
                } else if ((start - prepare_started) >= pdMS_TO_TICKS(RTD_SOUND_DURATION_MS)) {
                    /* The dashboard/buzzer driver must consume rtd_sound_active. */
                    car_state = CAR_ENABLE;
                    data->ready_to_drive = 1U;
                    data->rtd_sound_active = 0U;
                }
                break;
            }
            case CAR_ENABLE:{
                if (faults != SAFETY_FAULT_NONE || r2d != GPIO_PIN_SET) {
                    car_state = CAR_IDLE;
                    data->ready_to_drive = 0U;
                }
                break;
            }
            default:
                car_state = CAR_IDLE;
                data->ready_to_drive = 0U;
                data->rtd_sound_active = 0U;
                break;
        }
        /* Publish the state before changing the CAN inhibit. If leaving
         * ENABLE, a preempting motor task sees the non-enabled state and can
         * only request zero. If entering ENABLE, a preempting motor task sees
         * the still-asserted inhibit and can only queue zero. */
        data->car_state = car_state;
        if (car_state == CAR_ENABLE && faults == SAFETY_FAULT_NONE) {
            if (can_bus_release_motor_inhibit(&data->can_bus) != pdPASS) {
                data->car_state = CAR_IDLE;
                data->ready_to_drive = 0U;
                (void)can_bus_request_motor_inhibit(&data->can_bus);
            }
        } else {
            (void)can_bus_request_motor_inhibit(&data->can_bus);
        }
        xEventGroupSetBits(data->idwg_group, WD_STATE_MACHINE);
        vTaskDelayUntil(&start, pdMS_TO_TICKS(STATE_MACHINE_DELAY_MS));
    }
}

task_entry_t create_state_machine_task(app_data_t *data) {
    task_entry_t entry = {0};
    BaseType_t status = xTaskCreate(
        state_machine_task,            
        "State Machine",               // Task name (string)
        STATE_MACHINE_STACK_SIZE,// Stack size (words, adjust as needed)
        data,                    // Task parameters
        SM_PRIO,    // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    
    configASSERT(status == pdPASS);
    vTaskSuspend(entry.handle);
    entry.name = "state_machine";
    return entry;
}
