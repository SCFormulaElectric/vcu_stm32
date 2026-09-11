#ifndef CAN_TX_SAFETY_H
#define CAN_TX_SAFETY_H

#include <stdint.h>
#include "Peripherals/can_protocol.h"

typedef struct {
    uint32_t tx_id;
    uint8_t dlc;
    uint8_t tx_packet[8];
} can_tx_message_t;

typedef enum {
    CAN_TX_FAILURE_QUEUE = 0,
    CAN_TX_FAILURE_INVALID_DLC,
    CAN_TX_FAILURE_MAILBOX_TIMEOUT,
    CAN_TX_FAILURE_HAL_TRANSMIT,
    CAN_TX_FAILURE_REQUEUE,
    CAN_TX_FAILURE_BUS_OFF,
    CAN_TX_FAILURE_GENERIC_MOTOR_COMMAND
} can_tx_failure_t;

typedef struct {
    volatile uint32_t queue_failures;
    volatile uint32_t invalid_dlc_failures;
    volatile uint32_t mailbox_timeout_failures;
    volatile uint32_t hal_transmit_failures;
    volatile uint32_t requeue_failures;
    volatile uint32_t bus_off_failures;
    volatile uint32_t generic_motor_command_failures;
    volatile uint8_t fault_latched;
    volatile uint8_t inhibit_requested;
} can_tx_safety_state_t;

void can_tx_safety_initialize(can_tx_safety_state_t *state,
    can_tx_message_t *initial_motor_command);
void can_tx_safety_request_inhibit(can_tx_safety_state_t *state,
    uint8_t inhibit_requested);
void can_tx_safety_latch_failure(can_tx_safety_state_t *state,
    can_tx_failure_t failure);
uint8_t can_tx_safety_positive_allowed(const can_tx_safety_state_t *state);
uint8_t can_tx_safety_frame_valid(const can_tx_message_t *message);
uint8_t can_tx_safety_motor_command_positive(const can_tx_message_t *message);
void can_tx_safety_make_zero_motor_command(can_tx_message_t *message);
uint8_t can_tx_safety_select_motor_command(can_tx_safety_state_t *state,
    const can_tx_message_t *requested, can_tx_message_t *selected);

#endif /* CAN_TX_SAFETY_H */
