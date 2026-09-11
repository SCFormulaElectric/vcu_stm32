#include "Tasks/CAN/can_receiver_task.h"
#include "Peripherals/can_rx_policy.h"
#include "Peripherals/bms_client.h"

static void process_bms_message(app_data_t *data,
    const can_rx_message_t *message)
{
    uint8_t follow_up[8] = {0};

    if (bms_client_process_frame(&data->bms, message->id, message->dlc,
        message->data, (uint32_t)xTaskGetTickCount(), follow_up) != 0U) {
        can_tx_message_t command = {0};
        command.tx_id = BMS_CAN_COMMAND_ID;
        command.dlc = BMS_CAN_FRAME_DLC;
        memcpy(command.tx_packet, follow_up, sizeof(command.tx_packet));
        (void)can_bus_queue_generic_message(&data->can_bus, &command);
    }
}

static void process_fault_status(app_data_t *data,
    const can_rx_message_t *message)
{
    fault_codes_t decoded = {0};

    decoded.INV_Post_Fault_Lo =
        ((uint16_t)message->data[1] << 8) | message->data[0];
    decoded.INV_Post_Fault_Hi =
        ((uint16_t)message->data[3] << 8) | message->data[2];
    decoded.INV_Run_Fault_Lo =
        ((uint16_t)message->data[5] << 8) | message->data[4];
    decoded.INV_Run_Fault_Hi =
        ((uint16_t)message->data[7] << 8) | message->data[6];
    motor_control_publish_fault_status(&data->motorControl, &decoded,
        (uint32_t)xTaskGetTickCount());
}

void can_receiver_task(void *argument) {
    app_data_t *data = (app_data_t *) argument;
    QueueHandle_t queue = data->can_bus.can_rx_queue;
    can_rx_message_t msg;

    for (;;) {
        uint32_t generic_processed;

        (void)ulTaskNotifyTake(pdTRUE,
            pdMS_TO_TICKS(CAN_RX_FALLBACK_WAKE_MS));
        while (xQueueReceive(data->can_bus.can_rx_fault_queue, &msg, 0U) ==
            pdPASS) {
            process_fault_status(data, &msg);
        }
        for (generic_processed = 0U;
            generic_processed < CAN_RX_MAX_GENERIC_PER_WAKE;
            generic_processed++) {
            if (xQueueReceive(data->can_bus.can_rx_fault_queue, &msg, 0U) ==
                pdPASS) {
                process_fault_status(data, &msg);
                continue;
            }
            if (xQueueReceive(queue, &msg, 0U) != pdPASS) {
                break;
            }
            if (msg.id == CAN_ID_MC_PARAMETER_RESPONSE &&
                msg.dlc == CAN_DLC_MC_PARAMETER_RESPONSE) {
                parameter_response_t response = {0};
                response.Parameter_Address =
                    ((uint16_t)msg.data[1] << 8) | msg.data[0];
                response.Write_Success = msg.data[2];
                response.Data = ((uint16_t)msg.data[5] << 8) | msg.data[4];
                data->motorControl.param_response = response;
            }
            else if (IS_MOTOR_CONTROLLER_ID(msg.id)) {
                process_MC_msg(data, msg);
            }
            else if (IS_DASHBOARD_ID(msg.id)) {
                process_Dashboard_msg(data, msg);
            }
        }
        for (generic_processed = 0U;
            generic_processed < BMS_CAN_QUEUE_LENGTH;
            generic_processed++) {
            if (xQueueReceive(data->can_bus.can_rx_fault_queue, &msg, 0U) ==
                pdPASS) {
                process_fault_status(data, &msg);
                continue;
            }
            if (xQueueReceive(data->can_bus.bms_rx_queue, &msg, 0U) !=
                pdPASS) {
                break;
            }
            process_bms_message(data, &msg);
        }
        while (xQueueReceive(data->can_bus.can_rx_fault_queue, &msg, 0U) ==
            pdPASS) {
            process_fault_status(data, &msg);
        }
        xEventGroupSetBits(data->idwg_group, WD_CAN_RX);
    }
}

