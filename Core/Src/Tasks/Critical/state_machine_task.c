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
        const uint8_t inverter_status_fresh =
            (data->motorControl.fault_codes_valid != 0U) &&
            ((start - data->motorControl.fault_codes_last_tick) <= pdMS_TO_TICKS(250));
        const uint32_t faults =
            ((tsms != GPIO_PIN_SET) ? SAFETY_FAULT_TSMS : 0U) |
            ((bms != GPIO_PIN_SET) ? SAFETY_FAULT_BMS : 0U) |
            ((data->motorControl.input_faults.apps_fault != 0U) ? SAFETY_FAULT_APPS : 0U) |
            ((data->motorControl.input_faults.bpps_fault != 0U) ? SAFETY_FAULT_BPPS : 0U) |
            ((!inverter_status_fresh) ? SAFETY_FAULT_CAN : 0U) |
            (is_fault(&data->motorControl.fault_codes) ? SAFETY_FAULT_INVERTER : 0U);
        data->safety_faults = faults;

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
        data->car_state = car_state;
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
