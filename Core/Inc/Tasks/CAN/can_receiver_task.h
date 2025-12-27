#ifndef CAN_RECEIVER_TASK_H
#define CAN_RECEIVER_TASK_H

#include "app.h"
#include "Tasks/Task_Helper/watchdog_tasks_defs.h"
#include "Peripherals/can_bus.h"

#define CAN_RX_DELAY_MS 20
#define CAN_RX_STACK_SIZE 2*KILOBYTE

// Motor Controller IDs
#define MOTOR_CONTROLLER_ID_MIN 0x0A0
#define MOTOR_CONTROLLER_ID_MAX 0x0AF
#define IS_MOTOR_CONTROLLER_ID(id) ((id) >= MOTOR_CONTROLLER_ID_MIN && (id) <= MOTOR_CONTROLLER_ID_MAX)

//Dashboard IDS
#define DASHBOARD_ID_MIN
#define DASHBOARD_ID_MAX
#define IS_DASHBOARD_ID(id) ((id) >= DASHBOARD_ID_MIN && (id) <= DASHBOARD_ID_MAX)
/*0x0A0 Slow/10 Hz Temperatures #1 0x0001
0x0A1 Slow/10 Hz Temperatures #2 0x0002
0x0A2 Slow/10 Hz Temperatures #3 0x0004
0x0A3 Fast/100 Hz Analog Inputs Voltages 0x0008
0x0A4 Fast/100 Hz Digital Input Status 0x0010
0x0A5 Fast/100 Hz Motor Position Information 0x0020
0x0A6 Fast/100 Hz Current Information 0x0040
0x0A7 Fast/100 Hz Voltage Information 0x0080
0x0A8 Fast/100 Hz Flux Information 0x0100
0x0A9 Slow/10 Hz Internal Voltages 0x0200
0x0AA Fast/100 Hz Internal States 0x0400
0x0AB Fast/100 Hz Fault Codes 0x0800
0x0AC Fast/100 Hz Torque & Timer Information 0x1000
0x0AD Fast/100 Hz Modulation Index & Flux Weakening Output
Information 0x2000
0x0AE Slow/10 Hz Firmware Information 0x4000
0x0AF 100 Hz (fixed) Diagnostic Data */


void can_receiver_task(void *argument);
void process_MC_msg(app_data_t *data, can_rx_message_t message);
void process_Dashboard_msg(app_data_t *data, can_rx_message_t message);

task_entry_t create_can_receiver_task(void);

#endif /* CAN_RECEIVER_TASK_H */