void process_MC_msg(app_data_t *data, can_rx_message_t message) {
    if (message.dlc != CAN_DLC_MC_STATUS) {
        return;
    }
    if (message.id == CAN_ID_MC_TEMPERATURE_1) {
        data->motorControl.temp.INV_Module_A_Temp = ((uint16_t)message.data[1] << 8) | message.data[0];
        data->motorControl.temp.INV_Module_B_Temp = ((uint16_t)message.data[3] << 8) | message.data[2];
        data->motorControl.temp.INV_Module_C_Temp = ((uint16_t)message.data[5] << 8) | message.data[4];
        data->motorControl.temp.INV_GDB_Temp = ((uint16_t)message.data[7] << 8) | message.data[6];
    }
    else if (message.id == CAN_ID_MC_TEMPERATURE_2) {
        data->motorControl.temp.INV_Control_Board_Temp = ((uint16_t)message.data[1] << 8) | message.data[0];
        data->motorControl.temp.INV_RTD1_Temperature = ((uint16_t)message.data[3] << 8) | message.data[2];
        data->motorControl.temp.INV_RTD2_Temperature = ((uint16_t)message.data[5] << 8) | message.data[4];
        data->motorControl.temp.INV_Hot_Spot_Temp_Motor = ((uint16_t)message.data[7] << 8) | message.data[6];
    }
    else if (message.id == CAN_ID_MC_TEMPERATURE_3) {
        data->motorControl.temp.INV_Coolant_Temp = ((uint16_t)message.data[1] << 8) | message.data[0];
        data->motorControl.temp.INV_Hot_Spot_Temp_Inverter = ((uint16_t)message.data[3] << 8) | message.data[2];
        data->motorControl.temp.INV_Motor_Temp = ((uint16_t)message.data[5] << 8) | message.data[4];
        data->motorControl.temp.INV_Torque_Shudder = ((uint16_t)message.data[7] << 8) | message.data[6];
    }
    /* Expected but unused Cascadia broadcasts are ignored without logging. */
}

void process_Dashboard_msg(app_data_t *data, can_rx_message_t message) {
    (void)data;
    (void)message;
}


