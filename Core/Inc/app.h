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
    S0,
	S1,
	S2
} system_state_t;

typedef struct {
    const char *name;
    TaskHandle_t *handle;
} task_entry_t;

extern volatile system_state_t extern_curr_state;

typedef struct app_data_s {
	// Task handles
	task_entry_t task_entires[NUM_TASKS];

    StartUpMode  startup_mode;

	can_bus_t      can_bus;
	MotorControl_t motorControl;
	QueueHandle_t  cli_queue;

	volatile uint16_t throttle_level;
	volatile uint16_t brake_level;

} app_data_t;


void create_app();
uint16_t getThrottle();

#endif /* APP_H */
