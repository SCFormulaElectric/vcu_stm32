#ifndef can_transmitter_TASK_H
#define can_transmitter_TASK_H

#include "app.h"
#include "Tasks/Task_Helper/watchdog_tasks_defs.h"
#include "Peripherals/can_bus.h"

#define CAN_TX_STACK_SIZE KILOBYTE
#define CAN_TX_MAILBOX_WAIT_MS       2U
#define CAN_TX_FALLBACK_WAKE_MS      2U
#define CAN_TX_MAX_FRAMES_PER_WAKE   4U
/* A direct inhibit notifies CAN TX immediately. If a notification is missed,
 * the software fallback adds at most 2 ms, followed by at most 2 ms waiting
 * for a HAL mailbox. Scheduler jitter, CAN arbitration, physical delivery, and
 * inverter response are outside this software bound and require measurement. */
void can_transmitter_task(void *argument);


task_entry_t create_can_transmitter_task(app_data_t *data);

#endif /* can_transmitter_TASK_H */

