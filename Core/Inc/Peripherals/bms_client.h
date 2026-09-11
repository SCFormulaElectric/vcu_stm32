#ifndef BMS_CLIENT_H
#define BMS_CLIENT_H

#include <stdint.h>

#include "Peripherals/bms_can_protocol.h"

typedef struct {
    uint8_t signals[BMS_CAN_SUMMARY_MAX_SIGNALS];
} bms_client_slot_t;

typedef struct {
    uint16_t period_ms;
    uint8_t active_count;
    uint8_t generation;
    bms_client_slot_t slots[BMS_CAN_SUMMARY_MAX_FRAMES];
} bms_client_layout_t;

typedef struct {
    uint8_t format_version;
    uint8_t state;
    uint16_t soc_permille;
    int32_t pack_voltage_mv;
    int32_t pack_current_ma;
    uint16_t minimum_cell_mv;
    uint16_t maximum_cell_mv;
    int16_t maximum_temperature_dc;
    uint32_t active_faults;
    uint32_t latched_faults;
    uint8_t output_requests;
    uint16_t valid_cell_count;
    uint16_t valid_temperature_count;
    uint32_t valid_signals;
} bms_client_values_t;

typedef struct {
    bms_client_layout_t layout;
    bms_client_values_t values;
    uint32_t last_rx_ms;
    uint32_t last_summary_ms[BMS_CAN_SUMMARY_MAX_FRAMES];
    uint16_t summary_seen_mask;
    uint8_t layout_valid;
    uint8_t readback_in_progress;
    uint8_t readback_next_slot;
    uint8_t readback_transaction;
    uint8_t next_transaction;
    uint8_t transaction_open;
    uint8_t transaction_id;
    uint8_t last_opcode;
    uint8_t last_status;
    uint8_t last_transaction;
    uint32_t ack_count;
    uint32_t malformed_frames;
} bms_client_t;

void bms_client_initialize(bms_client_t *client);
uint8_t bms_client_signal_width(uint8_t signal);
const char *bms_client_signal_name(uint8_t signal);
uint8_t bms_client_signal_from_name(const char *name, uint8_t *signal);
uint32_t bms_client_pack_descriptor(const uint8_t signals[8]);
void bms_client_unpack_descriptor(uint32_t descriptor, uint8_t signals[8]);
uint8_t bms_client_layout_is_valid(const bms_client_layout_t *layout);
uint8_t bms_client_allocate_transaction(bms_client_t *client);
void bms_client_build_command(uint8_t opcode, uint8_t transaction_id,
    uint8_t argument, uint32_t descriptor, uint8_t data[8]);
/* Returns one when a follow-up READ_SLOT command was produced. */
uint8_t bms_client_process_frame(bms_client_t *client, uint32_t id,
    uint8_t dlc, const uint8_t data[8], uint32_t now_ms,
    uint8_t follow_up[8]);

#endif /* BMS_CLIENT_H */