task_entry_t create_can_receiver_task(app_data_t *data) {
    task_entry_t entry = {0};

    BaseType_t status = xTaskCreate(
        can_receiver_task,
        "CAN Receiver",
        CAN_RX_STACK_SIZE,
        data,
        CAN_PRIO,
        &entry.handle
    );

    configASSERT(status == pdPASS);
    data->can_bus.rx_task_handle = entry.handle;
    vTaskSuspend(entry.handle);

    entry.name = "can_rx";
    return entry;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    CAN_RxHeaderTypeDef header = {0};
    can_rx_message_t message = {0};
    can_rx_frame_class_t frame_class;
    BaseType_t higher_priority_task_woken = pdFALSE;
    BaseType_t queued = pdFALSE;

    if (hcan != &hcan1) {
        return;
    }
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &header, message.data) != HAL_OK) {
        app.can_bus.rx_fault_latched = 1U;
        app.can_bus.rx_malformed++;
        can_bus_force_inhibit_from_isr(&app.can_bus,
            &higher_priority_task_woken);
        portYIELD_FROM_ISR(higher_priority_task_woken);
        return;
    }
    if (header.DLC > 8U) {
        app.can_bus.rx_fault_latched = 1U;
        app.can_bus.rx_malformed++;
        can_bus_force_inhibit_from_isr(&app.can_bus,
            &higher_priority_task_woken);
        portYIELD_FROM_ISR(higher_priority_task_woken);
        return;
    }
    message.id = (header.IDE == CAN_ID_EXT) ? header.ExtId : header.StdId;
    message.dlc = (uint8_t)header.DLC;
    message.is_extended = (header.IDE != CAN_ID_STD) ? 1U : 0U;
    frame_class = can_rx_classify_frame(message.id, message.dlc,
        message.is_extended, (header.RTR != CAN_RTR_DATA) ? 1U : 0U);
    switch (frame_class) {
        case CAN_RX_CLASS_CRITICAL_FAULT_STATUS:
            if (app.can_bus.can_rx_fault_queue != NULL &&
                xQueueOverwriteFromISR(app.can_bus.can_rx_fault_queue,
                    &message, &higher_priority_task_woken) == pdPASS) {
                app.can_bus.rx_critical_updates++;
                queued = pdTRUE;
            } else {
                app.can_bus.rx_fault_latched = 1U;
                app.can_bus.rx_dropped++;
                can_bus_force_inhibit_from_isr(&app.can_bus,
                    &higher_priority_task_woken);
            }
            break;
        case CAN_RX_CLASS_PARAMETER_RESPONSE:
        case CAN_RX_CLASS_USED_INVERTER_STATUS:
        case CAN_RX_CLASS_DASHBOARD:
            if (app.can_bus.can_rx_queue != NULL &&
                xQueueSendFromISR(app.can_bus.can_rx_queue, &message,
                    &higher_priority_task_woken) == pdPASS) {
                queued = pdTRUE;
            } else {
                app.can_bus.rx_dropped++;
            }
            break;
        case CAN_RX_CLASS_BMS_TELEMETRY:
        case CAN_RX_CLASS_BMS_SUMMARY:
        case CAN_RX_CLASS_BMS_CONTROL:
            if (app.can_bus.bms_rx_queue != NULL &&
                xQueueSendFromISR(app.can_bus.bms_rx_queue, &message,
                    &higher_priority_task_woken) == pdPASS) {
                queued = pdTRUE;
            } else {
                app.can_bus.bms_rx_dropped++;
            }
            break;
        case CAN_RX_CLASS_UNUSED_INVERTER_STATUS:
            break;
        case CAN_RX_CLASS_MALFORMED:
            app.can_bus.rx_fault_latched = 1U;
            app.can_bus.rx_malformed++;
            can_bus_force_inhibit_from_isr(&app.can_bus,
                &higher_priority_task_woken);
            break;
        case CAN_RX_CLASS_BMS_MALFORMED:
            app.can_bus.rx_malformed++;
            break;
        case CAN_RX_CLASS_REJECTED:
        default:
            app.can_bus.rx_rejected++;
            break;
    }
    if (queued != pdFALSE && app.can_bus.rx_task_handle != NULL) {
        vTaskNotifyGiveFromISR(app.can_bus.rx_task_handle,
            &higher_priority_task_woken);
    }
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan) {
    uint32_t errors;

    if (hcan != &hcan1) {
        return;
    }
    errors = HAL_CAN_GetError(hcan);
    if ((errors & HAL_CAN_ERROR_RX_FOV0) != 0U) {
        BaseType_t higher_priority_task_woken = pdFALSE;
        app.can_bus.rx_fault_latched = 1U;
        app.can_bus.rx_dropped++;
        can_bus_force_inhibit_from_isr(&app.can_bus,
            &higher_priority_task_woken);
        portYIELD_FROM_ISR(higher_priority_task_woken);
    }
    if ((errors & HAL_CAN_ERROR_BOF) != 0U) {
        BaseType_t higher_priority_task_woken = pdFALSE;
        app.can_bus.bus_off_count++;
        can_tx_safety_latch_failure(&app.can_bus.tx_safety,
            CAN_TX_FAILURE_BUS_OFF);
        can_bus_force_inhibit_from_isr(&app.can_bus,
            &higher_priority_task_woken);
        portYIELD_FROM_ISR(higher_priority_task_woken);
    }
}
