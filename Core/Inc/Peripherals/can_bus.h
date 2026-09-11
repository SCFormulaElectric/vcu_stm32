#ifndef CAN_BUS_H
#define CAN_BUS_H
#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "Peripherals/can_protocol.h"
#include "Peripherals/can_tx_safety.h"

#define CAN_BUS_QUEUE_LENGTH 8
typedef struct {
    CAN_HandleTypeDef *hcan;
	QueueHandle_t can_tx_queue;
	QueueHandle_t can_rx_queue;
    QueueHandle_t can_rx_fault_queue;
    QueueHandle_t bms_rx_queue;
    QueueHandle_t motor_command_queue;
    TaskHandle_t tx_task_handle;
    can_tx_safety_state_t tx_safety;
    volatile uint32_t rx_dropped;
    volatile uint32_t rx_rejected;
    volatile uint32_t rx_malformed;
    volatile uint32_t rx_critical_updates;
    volatile uint32_t bms_rx_dropped;
    volatile uint8_t rx_fault_latched;
    TaskHandle_t rx_task_handle;
    volatile uint32_t tx_errors;
    volatile uint32_t bus_off_count;
} can_bus_t;

typedef struct {
    uint32_t id;
    uint8_t  dlc;
    uint8_t  data[8];
    uint8_t  is_extended;
} can_rx_message_t;

BaseType_t can_bus_publish_motor_command(can_bus_t *can_bus,
    const can_tx_message_t *command, uint8_t inhibit_requested);
BaseType_t can_bus_request_motor_inhibit(can_bus_t *can_bus);
BaseType_t can_bus_release_motor_inhibit(can_bus_t *can_bus);
BaseType_t can_bus_queue_generic_message(can_bus_t *can_bus,
    const can_tx_message_t *message);
void can_bus_force_inhibit_from_isr(can_bus_t *can_bus,
    BaseType_t *higher_priority_task_woken);

#endif
