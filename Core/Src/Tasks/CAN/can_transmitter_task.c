#include "Tasks/CAN/can_transmitter_task.h"

#define DATA_LENGTH 8

void can_transmitter_task(void *argument)
{
    app_data_t *data = (app_data_t *)argument;
    can_bus_t *can_bus = &data->can_bus;
    can_message_t can_tx;

    for (;;)
    {
        if (xQueueReceive(data->can_tx_queue, &can_tx, portMAX_DELAY) == pdPASS)
        {
            if (HAL_CAN_GetTxMailboxesFreeLevel(can_bus->hcan) > 0)
            {
                can_bus->tx_header.StdId = can_tx.tx_id;
                can_bus->tx_header.IDE   = CAN_ID_STD;
                can_bus->tx_header.RTR   = CAN_RTR_DATA;
                can_bus->tx_header.DLC   = DATA_LENGTH;

                memcpy(can_bus->tx_packet, can_tx.tx_packet, DATA_LENGTH);

                if (HAL_CAN_AddTxMessage(
                        can_bus->hcan,
                        &can_bus->tx_header,
                        can_bus->tx_packet,
                        &can_bus->tx_mailbox) != HAL_OK)
                {
                    data->canbus_fault = 1;
                }
            }
        }
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
