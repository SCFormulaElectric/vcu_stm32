#ifndef BRAKE_PEDAL_PLAUSIBILITY_CHECK_TASK_H
#define BRAKE_PEDAL_PLAUSIBILITY_CHECK_TASK_H

#include "app.h"
#define BPPS_THROTTLE_ENABLED 250
#define BPPS_THROTTLE_DISABLED 50
#define BPPS_BRAKE_TRESH 100
#define BPPS_DELAY_MS 50
#define BPPS_STACK_SIZE_WORDS 256

void brake_pedal_plausibility_check_task(void *argument);
task_entry_t create_brake_pedal_plausibility_check_task(void);

#endif /* BRAKE_PEDAL_PLAUSIBILITY_CHECK_TASK_H */

