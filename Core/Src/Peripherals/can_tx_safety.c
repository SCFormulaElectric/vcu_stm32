#include "Peripherals/can_tx_safety.h"

void can_tx_safety_make_zero_motor_command(can_tx_message_t *message)
{
    uint32_t index;

    if (message == 0) {
        return;
    }
    message->tx_id = CAN_ID_MC_COMMAND;
    message->dlc = CAN_DLC_MC_COMMAND;
    for (index = 0U; index < 8U; index++) {
        message->tx_packet[index] = 0U;
    }
}

void can_tx_safety_initialize(can_tx_safety_state_t *state,
    can_tx_message_t *initial_motor_command)
{
    if (state != 0) {
        state->queue_failures = 0U;
        state->invalid_dlc_failures = 0U;
        state->mailbox_timeout_failures = 0U;
        state->hal_transmit_failures = 0U;
        state->requeue_failures = 0U;
        state->bus_off_failures = 0U;
        state->generic_motor_command_failures = 0U;
        state->fault_latched = 0U;
        state->inhibit_requested = 1U;
    }
    can_tx_safety_make_zero_motor_command(initial_motor_command);
}

void can_tx_safety_request_inhibit(can_tx_safety_state_t *state,
    uint8_t inhibit_requested)
{
    if (state != 0) {
        state->inhibit_requested = (inhibit_requested != 0U) ? 1U : 0U;
    }
}

void can_tx_safety_latch_failure(can_tx_safety_state_t *state,
    can_tx_failure_t failure)
{
    if (state == 0) {
        return;
    }
    switch (failure) {
        case CAN_TX_FAILURE_QUEUE: state->queue_failures++; break;
        case CAN_TX_FAILURE_INVALID_DLC: state->invalid_dlc_failures++; break;
        case CAN_TX_FAILURE_MAILBOX_TIMEOUT: state->mailbox_timeout_failures++; break;
        case CAN_TX_FAILURE_HAL_TRANSMIT: state->hal_transmit_failures++; break;
        case CAN_TX_FAILURE_REQUEUE: state->requeue_failures++; break;
        case CAN_TX_FAILURE_BUS_OFF: state->bus_off_failures++; break;
        case CAN_TX_FAILURE_GENERIC_MOTOR_COMMAND:
            state->generic_motor_command_failures++;
            break;
        default: state->queue_failures++; break;
    }
    state->fault_latched = 1U;
    state->inhibit_requested = 1U;
}

uint8_t can_tx_safety_positive_allowed(const can_tx_safety_state_t *state)
{
    return (state != 0 && state->fault_latched == 0U &&
        state->inhibit_requested == 0U) ? 1U : 0U;
}

uint8_t can_tx_safety_frame_valid(const can_tx_message_t *message)
{
    return (message != 0 && message->dlc <= 8U) ? 1U : 0U;
}

uint8_t can_tx_safety_motor_command_positive(const can_tx_message_t *message)
{
    if (message == 0 || message->tx_id != CAN_ID_MC_COMMAND ||
        message->dlc != CAN_DLC_MC_COMMAND) {
        return 0U;
    }
    return (message->tx_packet[0] != 0U || message->tx_packet[1] != 0U) ?
        1U : 0U;
}

uint8_t can_tx_safety_select_motor_command(can_tx_safety_state_t *state,
    const can_tx_message_t *requested, can_tx_message_t *selected)
{
    if (state == 0 || selected == 0 || requested == 0 ||
        requested->tx_id != CAN_ID_MC_COMMAND ||
        requested->dlc != CAN_DLC_MC_COMMAND) {
        if (state != 0) {
            can_tx_safety_latch_failure(state, CAN_TX_FAILURE_INVALID_DLC);
        }
        can_tx_safety_make_zero_motor_command(selected);
        return 0U;
    }
    if (can_tx_safety_positive_allowed(state) == 0U) {
        can_tx_safety_make_zero_motor_command(selected);
        return 1U;
    }
    *selected = *requested;
    return 1U;
}
