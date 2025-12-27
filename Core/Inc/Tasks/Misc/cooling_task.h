#ifndef COOLING_TASK_H
#define COOLING_TASK_H

#include "app.h"
#include "watchdog_tasks_defs.h"
#include <math.h>

#define THERMISTOR_PIN1 5
#define THERMISTOR_PIN2 6
#define COOLING_DELAY_MS 2000
#define COOLING_STACK_SIZE_WORDS 512
#define VOLTAGE_DIVIDER_RESISTANCE(x) ((14666 * x) / (5 - x))


void cooling_task(void *argument);
float evaluateExpression(float x);
float thermistorToCelsius(const float reading);

task_entry_t create_cooling_task(void);

#endif /* COOLING_TASK_H */

