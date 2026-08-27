#ifndef COOLING_TASK_H
#define COOLING_TASK_H

#include "app.h"
#include "Tasks/Task_Helper/watchdog_tasks_defs.h"
#include <math.h>

#define THERMISTOR_PIN1                 4
#define THERMISTOR_PIN2                 5
#define COOLING_DELAY_MS                2000
#define COOLING_STACK_SIZE              KILOBYTE
#define VOLTAGE_DIVIDER_RESISTANCE(x)   ((14666 * x) / (5 - x))


void cooling_task(void *argument);
float evaluateExpression(float x);
float thermistorToCelsius(const float reading);

task_entry_t create_cooling_task(app_data_t *data);

#endif /* COOLING_TASK_H */

