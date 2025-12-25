#ifndef BRAKE_PEDAL_PLAUSIBILITY_CHECK_TASK_H
#define BRAKE_PEDAL_PLAUSIBILITY_CHECK_TASK_H

#include "app.h"
#define BPPS_THROTTLE_ENABLED   250
#define BPPS_THROTTLE_DISABLED  50
#define BPPS_BRAKE_TRESH        100
#define BPPS_DELAY_MS           50
#define BPPS_STACK_SIZE_WORDS   256

#define BRAKE_PIN1          3
#define BRAKE_PIN2          4
#define BRAKE_PIN1_MIN      400
#define BRAKE_PIN1_MAX      3681
#define BRAKE_PIN2_MIN      400
#define BRAKE_PIN2_MAX      3681
void brake_pedal_plausibility_check_task(void *argument);
task_entry_t create_brake_pedal_plausibility_check_task(void);
uint16_t map_to_percentage(uint16_t input, uint16_t min_val, uint16_t max_val);

#endif /* BRAKE_PEDAL_PLAUSIBILITY_CHECK_TASK_H */

