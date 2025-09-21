#include "Tasks/motor_controller_task.h"

// Task: Motor Controller

void motor_controller_task(void *argument) {
    app_data *data = (app_data *) argument;
    MotorControl_t *motorControl = &data->motorControl;
    QueueHandle_t can_tx_queue = data->can_tx_queue;

    for (;;) {
        TickType_t start = xTaskGetTickCount();
        uint16_t throttle = data->getThrottle();
        if(!motorControl->enabled || motorControl->fault) {
        }
        else {
            if (throttle < 0 || throttle > 1400) throttle = 0;
            if (throttle > 1000 && throttle < 1300) throttle = 1000;

            uint16_t torque = (throttle * MAX_TORQUE * 10) / 1000; // must send torque x 10. 300nm requires sending a value of 3000
            can_message_t torque_command = create_motor_controller_command(torque, 0, Forward, 1, 0, 0, 0);

            if (xQueueSend(can_tx_queue, &torque_command, pdMS_TO_TICKS(10)) != pdPASS) {
                //error checking
            }
        }
        vTaskDelayUntil(&start, pdMS_TO_TICKS(1000/motor_control_FREQ));
    }
}

TaskHandle_t create_motor_controller_task(app_data *data) {
    TaskHandle_t handle = NULL;
    xTaskCreate(
        motor_controller_task,   // Task function
        "Motor Controller",      // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        MCT_PRIO + 1,            // Priority (adjust as needed)
        &handle                  // Task handle
    );
    return handle;
}


can_message_t create_motor_controller_command(
    uint16_t torque, 
    uint16_t speed, 
    uint16_t torque_limit, 
    bool direction, 
    bool inverter_en, 
    bool inverter_discharge, 
    bool speed_mode_enable) 
{
    can_message_t motor_command;
    motor_command.tx_id = 0x0C0;
    motor_command.tx_packet[0] = torque & 0xFF;        //lower
    motor_command.tx_packet[1] = (torque >> 8) & 0xFF; //upper
    motor_command.tx_packet[2] = speed & 0xFF;         //lower
    motor_command.tx_packet[3] = (speed >> 8) & 0xFF;  //upper
    motor_command.tx_packet[4] = direction;            //0-reverse 1-forward
    motor_command.tx_packet[5] = inverter_en;   
    motor_command.tx_packet[6] = inverter_discharge;   
    motor_command.tx_packet[7] = speed_mode_enable;   

    return motor_command;
}
/*It is recommended to send regularly scheduled CAN commands to the inverter when in CAN control
mode. Some limiting functions act upon the receipt of the command and may not work properly if
significant time exists between CAN commands. i.e. > 1 [s].*/
/*default range of CAN message IDs is 0x0A0 – 0x0CF*/