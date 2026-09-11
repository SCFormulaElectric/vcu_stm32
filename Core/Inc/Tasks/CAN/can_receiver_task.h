#ifndef CAN_RECEIVER_TASK_H
#define CAN_RECEIVER_TASK_H

#include "app.h"
#include "Tasks/Task_Helper/watchdog_tasks_defs.h"
#include "Peripherals/can_protocol.h"

#define CAN_RX_FALLBACK_WAKE_MS 20U
#define CAN_RX_MAX_GENERIC_PER_WAKE CAN_QUEUE_LENGTH
#define CAN_RX_STACK_SIZE 2*KILOBYTE

// Motor Controller IDs
#define MOTOR_CONTROLLER_ID_MIN CAN_ID_MC_BROADCAST_MIN
#define MOTOR_CONTROLLER_ID_MAX CAN_ID_MC_BROADCAST_MAX
#define IS_MOTOR_CONTROLLER_ID(id) CAN_ID_IS_MOTOR_CONTROLLER(id)

/* Dashboard IDs must not overlap the Cascadia broadcast range. */
#define DASHBOARD_ID_MIN CAN_ID_DASHBOARD_MIN
#define DASHBOARD_ID_MAX CAN_ID_DASHBOARD_MAX
#define IS_DASHBOARD_ID(id) CAN_ID_IS_DASHBOARD(id)


void can_receiver_task(void *argument);
void process_MC_msg(app_data_t *data, can_rx_message_t message);
void process_Dashboard_msg(app_data_t *data, can_rx_message_t message);

task_entry_t create_can_receiver_task(app_data_t *data);

#endif /* CAN_RECEIVER_TASK_H */

