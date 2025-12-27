#include "Tasks/Misc/light_controller_task.h"

// Task: Light Controller

void light_controller_task(void *argument) {
    app_data_t *data = (app_data_t *) argument;
    GPIO_PinState tsms, bms;
    for (;;) {
        TickType_t start = xTaskGetTickCount();

        tsms = HAL_GPIO_ReadPin(GPIOA, TSMS_PIN);
        bms = HAL_GPIO_ReadPin(GPIOA, BMS_PIN);
        if (data->brake_level > BRAKE_THRESHOLD) {
            // todo: make the light turn red
        }

        if (tsms != GPIO_PIN_SET) {
            serial_print("TSMS FAULTED!");
            // todo: ERROR OUT make the hoop light turn red
        }

        if (bms != GPIO_PIN_SET) {
            serial_print("BMS FAULTED!");
            // todo: ERROR OUT make the hoop light turn red
        }

        xEventGroupSetBits(data->idwg_group, WD_LIGHT_CONTROLLER);
        vTaskDelayUntil(&start, pdMS_TO_TICKS(LIGHT_CONTROLLER_DELAY_MS));
    }
}

task_entry_t create_light_controller_task(app_data_t *data) {
    task_entry_t entry = {0};
    BaseType_t status = xTaskCreate(
        light_controller_task,            
        "Light Controller",               // Task name (string)
        LIGHT_CONTROLLER_STACK_SIZE,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        LC_PRIO,    // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    configASSERT(status == pdPASS);
    vTaskSuspend(entry.handle);
    entry.name = "light_controller";
    return entry;
}
