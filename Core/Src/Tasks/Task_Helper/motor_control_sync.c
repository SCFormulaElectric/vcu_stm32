#include "Tasks/Task_Helper/motor_control.h"

#include "FreeRTOS.h"
#include "task.h"

void motor_control_publish_fault_status(MotorControl_t *control,
    const fault_codes_t *fault_codes, uint32_t tick)
{
    if (control == NULL || fault_codes == NULL) {
        return;
    }
    taskENTER_CRITICAL();
    control->fault_codes = *fault_codes;
    control->fault_codes_last_tick = tick;
    control->fault_codes_valid = 1U;
    taskEXIT_CRITICAL();
}

void motor_control_read_fault_status(const MotorControl_t *control,
    inverter_fault_status_snapshot_t *snapshot)
{
    if (snapshot == NULL) {
        return;
    }
    if (control == NULL) {
        *snapshot = (inverter_fault_status_snapshot_t){0};
        return;
    }
    taskENTER_CRITICAL();
    snapshot->fault_codes = control->fault_codes;
    snapshot->last_tick = control->fault_codes_last_tick;
    snapshot->valid = control->fault_codes_valid;
    taskEXIT_CRITICAL();
}
