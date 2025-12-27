#include "Tasks/Critical/independent_watchdog_task.h"

// Task: Independent Watchdog

void independent_watchdog_task(void *argument) {
    app_data_t *data = (app_data_t *) argument;
    const TickType_t window = pdMS_TO_TICKS(IDWG_WINDOW_MS);

    for (;;)
    {
        xEventGroupSetBits(data->idwg_group, WD_IDWG);
        EventBits_t bits = xEventGroupWaitBits(
            data->idwg_group,
            WD_ALL_TASKS,
            pdTRUE,        // clear bits after success
            pdTRUE,        // wait for ALL bits
            window
        );

        if ((bits & WD_ALL_TASKS) == WD_ALL_TASKS)
        {
            HAL_IWDG_Refresh(&hiwdg);
        }
        else 
        {
            EventBits_t missing = WD_ALL_TASKS & (~bits);
            for (size_t i = 0; i < NUM_TASKS; i++) 
            {
                if (missing & (1 << i)) 
                {
                    serial_print("IDWG failed: task %s did not set its bit\r\n", app.task_entries[i].name);
                }
            }
            for (;;) {
                // Wait for IWDG to reset the MCU
            }
        }
    }
}

task_entry_t create_independent_watchdog_task(app_data_t *data) {
    task_entry_t entry = {0};

    BaseType_t status = xTaskCreate(
        independent_watchdog_task,
        "Independent Watchdog",               // Task name (string)
        IDWG_STACK_SIZE_WORDS,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        IDWG_PRIO,    // Priority (adjust as needed)
        &entry.handle
    );
    configASSERT(status == pdPASS);
    vTaskSuspend(entry.handle);

    entry.name = "idwg";
    return entry;
}
