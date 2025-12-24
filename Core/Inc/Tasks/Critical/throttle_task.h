#ifndef throttle_task_H
#define throttle_task_H

#include "app.h"
#include <stdint.h>
#include "semphr.h"
#define MAX_PEDAL_DIFFERENCE 100
#define THROTTLE_FAULT_TOLERANCE 150
#define THROTTLE_TASK_DELAY_MS 50

#define THROTTLE_PIN1 1
#define THROTTLE_PIN2 2

#define THROTTLE_PIN1_MIN 600
#define THROTTLE_PIN1_MAX 3719 
#define THROTTLE_PIN2_MIN 330
#define THROTTLE_PIN2_MAX 1892

void throttle_task(void *argument);
task_entry_t create_throttle_task(void);
uint_8 apps_faulted(uint16_t level1, uint16_t level2);
uint_8 invalid_signal_check(uint16_t input_1, uint16_t input_2);
uint_8 out_of_bounds(uint16_t level1, uint16_t level2);

#endif /* throttle_task_H */

