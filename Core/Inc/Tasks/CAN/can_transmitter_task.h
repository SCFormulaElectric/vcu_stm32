#ifndef can_transmitter_TASK_H
#define can_transmitter_TASK_H

#include "app.h"
#include "Tasks/Task_Helper/watchdog_tasks_defs.h"
#include "Peripherals/can_bus.h"

#define CAN_TX_STACK_SIZE KILOBYTE
#define CAN_TX_MAILBOX_WAIT_MS  5
#define CAN_TX_TASK_DELAY_MS 15
void can_transmitter_task(void *argument);


task_entry_t create_can_transmitter_task(void);

#endif /* can_transmitter_TASK_H */

