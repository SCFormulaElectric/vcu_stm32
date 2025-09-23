#include "FreeRTOS.h"
#include "task.h"
#include "Tasks/can_receiver_task.h"

// Task: CAN Receiver

void can_receiver_task(void *argument) {
    app_data_t *data = (app_data_t *) argument;
    QueueHandle_t queue = data->can_rx_queue;
    can_message_t msg;
    for (;;) {
        // TODO: Implement CAN Receiver functionality
        if (xQueueReceive(queue, &msg, portMAX_DELAY))
        {
            if (msg.tx_id == MC_temp1_addr) {
                data->motorControl.temp.INV_Module_A_Temp       = ((uint16_t)msg.tx_packet[1] << 8) | msg.tx_packet[0];
                data->motorControl.temp.INV_Module_B_Temp       = ((uint16_t)msg.tx_packet[3] << 8) | msg.tx_packet[2];
                data->motorControl.temp.INV_Module_C_Temp       = ((uint16_t)msg.tx_packet[5] << 8) | msg.tx_packet[4];
                data->motorControl.temp.INV_GDB_Temp            = ((uint16_t)msg.tx_packet[7] << 8) | msg.tx_packet[6];
            }
            if (msg.tx_id == MC_temp2_addr) {
                data->motorControl.temp.INV_Control_Board_Temp  = ((uint16_t)msg.tx_packet[1] << 8) | msg.tx_packet[0];
                data->motorControl.temp.INV_RTD1_Temperature    = ((uint16_t)msg.tx_packet[3] << 8) | msg.tx_packet[2];
                data->motorControl.temp.INV_RTD2_Temperature    = ((uint16_t)msg.tx_packet[5] << 8) | msg.tx_packet[4];
                data->motorControl.temp.INV_Hot_Spot_Temp_Motor = ((uint16_t)msg.tx_packet[7] << 8) | msg.tx_packet[6];
            }
            if (msg.tx_id == MC_temp3_addr) {
                data->motorControl.temp.INV_Coolant_Temp        = ((uint16_t)msg.tx_packet[1] << 8) | msg.tx_packet[0];
                data->motorControl.temp.INV_Hot_Spot_Temp_Inverter = ((uint16_t)msg.tx_packet[3] << 8) | msg.tx_packet[2];
                data->motorControl.temp.INV_Motor_Temp          = ((uint16_t)msg.tx_packet[5] << 8) | msg.tx_packet[4];
                data->motorControl.temp.INV_Torque_Shudder      = ((uint16_t)msg.tx_packet[7] << 8) | msg.tx_packet[6];
            }
        }
    }
}

TaskHandle_t create_can_receiver_task(app_data *data) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        can_receiver_task,            // Task function
        "CAN Receiver",               // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        CAN_receiver_task_PRIO + 1,    // Priority (adjust as needed)
        &handle                  // Task handle
    );
    return handle;
}
