#include "FreeRTOS.h"
#include "task.h"
#include "Tasks/can_transceiver_task.h"

// Task: CAN Transceiver
#define DATA_LENGTH = 8;

void can_transceiver_task(void *argument) {
    app_data *data = (app_data *) argument;
    //stored in local variables so it can't be overwritten during a write
    canbus_t *canbus = &data->canbus;
    CAN_HandleTypeDef *hcan = canbus->hcan;
    CAN_TxHeaderTypeDef *tx_header = data->canbus.tx_header;
    uint8_t can_data[DATA_LENGTH];


    for (;;) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        //Make sure can message can't be overidden while writing
        taskENTER_CRITICAL();
        for(int i = 0; i < 8; i++) {
            can_data[i] = canbus->tx_packet[i];
            canbus->tx_packet[i] = 0;
            tx_header->StdId = canbus->tx_header.StdId;
        }
        taskEXIT_CRITICAL();
        can_status = HAL_CAN_AddTxMessage(&canbus->hcan, tx_header, can_data, &canbus->tx_mailbox);
    }
}

TaskHandle_t create_can_transceiver_task(void) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        can_transceiver_task,            // Task function
        "CAN Transceiver",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        NULL,                    // Task parameters
        CAN_PRIO,                // Priority (adjust as needed)
        &handle                  // Task handle
    );
    return handle;
}
