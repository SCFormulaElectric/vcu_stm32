#ifndef APP_H
#define APP_H

#include "motor_control.h"
#include "can_bus.h"
#include "FreeRTOS.h"
#include "task.h"
#include "adc.h"
#include "queue.h"

#define BPPS_PRIO 13
#define APPS_PRIO 13
#define IWT_PRIO  13
#define MCT_PRIO  10
#define CAN_PRIO 10
#define Cooling_PRIO 10
#define LC_PRIO 10
#define telemetry_task_PRIO 6
#define state_machine_PRIO 6
#define dash_PRIO 6
#define sd_card_PRIO 6
#define cli_input_PRIO 6

#define motor_control_interval 25
#define MAX_TORQUE 300 

#define NUM_TASKS 14

#define CLI_QUEUE_LENGTH    10
#define CLI_ITEM_SIZE       sizeof(char)

#define CAN_QUEUE_LENGTH    10
#define CAN_MESSAGE_SIZE    sizeof(can_message_t)

typedef enum {
    ALL,
    CLI_ONLY
} StartUpMode;

typedef enum {
    CAR_IDLE,
    CAR_PREPARE,
    CAR_ENABLE,
} car_state_t;

typedef struct {
    const char *name;
    TaskHandle_t handle;
} task_entry_t;

typedef struct app_data_s {
	// Task handles
	task_entry_t task_entires[NUM_TASKS];

    StartUpMode  startup_mode;
	can_bus_t      can_bus;
	MotorControl_t motorControl;
	QueueHandle_t  cli_queue;
	
	volatile car_state_t car_state;
	volatile uint16_t throttle_level;
	volatile uint16_t brake_level;

} app_data_t;


void create_app();

#endif /* APP_H */
