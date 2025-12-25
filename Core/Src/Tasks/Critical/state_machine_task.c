#include "Tasks/Critical/state_machine_task.h"

// Task: State Machine

void state_machine_task(void *argument) {
    app_data_t *data = (app_data_t *)argument;
    GPIO_PinState tsms;
    GPIO_PinState r2d;
    uint8_t count = 0;
    for (;;) {
        TickType_t start = xTaskGetTickCount();
        car_state_t car_state = data->car_state;
        tsms = HAL_GPIO_ReadPin(GPIOA, TSMS_PIN);
        r2d = HAL_GPIO_ReadPin(GPIOA, R2D_PIN);
        switch (car_state) {
            case CAR_IDLE:{
                count = 0;
                if(tsms == GPIO_PIN_SET && r2d == GPIO_PIN_SET && data->brake_level > BRAKE_THRESHOLD) {
                    car_state = CAR_PREPARE;
                }
                break;
            }
            case CAR_PREPARE:{
                if (0 == count) {
                    // send CAN message to the dashboard to play the buzzer
                }
                count++;
                count %= BUZZ_TIMEOUT_SECONDS;
                break;
            }
            case CAR_ENABLE:{
                count = 0;
                if (tsms != GPIO_PIN_SET) {
                    car_state = CAR_IDLE;
                }
                break;
            }
        }
        data->car_state = car_state;
        vTaskDelayUntil(&start, pdMS_TO_TICKS(STATE_MACHINE_DELAY_MS));
    }
}

task_entry_t create_state_machine_task(void) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        state_machine_task,            
        "State Machine",               // Task name (string)
        STATE_MACHINE_STACK_SIZE_WORDS,// Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &handle                  // Task handle
    );
    task_entry_t entry;
    entry.handle = handle;
    entry.name = "state_machine";
    return entry;
}
