#include "Tasks/Critical/motor_controller_task.h"

// Task: Motor Controller

void motor_controller_task(void *argument) {
    app_data_t *data = (app_data_t *)argument;
    MotorControl_t *motorControl = &data->motorControl;

    state_t task_state = STATE_DISABLE;
    TickType_t last_command_tick = 0;
    
    motorControl->lastTorqueCommand = 0;
    motorControl->torqueCommand = 0;
    const can_tx_message_t free_roll_can_msg = create_motor_controller_command(0, 0, 0, 0, 0, 0, 0);
    
    for (;;) {
        TickType_t start = xTaskGetTickCount();
        inverter_fault_status_snapshot_t inverter_status = {0};
        motor_control_read_fault_status(motorControl, &inverter_status);
        const uint8_t inverter_status_safe =
            inverter_fault_status_snapshot_is_fresh(&inverter_status,
                (uint32_t)start,
                (uint32_t)pdMS_TO_TICKS(CAN_INVERTER_FAULT_STATUS_TIMEOUT_MS)) &&
            !is_fault(&inverter_status.fault_codes);
        // Check we are in the correct car state first
        if (data->car_state != CAR_ENABLE || data->ready_to_drive == 0U ||
            data->safety_faults != SAFETY_FAULT_NONE){
            (void)can_bus_publish_motor_command(&data->can_bus,
                &free_roll_can_msg, 1U);
            motorControl->lastTorqueCommand = 0;
            motorControl->torqueCommand = 0;
            xEventGroupSetBits(data->idwg_group, WD_MOTOR_CONTROLLER);
            vTaskDelayUntil(&start, pdMS_TO_TICKS(motor_control_interval));
            continue;
        }
        switch(task_state) {
            case STATE_ENABLE:
                uint16_t throttle = data->throttle_level;  
                if (motorControl->input_faults.apps_fault != 0U ||
                    motorControl->input_faults.bpps_fault != 0U ||
                    inverter_status_safe == 0U) {
                        (void)can_bus_publish_motor_command(&data->can_bus,
                            &free_roll_can_msg, 1U);
                        motorControl->lastTorqueCommand = 0;
                        motorControl->torqueCommand = 0;
                        task_state = STATE_DISABLE;
                        break;
                }
                if (throttle < THROTTLE_DEADZONE) {
                    (void)can_bus_publish_motor_command(&data->can_bus,
                        &free_roll_can_msg, 1U);
                    motorControl->lastTorqueCommand = 0;
                    motorControl->torqueCommand = 0;
                }
                else {
                    const uint16_t shaped_throttle = pedal_response_apply(
                        &data->pedal_response, throttle);
                    const uint16_t requested_torque_x10 = (uint16_t)(
                        (shaped_throttle * MAX_TORQUE * 10U) / 1000U);
                    uint16_t torque_x10 = torque_request_limit_rise(
                        motorControl->lastTorqueCommand,
                        requested_torque_x10,
                        MAX_TORQUE_RISE_X10_PER_CYCLE);
                    motorControl->torqueCommand = torque_x10;
                    // @todo @note Check if the cascadia motor controller needs constant torque commands.
                    if (torque_x10 == motorControl->lastTorqueCommand &&
                        (xTaskGetTickCount() - last_command_tick) < pdMS_TO_TICKS(100)) {
                        break;
                    }
                    can_tx_message_t torque_cmd = create_motor_controller_command(torque_x10, 0, 1, 1, 0, 0, 0);
                    if (can_bus_publish_motor_command(&data->can_bus,
                        &torque_cmd, 0U) == pdPASS) {
                        motorControl->lastTorqueCommand = torque_x10;
                        last_command_tick = xTaskGetTickCount();
                    }
                }
                break;

            case STATE_DISABLE: 
                (void)can_bus_publish_motor_command(&data->can_bus,
                    &free_roll_can_msg, 1U);
                motorControl->lastTorqueCommand = 0;
                motorControl->torqueCommand = 0;
                //checks that there is no longer a throttle or plausibility error
                if(motorControl->input_faults.apps_fault == 0U && motorControl->input_faults.bpps_fault == 0U)
                {
                    //if there is no motor control fault go straight to enable
                    if (inverter_status_safe != 0U) {
                        task_state = STATE_ENABLE;
                        motorControl->lastTorqueCommand = 0;
                    }
                    //if there is a motor control fault that can be cleared clear it
                    //other faults
                }
                break;

            case STATE_WAIT:
                //checks return message
                if(motorControl->param_response.Write_Success == 1) {
                    motorControl->param_response = (parameter_response_t){0};
                    task_state = STATE_ENABLE;
                }
                else {
                    task_state = STATE_DISABLE;
                }
                    (void)can_bus_publish_motor_command(&data->can_bus,
                        &free_roll_can_msg, 1U);
                    motorControl->torqueCommand = 0;
                    break;

            }
            xEventGroupSetBits(data->idwg_group, WD_MOTOR_CONTROLLER);
            vTaskDelayUntil(&start, pdMS_TO_TICKS(motor_control_interval));
    }
}

task_entry_t create_motor_controller_task(app_data_t *data) {
    task_entry_t entry = {0};
    
    BaseType_t status = xTaskCreate(
        motor_controller_task,   
        "Motor Controller",      // Task name (string)
        MC_STACK_SIZE,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        MCT_PRIO,            // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    configASSERT(status == pdPASS);
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
    can_tx_message_t motor_command = {0};
    motor_command.tx_id = CAN_ID_MC_COMMAND;
    motor_command.dlc = CAN_DLC_MC_COMMAND;
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
    can_tx_message_t rw_command = {0};
    rw_command.dlc = CAN_DLC_MC_PARAMETER_RW;
    rw_command.tx_id = CAN_ID_MC_PARAMETER_RW;
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
/* Broadcast IDs are defined in can_protocol.h. */
/* CAN_ID_MC_PARAMETER_RW: Read/Write; CAN_ID_MC_PARAMETER_RESPONSE: Response. */
/*20 Fault Clear Boolean
Writing a 0 to this parameter clears any
active faults. This command can be sent
through CAN in CAN as well as VSM mode.
*/
