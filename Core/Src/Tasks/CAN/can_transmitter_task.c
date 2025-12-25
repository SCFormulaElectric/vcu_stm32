#include "Tasks/CAN/can_transmitter_task.h"


void can_transmitter_task(void *argument)
{
    app_data_t *data = (app_data_t *)argument;
    can_bus_t *can_bus = &data->can_bus;

    can_tx_message_t can_tx;
    CAN_TxHeaderTypeDef tx_header;
    uint8_t tx_data[8];
    uint32_t txMailbox;
    uint8_t mailbox_found = 0;
    TickType_t start;
    TickType_t mailbox_timer_start;

    for (;;)
    {
        start = xTaskGetTickCount();
        if (xQueueReceive(can_bus->can_tx_queue, &can_tx, CAN_TX_MAILBOX_WAIT_MS) != pdPASS) {
            vTaskDelay(pdMS_TO_TICKS(CAN_TX_TASK_DELAY_MS));
        }
        mailbox_timer_start = xTaskGetTickCount();
        
        mailbox_found = 0;
        while ((xTaskGetTickCount() - mailbox_timer_start) < pdMS_TO_TICKS(CAN_TX_MAILBOX_WAIT_MS))
        {
            if (HAL_CAN_GetTxMailboxesFreeLevel(can_bus->hcan) > 0)
            {
                mailbox_found = 1;
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(1));
        }

        if (0 == mailbox_found)
        {
            xQueueSendToFront(can_bus->can_tx_queue, &can_tx, 0);
            vTaskDelay(pdMS_TO_TICKS(CAN_TX_TASK_DELAY_MS));
            continue;
        }
        
        tx_header.StdId = can_tx.tx_id;
        tx_header.IDE   = CAN_ID_STD;
        tx_header.RTR   = CAN_RTR_DATA;
        tx_header.DLC   = can_tx.dlc;

        memcpy(tx_data, can_tx.tx_packet, can_tx.dlc);

        if (HAL_CAN_AddTxMessage(
                can_bus->hcan,
                &tx_header,
                tx_data,
                &txMailbox) != HAL_OK)
        {
            //LOG CAN BUS FAULT HERE
        }
        vTaskDelayUntil(&start, CAN_TX_TASK_DELAY_MS)
    }
}




task_entry_t create_can_transmitter_task(app_data_t *data) {
    task_entry_t entry = {0};

    xTaskCreate(
        can_transmitter_task,
        "CAN Transmitter",
        256,
        data,
        CAN_PRIO,
        &entry.handle
    );

    vTaskSuspend(entry.handle);

    entry.name = "can_transmitter";
    return entry;
}
