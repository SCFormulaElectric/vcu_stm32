#ifndef CAN_BUS_H
#define CAN_BUS_H
#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "queue.h"

//Todo: check if these fields should be volatile or not
typedef struct {
    CAN_HandleTypeDef *hcan;
    CAN_TxHeaderTypeDef *tx_header;
    CAN_RxHeaderTypeDef *rx_header;
    volatile uint8_t  rx_packet[8];
    uint8_t  tx_packet[8];
    volatile uint32_t tx_mailbox;
	QueueHandle_t can_tx_queue;
	QueueHandle_t can_rx_queue;
} can_bus_t;

typedef struct {
    uint32_t tx_id;
    uint8_t  tx_packet[8];
} can_message_t;

extern CAN_HandleTypeDef hcan1;
extern CAN_TxHeaderTypeDef tx_header;
extern CAN_RxHeaderTypeDef rx_header;
#endif