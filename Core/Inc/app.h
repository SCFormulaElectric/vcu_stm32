#ifndef APP_H
#define APP_H

#include <stdint.h>
#include "motor_control.h"
#include "can_bus.h"
#include "FreeRTOS.h"
#include "task.h"
#include "adc.h"
#include "queue.h"
#include "event_groups.h"

#define NUM_TASKS 14

// Define priorities of tasks
#define BPPS_PRIO               15
#define APPS_PRIO               15
#define IDWG_PRIO               14
#define MCT_PRIO                14
#define CAN_PRIO                12
#define Cooling_PRIO            11
#define LC_PRIO                 10
#define state_machine_PRIO      12
#define telemetry_task_PRIO     6
#define sd_card_PRIO            5
#define cli_input_PRIO          4
#define dash_PRIO               3
#define default_task_prio       1

// Helper defines for app.c
#define CLI_QUEUE_LENGTH    10
#define CLI_ITEM_SIZE       sizeof(char)

#define CAN_QUEUE_LENGTH    10
#define CAN_TX_MESSAGE_SIZE    sizeof(can_tx_message_t)
#define CAN_RX_MESSAGE_SIZE    sizeof(can_rx_message_t)

// Helper function for all tasks!
#define ADC_TO_VOLTS(x) ((x) / 818.0f)

typedef enum {
    ALL,
    CLI_ONLY,
    NO_IDWG
} StartUpMode;

typedef enum {
    CAR_IDLE,
    CAR_PREPARE,
    CAR_ENABLE
} car_state_t;

typedef struct {
    const char *name;
    TaskHandle_t handle;
} task_entry_t;

typedef struct app_data_s {
	// Task handles
	task_entry_t task_entries[NUM_TASKS];

    StartUpMode         startup_mode;
	can_bus_t           can_bus;
	MotorControl_t      motorControl;
	QueueHandle_t       cli_queue;

	volatile car_state_t    car_state;
	volatile uint16_t       throttle_level;
	volatile uint16_t       brake_level;
    EventGroupHandle_t idwg_group;
} app_data_t;


void create_app();

#endif /* APP_H */
