#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "Peripherals/can_tx_safety.h"

static can_tx_message_t make_positive_command(uint16_t torque)
{
    can_tx_message_t command = {0};

    command.tx_id = CAN_ID_MC_COMMAND;
    command.dlc = CAN_DLC_MC_COMMAND;
    command.tx_packet[0] = (uint8_t)(torque & 0xFFU);
    command.tx_packet[1] = (uint8_t)(torque >> 8);
    command.tx_packet[5] = 1U;
    return command;
}

static void assert_zero_motor_command(const can_tx_message_t *command)
{
    uint32_t index;

    assert(command->tx_id == CAN_ID_MC_COMMAND);
    assert(command->dlc == CAN_DLC_MC_COMMAND);
    for (index = 0U; index < 8U; index++) {
        assert(command->tx_packet[index] == 0U);
    }
}

static void test_positive_overwritten_by_dominant_zero(void)
{
    can_tx_safety_state_t state;
    can_tx_message_t slot;
    const can_tx_message_t positive = make_positive_command(1200U);

    can_tx_safety_initialize(&state, &slot);
    assert_zero_motor_command(&slot);
    can_tx_safety_request_inhibit(&state, 0U);
    assert(can_tx_safety_select_motor_command(&state, &positive, &slot) == 1U);
    assert(can_tx_safety_motor_command_positive(&slot) == 1U);

    can_tx_safety_request_inhibit(&state, 1U);
    assert(can_tx_safety_select_motor_command(&state, &positive, &slot) == 1U);
    assert_zero_motor_command(&slot);

    /* Generic traffic is a separate queue and cannot mutate the motor slot. */
    {
        can_tx_message_t generic_pressure[10] = {{0}};
        uint32_t index;
        for (index = 0U; index < 10U; index++) {
            generic_pressure[index].tx_id = 0x180U + index;
            generic_pressure[index].dlc = 8U;
        }
        assert(generic_pressure[9].tx_id == 0x189U);
        assert_zero_motor_command(&slot);
    }
}

static void test_inhibit_race_rechecked_before_send(void)
{
    can_tx_safety_state_t state;
    can_tx_message_t slot;
    const can_tx_message_t positive = make_positive_command(500U);

    can_tx_safety_initialize(&state, &slot);
    can_tx_safety_request_inhibit(&state, 0U);
    assert(can_tx_safety_select_motor_command(&state, &positive, &slot) == 1U);
    assert(can_tx_safety_motor_command_positive(&slot) == 1U);

    can_tx_safety_latch_failure(&state, CAN_TX_FAILURE_MAILBOX_TIMEOUT);
    assert(can_tx_safety_select_motor_command(&state, &slot, &slot) == 1U);
    assert_zero_motor_command(&slot);
}

static void test_failure_accounting_and_latching(void)
{
    can_tx_safety_state_t state;
    can_tx_message_t selected;
    const can_tx_message_t positive = make_positive_command(900U);

    can_tx_safety_initialize(&state, &selected);
    can_tx_safety_latch_failure(&state, CAN_TX_FAILURE_QUEUE);
    can_tx_safety_latch_failure(&state, CAN_TX_FAILURE_INVALID_DLC);
    can_tx_safety_latch_failure(&state, CAN_TX_FAILURE_MAILBOX_TIMEOUT);
    can_tx_safety_latch_failure(&state, CAN_TX_FAILURE_HAL_TRANSMIT);
    can_tx_safety_latch_failure(&state, CAN_TX_FAILURE_REQUEUE);
    can_tx_safety_latch_failure(&state, CAN_TX_FAILURE_BUS_OFF);
    can_tx_safety_latch_failure(&state,
        CAN_TX_FAILURE_GENERIC_MOTOR_COMMAND);

    assert(state.queue_failures == 1U);
    assert(state.invalid_dlc_failures == 1U);
    assert(state.mailbox_timeout_failures == 1U);
    assert(state.hal_transmit_failures == 1U);
    assert(state.requeue_failures == 1U);
    assert(state.bus_off_failures == 1U);
    assert(state.generic_motor_command_failures == 1U);
    assert(state.fault_latched == 1U);

    can_tx_safety_request_inhibit(&state, 0U);
    assert(can_tx_safety_positive_allowed(&state) == 0U);
    assert(can_tx_safety_select_motor_command(&state, &positive,
        &selected) == 1U);
    assert_zero_motor_command(&selected);
}

static void test_invalid_dlc_and_controlled_reinitialization(void)
{
    can_tx_safety_state_t state;
    can_tx_message_t stale_slot = make_positive_command(1000U);
    can_tx_message_t invalid = stale_slot;

    invalid.dlc = 9U;
    assert(can_tx_safety_frame_valid(&invalid) == 0U);
    can_tx_safety_initialize(&state, &stale_slot);
    assert_zero_motor_command(&stale_slot);
    assert(state.inhibit_requested == 1U);

    assert(can_tx_safety_select_motor_command(&state, &invalid,
        &stale_slot) == 0U);
    assert(state.invalid_dlc_failures == 1U);
    assert(state.fault_latched == 1U);
    assert_zero_motor_command(&stale_slot);
}

int main(void)
{
    test_positive_overwritten_by_dominant_zero();
    test_inhibit_race_rechecked_before_send();
    test_failure_accounting_and_latching();
    test_invalid_dlc_and_controlled_reinitialization();
    puts("CAN TX safety tests passed");
    return 0;
}
