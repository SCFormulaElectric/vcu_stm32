#include "Tasks/motor_controller_task.h"
#include "app.h"

// Task: Motor Controller

void motor_controller_task(void *argument) {
    app_data *data = (app_data *)argument;
    MotorControl_t *motorControl = &data->motorControl;
    QueueHandle_t can_tx_queue = data->can_tx_queue;

    TickType_t start = xTaskGetTickCount();
    state_t task_state = STATE_DISABLE;
    bool disabled_sent = true;

    motorControl->lastTorqueCommand = 0;
    can_message_t free_roll = create_motor_controller_command(0, 0, true, false, false, false, 0);
    can_message_t clear_fault = create_motor_controller_rw_command(20, 1, 0);

    for (;;) {
        switch(task_state) {
            case STATE_ENABLE:
                nt16_t throttle = (int16_t)getThrottle();  
                //Checks for faults
                else if (motorControl->opState == throttle_error || motorControl->opState == plausibility_error || is_fault(motorControl->fault)) {
                        (void)xQueueSend(can_tx_queue, &free_roll, pdMS_TO_TICKS(5));
                        motorControl->lastTorqueCommand = 0;
                        task_state = STATE_DISABLE;
                }
                // Normal operation
                if (throttle > 1000 && throttle < 1300) throttle = 1000;
                if (throttle < 20 || throttle > 1400) {
                    (void)xQueueSend(can_tx_queue, &free_roll, pdMS_TO_TICKS(5));
                    motorControl->lastTorqueCommand = 0;
                } 
                else {
                    uint16_t torque_x10 = (uint16_t)((throttle * MAX_TORQUE * 10) / 1000);
                        can_message_t torque_cmd = create_motor_controller_command(torque_x10, 0, true, true, false, false, 0);
                        if (xQueueSend(can_tx_queue, &torque_cmd, pdMS_TO_TICKS(5)) == pdPASS) {
                            motorControl->lastTorqueCommand = torque_x10;
                        }
                    }
                }
                break;

            case STATE_DISABLE: 
                //checks that there is no longer a throttle or plausibility error
                if(motorControl->opState != throttle_error || motorControl->opState != plausibility_error)
                {
                    //if there is a motor control fault that can be cleared clear it
                    if(motorControl->fault == CAN_Command_Message_Lost_Fault) {
                        (void)xQueueSend(can_tx_queue, &clear_fault, pdMS_TO_TICKS(5));
                        task_state = STATE_WAIT;
                    }
                    //other faults

                    //if there is no motor control fault go straight to enable
                    else if (!is_fault(motorControl->fault)) {
                        task_state = STATE_ENABLE;
                        (void)xQueueSend(can_tx_queue, &free_roll, pdMS_TO_TICKS(5));
                        motorControl->lastTorqueCommand = 0;
                    }
                }
                break;

            case STATE_WAIT:
                //checks return message
                if(motorControl->param_response.Write_Success) {
                    motorControl->param_response = 0;
                    task_state = STATE_ENABLE;
                }
                else {
                    task_state = STATE_DISABLE;
                }
                (void)xQueueSend(can_tx_queue, &free_roll, pdMS_TO_TICKS(5));
        vTaskDelayUntil(&start, pdMS_TO_TICKS(motor_control_interval));
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
    bool direction, 
    bool inverter_en, 
    bool inverter_discharge, 
    bool speed_mode_enable,
    uint16_t torque_limit, ) 
{
    can_message_t motor_command;
    motor_command.tx_id = 0x0C0;
    motor_command.tx_packet[0] = torque & 0xFF;        //lower
    motor_command.tx_packet[1] = (torque >> 8) & 0xFF; //upper
    motor_command.tx_packet[2] = speed & 0xFF;         //lower
    motor_command.tx_packet[3] = (speed >> 8) & 0xFF;  //upper
    motor_command.tx_packet[4] = direction;            //0-reverse 1-forward
    motor_command.tx_packet[5] = (inverter_en & 0x01)  // bit 0
        | ((inverter_discharge & 0x01) << 1) // bit 1
        | ((speed_mode_enable & 0x01) << 2); // bit 2
    motor_command.tx_packet[6] = torque_limit & 0xFF;
    motor_command.tx_packet[7] = (torque_limit >> 8) & 0xFF;
    return motor_command;
}

can_message_t create_motor_controller_rw_command(
    uint16_t param_addr, //Bytes 0-1
    bool     rw,         //Bytes 2
    uint16_t data,       //Bytes 4-5
    )   
{
    can_message_t rw_command;
    rw_command.tx_id = 0x0C1;
    rw_command.tx_packet[0] = param_addr & 0xFF;
    rw_command.tx_packet[1] = (param_addr >> 8) & 0xFF;
    rw_command.tx_packet[2] = rw & 0xFF;         
    rw_command.tx_packet[4] = data & 0xFF; 
    rw_command.tx_packet[5] = (data >> 8) & 0xFF;   

    return rw_command;
}

/*It is recommended to send regularly scheduled CAN commands to the inverter when in CAN control
mode. Some limiting functions act upon the receipt of the command and may not work properly if
significant time exists between CAN commands. i.e. > 1 [s].*/
/*default range of CAN message IDs is 0x0A0 – 0x0CF*/
//0x0C1 Read Write
//0x0C2 Response
/*20 Fault Clear Boolean
Writing a 0 to this parameter clears any
active faults. This command can be sent
through CAN in CAN as well as VSM mode.
*/