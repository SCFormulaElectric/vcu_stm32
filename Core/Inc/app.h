#ifndef APP_H
#define APP_H

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "main.h"
#include "Tasks/Task_Helper/motor_control.h"
#include "Peripherals/can_bus.h"
#include "Peripherals/usb_conf.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Peripherals/adc.h"
#include "queue.h"
#include "event_groups.h"

#define NUM_TASKS 14

// Define priorities of tasks
#define BPPS_PRIO               16
#define APPS_PRIO               16
#define IDWG_PRIO               16
#define SD_CARD_PRIO            15
#define MCT_PRIO                14
#define CAN_PRIO                12
#define COOLING_PRIO            11
#define LC_PRIO                 10
#define SM_PRIO      			12
#define TELEMETRY_PRIO     		6
#define CLI_PRIO          		4
#define DASH_PRIO               3
#define DEFAULT_TASK_PRIO       1

// Helper defines for app.c
#define CLI_QUEUE_LENGTH        128
#define CLI_ITEM_SIZE           sizeof(char)

#define CAN_QUEUE_LENGTH        10
#define CAN_TX_MESSAGE_SIZE     sizeof(can_tx_message_t)
#define CAN_RX_MESSAGE_SIZE     sizeof(can_rx_message_t)

#define LOG_MSG_MAX_LEN         128
#define LOG_QUEUE_LENGTH        64
#define LOG_MSG_SIZE            sizeof(log_msg_t)

#define VCU_FIRMWARE_VERSION    "inspection-baseline"
#define VCU_TARGET_MCU          "STM32F407VET6"

// Helper function for all tasks!
#define ADC_TO_VOLTS(x) (((float)(x) * 3.3f) / 4095.0f)

// Stack sizes
#define KILOBYTE 256

typedef struct {
    char line[LOG_MSG_MAX_LEN];
} log_msg_t;

typedef enum {
    START_ALL,
    START_CLI_ONLY,
    START_NO_IDWG
} StartUpMode;

typedef enum {
    LOG_SD_CARD,
    LOG_SERIAL,
    LOG_NONE
} LogLevel;

typedef enum {
    CAR_IDLE,
    CAR_PREPARE,
    CAR_ENABLE
} car_state_t;

typedef enum {
    SAFETY_FAULT_NONE       = 0U,
    SAFETY_FAULT_TSMS       = (1U << 0),
    SAFETY_FAULT_BMS        = (1U << 1),
    SAFETY_FAULT_APPS       = (1U << 2),
    SAFETY_FAULT_BPPS       = (1U << 3),
    SAFETY_FAULT_INVERTER   = (1U << 4),
    SAFETY_FAULT_CAN        = (1U << 5),
    SAFETY_FAULT_ADC        = (1U << 6),
    SAFETY_FAULT_RTD        = (1U << 7)
} safety_fault_t;

typedef enum {
    PEDAL_RESPONSE_LINEAR = 0,
    PEDAL_RESPONSE_EARLY,
    PEDAL_RESPONSE_BALANCED,
    PEDAL_RESPONSE_PROGRESSIVE
} pedal_response_mode_t;

typedef struct {
    volatile pedal_response_mode_t mode;
    /* 0 = linear, 1000 = full selected curve. */
    volatile uint16_t strength;
} pedal_response_config_t;

typedef struct {
    const char *name;
    TaskHandle_t handle;
} task_entry_t;

typedef struct app_data_s {
	// Task handles
	task_entry_t task_entries[NUM_TASKS];

    StartUpMode         startup_mode;
    LogLevel            log_level;
	can_bus_t           can_bus;
	MotorControl_t      motorControl;
	QueueHandle_t       cli_queue;

	volatile car_state_t    car_state;
	volatile uint16_t       throttle_level;
	volatile uint16_t       brake_level;
    pedal_response_config_t pedal_response;
    volatile uint32_t      safety_faults;
    volatile uint8_t       ready_to_drive;
    volatile uint8_t       rtd_sound_active;
    EventGroupHandle_t      idwg_group;
    sd_card_t               sd_card;
} app_data_t;

extern app_data_t app;
void create_app();
void __serial_print(const char *str);
void serial_log(const char *fmt, ...);
#endif /* APP_H */
