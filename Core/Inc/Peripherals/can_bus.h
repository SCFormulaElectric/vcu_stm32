#ifndef CAN_BUS_H
#define CAN_BUS_H
#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "queue.h"

#define CAN_BUS_QUEUE_LENGTH 8
//Todo: check if these fields should be volatile or not
typedef struct {
    CAN_HandleTypeDef *hcan;
	QueueHandle_t can_tx_queue;
	QueueHandle_t can_rx_queue;
} can_bus_t;

typedef struct {
    uint32_t tx_id;
    uint8_t dlc;
    uint8_t  tx_packet[8];
} can_tx_message_t;

typedef struct {
    uint32_t id;
    uint8_t  dlc;
    uint8_t  data[8];
    uint8_t  is_extended;
} can_rx_message_t;

extern app_data_t app; 
#endif