#include "Tasks/CAN/can_receiver_task.h"

void can_receiver_task(void *argument) {
    app_data_t *data = (app_data_t *) argument;
    QueueHandle_t queue = data->can_bus.can_rx_queue;
    can_rx_message_t msg;

    for (;;) {
        TickType_t start = xTaskGetTickCount();
        while (xQueueReceive(queue, &msg, 0) == pdTRUE)
        {
            if (IS_MOTOR_CONTROLLER_ID(msg.id)) {
                process_MC_msg(data, msg);
            }
            else if (IS_DASHBOARD_ID(msg.id)) {
                process_Dashboard_msg(data, msg);
            }
        }
        xEventGroupSetBits(data->idwg_group, WD_CAN_RX);
        vTaskDelayUntil(&start, pdMS_TO_TICKS(CAN_RX_DELAY_MS));
    }
}

void process_MC_msg(app_data_t *data, can_rx_message_t message) {
    if (message.dlc < 8U) {
        return;
    }
    if (message.id == 0x0A0U) {
        data->motorControl.temp.INV_Module_A_Temp = ((uint16_t)message.data[1] << 8) | message.data[0];
        data->motorControl.temp.INV_Module_B_Temp = ((uint16_t)message.data[3] << 8) | message.data[2];
        data->motorControl.temp.INV_Module_C_Temp = ((uint16_t)message.data[5] << 8) | message.data[4];
        data->motorControl.temp.INV_GDB_Temp = ((uint16_t)message.data[7] << 8) | message.data[6];
    }
    else if (message.id == 0x0A1U) {
        data->motorControl.temp.INV_Control_Board_Temp = ((uint16_t)message.data[1] << 8) | message.data[0];
        data->motorControl.temp.INV_RTD1_Temperature = ((uint16_t)message.data[3] << 8) | message.data[2];
        data->motorControl.temp.INV_RTD2_Temperature = ((uint16_t)message.data[5] << 8) | message.data[4];
        data->motorControl.temp.INV_Hot_Spot_Temp_Motor = ((uint16_t)message.data[7] << 8) | message.data[6];
    }
    else if (message.id == 0x0A2U) {
        data->motorControl.temp.INV_Coolant_Temp = ((uint16_t)message.data[1] << 8) | message.data[0];
        data->motorControl.temp.INV_Hot_Spot_Temp_Inverter = ((uint16_t)message.data[3] << 8) | message.data[2];
        data->motorControl.temp.INV_Motor_Temp = ((uint16_t)message.data[5] << 8) | message.data[4];
        data->motorControl.temp.INV_Torque_Shudder = ((uint16_t)message.data[7] << 8) | message.data[6];
    }
    else if (message.id == 0x0ABU) {
        data->motorControl.fault_codes.INV_Post_Fault_Lo = ((uint16_t)message.data[1] << 8) | message.data[0];
        data->motorControl.fault_codes.INV_Post_Fault_Hi = ((uint16_t)message.data[3] << 8) | message.data[2];
        data->motorControl.fault_codes.INV_Run_Fault_Lo = ((uint16_t)message.data[5] << 8) | message.data[4];
        data->motorControl.fault_codes.INV_Run_Fault_Hi = ((uint16_t)message.data[7] << 8) | message.data[6];
        data->motorControl.fault_codes_last_tick = xTaskGetTickCount();
        data->motorControl.fault_codes_valid = 1U;
    }
    else {
        serial_log("CAN address in MC range, but not recognized");
    }
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
    vTaskSuspend(entry.handle);

    entry.name = "can_rx";
    return entry;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
    if (hcan != &hcan1 || app.can_bus.can_rx_queue == NULL) {
        return;
    }
    CAN_RxHeaderTypeDef header;
    can_rx_message_t message = {0};
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &header, message.data) != HAL_OK) {
        return;
    }
    message.id = (header.IDE == CAN_ID_EXT) ? header.ExtId : header.StdId;
    message.dlc = header.DLC;
    message.is_extended = (header.IDE == CAN_ID_EXT);
    BaseType_t higher_priority_task_woken = pdFALSE;
    if (xQueueSendFromISR(app.can_bus.can_rx_queue, &message, &higher_priority_task_woken) != pdPASS) {
        app.can_bus.rx_dropped++;
    }
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan) {
    if (hcan == &hcan1 && (HAL_CAN_GetError(hcan) & HAL_CAN_ERROR_BOF) != 0U) {
        app.can_bus.bus_off_count++;
    }
}
