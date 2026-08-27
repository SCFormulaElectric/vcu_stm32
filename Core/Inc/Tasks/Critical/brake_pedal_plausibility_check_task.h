#ifndef BRAKE_PEDAL_PLAUSIBILITY_CHECK_TASK_H
#define BRAKE_PEDAL_PLAUSIBILITY_CHECK_TASK_H

#include "app.h"
#include "Tasks/Task_Helper/watchdog_tasks_defs.h"
#define BPPS_DELAY_MS           50

#define BPPS_THROTTLE_ENABLED   250
#define BPPS_THROTTLE_DISABLED  50
#define BPPS_BRAKE_TRESH        100
#define BPPS_STACK_SIZE         KILOBYTE

#define BRAKE_PIN1          2
#define BRAKE_PIN2          3
#define BRAKE_PIN1_MIN      400
#define BRAKE_PIN1_MAX      3681
#define BRAKE_PIN2_MIN      400
#define BRAKE_PIN2_MAX      3681
void brake_pedal_plausibility_check_task(void *argument);
task_entry_t create_brake_pedal_plausibility_check_task(app_data_t *data);

#endif /* BRAKE_PEDAL_PLAUSIBILITY_CHECK_TASK_H */

