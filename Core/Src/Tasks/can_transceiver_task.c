#include "FreeRTOS.h"
#include "task.h"
#include "Tasks/can_transceiver_task.h"
#include "canbus.h"

// Task: CAN Transceiver
#define DATA_LENGTH = 8;

void can_transceiver_task(void *argument) {
    app_data_t *data = (app_data_t *) argument;
    canbus_t *canbus = &data->canbus;
    QueueHandle_t queue = data->can_tx_queue;
    can_message_t can_tx;
    HAL_StatusTypeDef can_status;

    for (;;) {
        if (xQueueReceive(queue, &can_tx, portMAX_DELAY)) {

            canbus->tx_header->StdId = can_tx.tx_id;
            for (int i = 0; i < 8; i++) {
                canbus->tx_packet[i] = can_tx.tx_packet[i];
            }

            // HAL_CAN_AddTxMessage takes pointers to CAN_HandleTypeDef, CAN_TxHeaderTypeDef, data buffer, and mailbox pointer
            can_status = HAL_CAN_AddTxMessage(canbus->hcan, canbus->tx_header, canbus->tx_packet, &canbus->tx_mailbox);

            if (can_status != HAL_OK) {
                data->canbus_fault = 1; 
            }
        }
    }
}


TaskHandle_t create_can_transceiver_task(app_data *data) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        can_transceiver_task,            // Task function
        "CAN Transceiver",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        CAN_PRIO,                // Priority (adjust as needed)
        &handle                  // Task handle
    );
    return handle;
}
