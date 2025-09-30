#include "FreeRTOS.h"
#include "task.h"
#include "Tasks/apps_implausibility_check_task.h"
#include "app.h"

void apps_implausibility_check_task(void *argument) {
    app_data *data = (app_data *) argument;
    MotorControl_t *motorControl = &data->motorControl;

    TickType_t start = xTaskGetTickCount();

    for (;;) {
        int16_t throttle = (int16_t)getThrottle();
        bool brake_engaged = data->brake_engaged;
        if (throttle > 250 && brake_engaged) {
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

TaskHandle_t create_apps_implausibility_check_task(app_data *data) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        apps_implausibility_check_task,            // Task function
        "APPS Implausibility Check",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        APPS_PRIO,               // Priority (adjust as needed)
        &handle                  // Task handle
    );
    return handle;
}
