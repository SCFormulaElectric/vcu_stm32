#include "Tasks/CAN/can_transmitter_task.h"

static void can_bus_latch_task_failure(can_bus_t *can_bus,
    can_tx_failure_t failure)
{
    can_tx_safety_latch_failure(&can_bus->tx_safety, failure);
    can_bus->tx_errors++;
}

static BaseType_t can_bus_overwrite_motor_slot(can_bus_t *can_bus,
    const can_tx_message_t *message)
{
    if (can_bus == NULL || can_bus->motor_command_queue == NULL ||
        message == NULL ||
        xQueueOverwrite(can_bus->motor_command_queue, message) != pdPASS) {
        if (can_bus != NULL) {
            can_bus_latch_task_failure(can_bus, CAN_TX_FAILURE_QUEUE);
        }
        return pdFAIL;
    }
    if (can_bus->tx_task_handle != NULL) {
        xTaskNotifyGive(can_bus->tx_task_handle);
    }
    return pdPASS;
}

static void can_bus_retain_zero_after_failure(can_bus_t *can_bus)
{
    can_tx_message_t zero_command;

    can_tx_safety_request_inhibit(&can_bus->tx_safety, 1U);
    can_tx_safety_make_zero_motor_command(&zero_command);
    (void)can_bus_overwrite_motor_slot(can_bus, &zero_command);
}

BaseType_t can_bus_publish_motor_command(can_bus_t *can_bus,
    const can_tx_message_t *command, uint8_t inhibit_requested)
{
    can_tx_message_t selected;
    BaseType_t queue_status;
    uint8_t valid;
    uint8_t requested_positive;

    if (can_bus == NULL || command == NULL) {
        return pdFAIL;
    }
    requested_positive = can_tx_safety_motor_command_positive(command);
    taskENTER_CRITICAL();
    /* Publishers may assert the global inhibit, but only the state machine may
     * release it after evaluating every torque prerequisite. */
    if (inhibit_requested != 0U) {
        can_tx_safety_request_inhibit(&can_bus->tx_safety, 1U);
    }
    valid = can_tx_safety_select_motor_command(&can_bus->tx_safety,
        command, &selected);
    if (valid == 0U) {
        can_bus->tx_errors++;
    }
    queue_status = can_bus_overwrite_motor_slot(can_bus, &selected);
    taskEXIT_CRITICAL();
    if (queue_status != pdPASS) {
        return pdFAIL;
    }
    if (valid == 0U || (requested_positive != 0U &&
        can_tx_safety_motor_command_positive(&selected) == 0U)) {
        return pdFAIL;
    }
    return pdPASS;
}

BaseType_t can_bus_request_motor_inhibit(can_bus_t *can_bus)
{
    can_tx_message_t zero_command;
    BaseType_t queue_status;

    if (can_bus == NULL) {
        return pdFAIL;
    }
    can_tx_safety_make_zero_motor_command(&zero_command);
    taskENTER_CRITICAL();
    can_tx_safety_request_inhibit(&can_bus->tx_safety, 1U);
    queue_status = can_bus_overwrite_motor_slot(can_bus, &zero_command);
    taskEXIT_CRITICAL();
    return queue_status;
}

BaseType_t can_bus_release_motor_inhibit(can_bus_t *can_bus)
{
    BaseType_t status = pdFAIL;

    if (can_bus == NULL) {
        return pdFAIL;
    }
    taskENTER_CRITICAL();
    if (can_bus->rx_fault_latched == 0U &&
        can_bus->tx_safety.fault_latched == 0U) {
        can_tx_safety_request_inhibit(&can_bus->tx_safety, 0U);
        status = pdPASS;
    }
    taskEXIT_CRITICAL();
    return status;
}

BaseType_t can_bus_queue_generic_message(can_bus_t *can_bus,
    const can_tx_message_t *message)
{
    if (can_bus == NULL || message == NULL ||
        can_tx_safety_frame_valid(message) == 0U) {
        if (can_bus != NULL) {
            can_bus_latch_task_failure(can_bus, CAN_TX_FAILURE_INVALID_DLC);
        }
        return pdFAIL;
    }
    if (message->tx_id == CAN_ID_MC_COMMAND) {
        can_bus_latch_task_failure(can_bus,
            CAN_TX_FAILURE_GENERIC_MOTOR_COMMAND);
        return pdFAIL;
    }
    if (xQueueSend(can_bus->can_tx_queue, message, 0U) != pdPASS) {
        can_bus_latch_task_failure(can_bus, CAN_TX_FAILURE_QUEUE);
        return pdFAIL;
    }
    if (can_bus->tx_task_handle != NULL) {
        xTaskNotifyGive(can_bus->tx_task_handle);
    }
    return pdPASS;
}

void can_bus_force_inhibit_from_isr(can_bus_t *can_bus,
    BaseType_t *higher_priority_task_woken)
{
    can_tx_message_t zero_command;

    if (can_bus == NULL || higher_priority_task_woken == NULL) {
        return;
    }
    can_tx_safety_request_inhibit(&can_bus->tx_safety, 1U);
    can_tx_safety_make_zero_motor_command(&zero_command);
    if (can_bus->motor_command_queue == NULL ||
        xQueueOverwriteFromISR(can_bus->motor_command_queue, &zero_command,
            higher_priority_task_woken) != pdPASS) {
        can_tx_safety_latch_failure(&can_bus->tx_safety,
            CAN_TX_FAILURE_QUEUE);
    }
    if (can_bus->tx_task_handle != NULL) {
        vTaskNotifyGiveFromISR(can_bus->tx_task_handle,
            higher_priority_task_woken);
    }
}

