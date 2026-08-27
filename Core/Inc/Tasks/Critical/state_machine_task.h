#ifndef STATE_MACHINE_TASK_H
#define STATE_MACHINE_TASK_H

#include "app.h"
#include "Tasks/Task_Helper/watchdog_tasks_defs.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "Peripherals/digital_pins.h"

#define STATE_MACHINE_DELAY_MS          50
#define STATE_MACHINE_STACK_SIZE        KILOBYTE
#define RTD_SOUND_DURATION_MS           2000
#define BRAKE_THRESHOLD                 100

void state_machine_task(void *argument);
task_entry_t create_state_machine_task(app_data_t *data);

#endif /* STATE_MACHINE_TASK_H */

