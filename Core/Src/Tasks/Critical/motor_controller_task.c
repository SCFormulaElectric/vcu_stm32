#include "Tasks/Critical/motor_controller_task.h"

// Task: Motor Controller

void motor_controller_task(void *argument) {
    app_data_t *data = (app_data_t *)argument;
    MotorControl_t *motorControl = &data->motorControl;

    state_t task_state = STATE_DISABLE;
    uint8_t disabled_sent = 1;
    
    motorControl->lastTorqueCommand = 0;
    const can_tx_message_t free_roll_can_msg = create_motor_controller_command(0, 0, 0, 0, 0, 0, 0);
    const can_tx_message_t clear_fault_can_msg = create_motor_controller_rw_command(20, 1, 0);
    
    for (;;) {
        TickType_t start = xTaskGetTickCount();
        // Check we are in the correct car state first
        if (data->car_state != CAR_ENABLE){
            vTaskDelayUntil(&start, pdMS_TO_TICKS(motor_control_interval));
            continue;
        }
        switch(task_state) {
            case STATE_ENABLE:
                uint16_t throttle = data->throttle_level;  
                if (motorControl->input_faults.apps_fault == 1 || motorControl->input_faults.bpps_fault == 1 || is_fault(motorControl->fault)) {
                        (void)xQueueSend(data->can_bus.can_tx_queue, &free_roll_can_msg, pdMS_TO_TICKS(MC_QUEUE_WAIT_MS));
                        motorControl->lastTorqueCommand = 0;
                        task_state = STATE_DISABLE;
                        break;
                }
                if (throttle < THROTTLE_DEADZONE) {
                    (void)xQueueSend(data->can_bus.can_tx_queue, &free_roll_can_msg, pdMS_TO_TICKS(MC_QUEUE_WAIT_MS));
                    motorControl->lastTorqueCommand = 0;
                }
                else {
                    uint16_t torque_x10 = (uint16_t)((throttle * MAX_TORQUE * 10) / 1000);
                    // @todo @note Check if the cascadia motor controller needs constant torque commands.
                    if (torque_x10 == motorControl->lastTorqueCommand) {
                        break;
                    }
                    can_tx_message_t torque_cmd = create_motor_controller_command(torque_x10, 0, 1, 1, 0, 0, 0);
                    if (xQueueSend(data->can_bus.can_tx_queue, &torque_cmd, pdMS_TO_TICKS(MC_QUEUE_WAIT_MS)) == pdPASS) {
                        motorControl->lastTorqueCommand = torque_x10;
                    }
                }
                break;

            case STATE_DISABLE: 
                //checks that there is no longer a throttle or plausibility error
                if(motorControl->input_faults.apps_fault != 1 || motorControl->input_faults.bpps_fault != 1)
                {
                    //if there is no motor control fault go straight to enable
                    if (!is_fault(motorControl->fault)) {
                        task_state = STATE_ENABLE;
                        motorControl->lastTorqueCommand = 0;
                    }
                    //if there is a motor control fault that can be cleared clear it
                    else if(motorControl->fault == CAN_Command_Message_Lost_Fault) {
                        (void)xQueueSend(data->can_bus.can_tx_queue, &clear_fault_can_msg, pdMS_TO_TICKS(MC_QUEUE_WAIT_MS));
                        task_state = STATE_WAIT;
                    }
                    //other faults

                }
                break;

            case STATE_WAIT:
                //checks return message
                if(motorControl->param_response.Write_Success == 1) {
                    motorControl->param_response = {0};
                    task_state = STATE_ENABLE;
                }
                else {
                    task_state = STATE_DISABLE;
                }
                (void)xQueueSend(data->can_bus.can_tx_queue, &free_roll_can_msg, pdMS_TO_TICKS(MC_QUEUE_WAIT_MS));
                break;

            }

            vTaskDelayUntil(&start, pdMS_TO_TICKS(motor_control_interval));
    }
}

task_entry_t create_motor_controller_task(app_data_t *data) {
    task_entry_t entry = {0};
    
    xTaskCreate(
        motor_controller_task,   
        "Motor Controller",      // Task name (string)
        256,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        MCT_PRIO + 1,            // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    vTaskSuspend(entry.handle);
    entry.name = "motor_controller";
    return entry;
}

can_tx_message_t create_motor_controller_command(
    uint16_t torque, 
    uint16_t speed, 
    uint8_t direction, 
    uint8_t inverter_en, 
    uint8_t inverter_discharge, 
    uint8_t speed_mode_enable,
    uint16_t torque_limit) 
{
    can_tx_message_t motor_command;
    motor_command.tx_id = 0x0C0;
    motor_command.dlc = 8;
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

can_tx_message_t create_motor_controller_rw_command(
    uint16_t param_addr, //Bytes 0-1
    uint8_t     rw,         //Bytes 2
    uint16_t data       //Bytes 4-5
    )   
{
    can_tx_message_t rw_command;
    rw_command.dlc = 6;
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