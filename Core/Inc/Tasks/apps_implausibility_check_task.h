#ifndef APPS_IMPLAUSIBILITY_CHECK_TASK_H
#define APPS_IMPLAUSIBILITY_CHECK_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "app.h"

// Task function
void apps_implausibility_check_task(void *argument);

// Function to create the task and return its handle
TaskHandle_t create_apps_implausibility_check_task(void);

#endif /* APPS_IMPLAUSIBILITY_CHECK_TASK_H */