static BaseType_t can_transmit_message(can_bus_t *can_bus,
    can_tx_message_t *message, uint8_t is_motor_command)
{
    CAN_TxHeaderTypeDef tx_header = {0};
    uint8_t tx_data[8] = {0};
    uint32_t tx_mailbox = 0U;
    TickType_t mailbox_started;

    if (can_tx_safety_frame_valid(message) == 0U) {
        can_bus_latch_task_failure(can_bus, CAN_TX_FAILURE_INVALID_DLC);
        can_bus_retain_zero_after_failure(can_bus);
        return pdFAIL;
    }
    if (is_motor_command == 0U && message->tx_id == CAN_ID_MC_COMMAND) {
        can_bus_latch_task_failure(can_bus,
            CAN_TX_FAILURE_GENERIC_MOTOR_COMMAND);
        can_bus_retain_zero_after_failure(can_bus);
        return pdFAIL;
    }
    if (is_motor_command != 0U) {
        can_tx_message_t selected;
        if (can_tx_safety_select_motor_command(&can_bus->tx_safety,
            message, &selected) == 0U) {
            can_bus->tx_errors++;
        }
        *message = selected;
    }

    mailbox_started = xTaskGetTickCount();
    while (HAL_CAN_GetTxMailboxesFreeLevel(can_bus->hcan) == 0U &&
        (xTaskGetTickCount() - mailbox_started) <
            pdMS_TO_TICKS(CAN_TX_MAILBOX_WAIT_MS)) {
        vTaskDelay(pdMS_TO_TICKS(1U));
    }
    if (HAL_CAN_GetTxMailboxesFreeLevel(can_bus->hcan) == 0U) {
        can_bus_latch_task_failure(can_bus, CAN_TX_FAILURE_MAILBOX_TIMEOUT);
        if (is_motor_command == 0U &&
            xQueueSendToFront(can_bus->can_tx_queue, message, 0U) != pdPASS) {
            can_bus_latch_task_failure(can_bus, CAN_TX_FAILURE_REQUEUE);
        }
        can_bus_retain_zero_after_failure(can_bus);
        return pdFAIL;
    }

    tx_header.ExtId = 0U;
    tx_header.IDE = CAN_ID_STD;
    tx_header.RTR = CAN_RTR_DATA;
    tx_header.TransmitGlobalTime = DISABLE;
    /* Keep final arbitration, frame copy, and HAL mailbox handoff indivisible
     * with respect to the CAN error ISR. Mailbox waiting remains outside. */
    taskENTER_CRITICAL();
    if (is_motor_command != 0U) {
        can_tx_message_t selected;
        if (can_tx_safety_select_motor_command(&can_bus->tx_safety,
            message, &selected) == 0U) {
            can_bus->tx_errors++;
        }
        *message = selected;
    }
    tx_header.StdId = message->tx_id;
    tx_header.DLC = message->dlc;
    memset(tx_data, 0, sizeof(tx_data));
    memcpy(tx_data, message->tx_packet, message->dlc);
    {
        const HAL_StatusTypeDef transmit_status = HAL_CAN_AddTxMessage(
            can_bus->hcan, &tx_header, tx_data, &tx_mailbox);
        taskEXIT_CRITICAL();
        if (transmit_status != HAL_OK) {
            can_bus_latch_task_failure(can_bus,
                CAN_TX_FAILURE_HAL_TRANSMIT);
            can_bus_retain_zero_after_failure(can_bus);
            return pdFAIL;
        }
    }
    return pdPASS;
}

void can_transmitter_task(void *argument)
{
    app_data_t *data = (app_data_t *)argument;
    can_bus_t *can_bus = &data->can_bus;

    for (;;)
    {
        uint32_t frames_processed;

        (void)ulTaskNotifyTake(pdTRUE,
            pdMS_TO_TICKS(CAN_TX_FALLBACK_WAKE_MS));
        for (frames_processed = 0U;
            frames_processed < CAN_TX_MAX_FRAMES_PER_WAKE;
            frames_processed++) {
            can_tx_message_t can_tx;
            uint8_t is_motor_command = 0U;

            if (xQueueReceive(can_bus->motor_command_queue, &can_tx, 0U) ==
                pdPASS) {
                is_motor_command = 1U;
            } else if (xQueueReceive(can_bus->can_tx_queue, &can_tx, 0U) !=
                pdPASS) {
                break;
            }
            if (can_transmit_message(can_bus, &can_tx,
                is_motor_command) != pdPASS) {
                break;
            }
        }
        xEventGroupSetBits(data->idwg_group, WD_CAN_TX);
    }
}




task_entry_t create_can_transmitter_task(app_data_t *data) {
    task_entry_t entry = {0};

    BaseType_t status = xTaskCreate(
        can_transmitter_task,
        "CAN TX",
        CAN_TX_STACK_SIZE,
        data,
        CAN_PRIO,
        &entry.handle
    );

    configASSERT(status == pdPASS);
    data->can_bus.tx_task_handle = entry.handle;
    vTaskSuspend(entry.handle);

    entry.name = "can_tx";
    return entry;
}
