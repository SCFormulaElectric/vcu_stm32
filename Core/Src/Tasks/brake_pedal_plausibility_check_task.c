#include "FreeRTOS.h"
#include "task.h"
#include "Tasks/brake_pedal_plausibility_check_task.h"
#include "app.h"

// Task: Brake Pedal Plausibility Check

void brake_pedal_plausibility_check_task(void *argument) {
    volatile app_data *data = (app_data *) argument;
    MotorControl_t *motorControl = &data->motorControl;

    TickType_t start = xTaskGetTickCount();

    for (;;) {
        int16_t throttle = (int16_t)getThrottle();
        // Check to see if it is pressed, currently at 10%
        bool brake_engaged = data->brake_level > 100;
        if (throttle > 250 && brake_engaged && motorControl->opState == enabled) {
            motorControl->plaus_fault = true;
            motorControl->opState = plausibility_error;
        } else if (throttle < 50) {
            if (!motorControl->fault) { // Only reset if no other fault exists
                motorControl->plaus_fault = false;
                motorControl->opState = enabled;
            }
        }     
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

TaskHandle_t create_brake_pedal_plausibility_check_task(void) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        brake_pedal_plausibility_check_task,            // Task function
        "Brake Pedal Plausibility Check",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        tskIDLE_PRIORITY + 1,    // Priority (adjust as needed)
        &handle                  // Task handle
    );
    return handle;
}
