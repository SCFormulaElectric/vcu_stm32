#include "Peripherals/bms_client.h"

#include <stddef.h>
#include <string.h>

static uint16_t get_u16(const uint8_t *source)
{
    return (uint16_t)((uint16_t)source[0] | ((uint16_t)source[1] << 8U));
}

static uint32_t get_u32(const uint8_t *source)
{
    return (uint32_t)source[0] | ((uint32_t)source[1] << 8U) |
        ((uint32_t)source[2] << 16U) | ((uint32_t)source[3] << 24U);
}

static void put_u32(uint8_t *destination, uint32_t value)
{
    destination[0] = (uint8_t)value;
    destination[1] = (uint8_t)(value >> 8U);
    destination[2] = (uint8_t)(value >> 16U);
    destination[3] = (uint8_t)(value >> 24U);
}

uint8_t bms_client_signal_width(uint8_t signal)
{
    switch (signal) {
        case BMS_CAN_SIGNAL_FORMAT_VERSION:
        case BMS_CAN_SIGNAL_STATE:
        case BMS_CAN_SIGNAL_OUTPUT_REQUESTS:
            return 1U;
        case BMS_CAN_SIGNAL_SOC_PERMILLE:
        case BMS_CAN_SIGNAL_PACK_VOLTAGE_100MV:
        case BMS_CAN_SIGNAL_PACK_CURRENT_100MA:
        case BMS_CAN_SIGNAL_MIN_CELL_MV:
        case BMS_CAN_SIGNAL_MAX_CELL_MV:
        case BMS_CAN_SIGNAL_MAX_TEMPERATURE_DC:
        case BMS_CAN_SIGNAL_ACTIVE_FAULTS_LOW16:
        case BMS_CAN_SIGNAL_VALID_CELL_COUNT:
        case BMS_CAN_SIGNAL_VALID_TEMPERATURE_COUNT:
            return 2U;
        case BMS_CAN_SIGNAL_ACTIVE_FAULTS:
        case BMS_CAN_SIGNAL_LATCHED_FAULTS:
            return 4U;
        default:
            return 0U;
    }
}

const char *bms_client_signal_name(uint8_t signal)
{
    switch (signal) {
        case BMS_CAN_SIGNAL_FORMAT_VERSION: return "format";
        case BMS_CAN_SIGNAL_STATE: return "state";
        case BMS_CAN_SIGNAL_SOC_PERMILLE: return "soc";
        case BMS_CAN_SIGNAL_PACK_VOLTAGE_100MV: return "pack_voltage";
        case BMS_CAN_SIGNAL_PACK_CURRENT_100MA: return "pack_current";
        case BMS_CAN_SIGNAL_MIN_CELL_MV: return "min_cell";
        case BMS_CAN_SIGNAL_MAX_CELL_MV: return "max_cell";
        case BMS_CAN_SIGNAL_MAX_TEMPERATURE_DC: return "max_temperature";
        case BMS_CAN_SIGNAL_ACTIVE_FAULTS_LOW16: return "faults_low16";
        case BMS_CAN_SIGNAL_ACTIVE_FAULTS: return "active_faults";
        case BMS_CAN_SIGNAL_LATCHED_FAULTS: return "latched_faults";
        case BMS_CAN_SIGNAL_OUTPUT_REQUESTS: return "outputs";
        case BMS_CAN_SIGNAL_VALID_CELL_COUNT: return "cell_count";
        case BMS_CAN_SIGNAL_VALID_TEMPERATURE_COUNT: return "temperature_count";
        default: return "none";
    }
}

uint8_t bms_client_signal_from_name(const char *name, uint8_t *signal)
{
    uint8_t candidate;

    if (name == NULL || signal == NULL) {
        return 0U;
    }
    for (candidate = 1U; candidate <= BMS_CAN_SIGNAL_MAX; candidate++) {
        if (strcmp(name, bms_client_signal_name(candidate)) == 0) {
            *signal = candidate;
            return 1U;
        }
    }
    return 0U;
}

uint32_t bms_client_pack_descriptor(const uint8_t signals[8])
{
    uint32_t descriptor = 0U;
    uint32_t index;

    if (signals == NULL) {
        return 0U;
    }
    for (index = 0U; index < BMS_CAN_SUMMARY_MAX_SIGNALS; index++) {
        descriptor |= ((uint32_t)signals[index] & 0x0FU) << (index * 4U);
    }
    return descriptor;
}

void bms_client_unpack_descriptor(uint32_t descriptor, uint8_t signals[8])
{
    uint32_t index;

    if (signals == NULL) {
        return;
    }
    for (index = 0U; index < BMS_CAN_SUMMARY_MAX_SIGNALS; index++) {
        signals[index] = (uint8_t)((descriptor >> (index * 4U)) & 0x0FU);
    }
}

uint8_t bms_client_layout_is_valid(const bms_client_layout_t *layout)
{
    uint32_t slot_index;

    if (layout == NULL || layout->period_ms != BMS_CAN_SUMMARY_PERIOD_MS ||
        layout->active_count > BMS_CAN_SUMMARY_MAX_FRAMES) {
        return 0U;
    }
    for (slot_index = 0U; slot_index < layout->active_count; slot_index++) {
        uint32_t signal_index;
        uint32_t bytes = 0U;
        uint8_t terminated = 0U;

        for (signal_index = 0U;
            signal_index < BMS_CAN_SUMMARY_MAX_SIGNALS; signal_index++) {
            const uint8_t signal = layout->slots[slot_index].signals[signal_index];
            const uint8_t width = bms_client_signal_width(signal);
            if (signal == BMS_CAN_SIGNAL_NONE) {
                terminated = 1U;
                continue;
            }
            if (terminated != 0U || width == 0U || bytes + width > 8U) {
                return 0U;
            }
            bytes += width;
        }
        if (bytes == 0U) {
            return 0U;
        }
    }
    return 1U;
}

void bms_client_initialize(bms_client_t *client)
{
    if (client == NULL) {
        return;
    }
    memset(client, 0, sizeof(*client));
    client->layout.period_ms = BMS_CAN_SUMMARY_PERIOD_MS;
    client->layout.active_count = 2U;
    client->layout.generation = 1U;
    client->layout.slots[0].signals[0] = BMS_CAN_SIGNAL_FORMAT_VERSION;
    client->layout.slots[0].signals[1] = BMS_CAN_SIGNAL_STATE;
    client->layout.slots[0].signals[2] = BMS_CAN_SIGNAL_SOC_PERMILLE;
    client->layout.slots[0].signals[3] = BMS_CAN_SIGNAL_PACK_VOLTAGE_100MV;
    client->layout.slots[0].signals[4] = BMS_CAN_SIGNAL_PACK_CURRENT_100MA;
    client->layout.slots[1].signals[0] = BMS_CAN_SIGNAL_MIN_CELL_MV;
    client->layout.slots[1].signals[1] = BMS_CAN_SIGNAL_MAX_CELL_MV;
    client->layout.slots[1].signals[2] = BMS_CAN_SIGNAL_MAX_TEMPERATURE_DC;
    client->layout.slots[1].signals[3] = BMS_CAN_SIGNAL_ACTIVE_FAULTS_LOW16;
    client->layout_valid = 1U;
    client->next_transaction = 1U;
}

uint8_t bms_client_allocate_transaction(bms_client_t *client)
{
    uint8_t transaction;

    if (client == NULL) {
        return 0U;
    }
    transaction = client->next_transaction++;
    if (transaction == 0U) {
        transaction = client->next_transaction++;
    }
    return transaction;
}

void bms_client_build_command(uint8_t opcode, uint8_t transaction_id,
    uint8_t argument, uint32_t descriptor, uint8_t data[8])
{
    memset(data, 0, 8U);
    data[0] = BMS_CAN_PROTOCOL_VERSION;
    data[1] = opcode;
    data[2] = transaction_id;
    data[3] = argument;
    put_u32(&data[4], descriptor);
}

static void decode_signal(bms_client_t *client, uint8_t signal,
    const uint8_t *source)
{
    switch (signal) {
        case BMS_CAN_SIGNAL_FORMAT_VERSION:
            client->values.format_version = source[0];
            break;
        case BMS_CAN_SIGNAL_STATE:
            client->values.state = source[0];
            break;
        case BMS_CAN_SIGNAL_SOC_PERMILLE:
            client->values.soc_permille = get_u16(source);
            break;
        case BMS_CAN_SIGNAL_PACK_VOLTAGE_100MV:
            client->values.pack_voltage_mv = (int32_t)get_u16(source) * 100;
            break;
        case BMS_CAN_SIGNAL_PACK_CURRENT_100MA:
            client->values.pack_current_ma = (int32_t)(int16_t)get_u16(source) * 100;
            break;
        case BMS_CAN_SIGNAL_MIN_CELL_MV:
            client->values.minimum_cell_mv = get_u16(source);
            break;
        case BMS_CAN_SIGNAL_MAX_CELL_MV:
            client->values.maximum_cell_mv = get_u16(source);
            break;
        case BMS_CAN_SIGNAL_MAX_TEMPERATURE_DC:
            client->values.maximum_temperature_dc = (int16_t)get_u16(source);
            break;
        case BMS_CAN_SIGNAL_ACTIVE_FAULTS_LOW16:
            client->values.active_faults = (client->values.active_faults &
                0xFFFF0000UL) | get_u16(source);
            break;
        case BMS_CAN_SIGNAL_ACTIVE_FAULTS:
            client->values.active_faults = get_u32(source);
            break;
        case BMS_CAN_SIGNAL_LATCHED_FAULTS:
            client->values.latched_faults = get_u32(source);
            break;
        case BMS_CAN_SIGNAL_OUTPUT_REQUESTS:
            client->values.output_requests = source[0];
            break;
        case BMS_CAN_SIGNAL_VALID_CELL_COUNT:
            client->values.valid_cell_count = get_u16(source);
            break;
        case BMS_CAN_SIGNAL_VALID_TEMPERATURE_COUNT:
            client->values.valid_temperature_count = get_u16(source);
            break;
        default:
            return;
    }
    client->values.valid_signals |= (1UL << signal);
}

static void decode_summary(bms_client_t *client, uint8_t slot,
    const uint8_t data[8], uint32_t now_ms)
{
    uint32_t signal_index;
    uint32_t offset = 0U;

    if (client->layout_valid == 0U || slot >= client->layout.active_count) {
        return;
    }
    for (signal_index = 0U;
        signal_index < BMS_CAN_SUMMARY_MAX_SIGNALS; signal_index++) {
        const uint8_t signal = client->layout.slots[slot].signals[signal_index];
        const uint8_t width = bms_client_signal_width(signal);
        if (signal == BMS_CAN_SIGNAL_NONE) {
            break;
        }
        if (width == 0U || offset + width > 8U) {
            client->malformed_frames++;
            return;
        }
        decode_signal(client, signal, &data[offset]);
        offset += width;
    }
    client->last_summary_ms[slot] = now_ms;
    client->summary_seen_mask |= (uint16_t)(1UL << slot);
}

static void decode_fixed_telemetry(bms_client_t *client, uint32_t id,
    const uint8_t data[8])
{
    switch (id - BMS_CAN_BASE_ID) {
        case 0x00U:
            client->values.state = data[0];
            client->values.soc_permille = get_u16(&data[2]);
            client->values.valid_signals |= (1UL << BMS_CAN_SIGNAL_STATE) |
                (1UL << BMS_CAN_SIGNAL_SOC_PERMILLE);
            break;
        case 0x01U:
            client->values.pack_voltage_mv = (int32_t)get_u32(&data[0]);
            client->values.pack_current_ma = (int32_t)get_u32(&data[4]);
            client->values.valid_signals |=
                (1UL << BMS_CAN_SIGNAL_PACK_VOLTAGE_100MV) |
                (1UL << BMS_CAN_SIGNAL_PACK_CURRENT_100MA);
            break;
        case 0x02U:
            client->values.minimum_cell_mv = get_u16(&data[0]);
            client->values.maximum_cell_mv = get_u16(&data[2]);
            client->values.soc_permille = get_u16(&data[6]);
            client->values.valid_signals |=
                (1UL << BMS_CAN_SIGNAL_MIN_CELL_MV) |
                (1UL << BMS_CAN_SIGNAL_MAX_CELL_MV) |
                (1UL << BMS_CAN_SIGNAL_SOC_PERMILLE);
            break;
        case 0x03U:
            client->values.maximum_temperature_dc = (int16_t)get_u16(&data[2]);
            client->values.valid_temperature_count = get_u16(&data[4]);
            client->values.valid_cell_count = get_u16(&data[6]);
            client->values.valid_signals |=
                (1UL << BMS_CAN_SIGNAL_MAX_TEMPERATURE_DC) |
                (1UL << BMS_CAN_SIGNAL_VALID_TEMPERATURE_COUNT) |
                (1UL << BMS_CAN_SIGNAL_VALID_CELL_COUNT);
            break;
        case 0x04U:
            client->values.active_faults = get_u32(&data[0]);
            client->values.latched_faults = get_u32(&data[4]);
            client->values.valid_signals |=
                (1UL << BMS_CAN_SIGNAL_ACTIVE_FAULTS) |
                (1UL << BMS_CAN_SIGNAL_LATCHED_FAULTS);
            break;
        case 0x05U:
            client->values.output_requests = data[0];
            client->values.valid_signals |=
                (1UL << BMS_CAN_SIGNAL_OUTPUT_REQUESTS);
            break;
        default:
            break;
    }
}

uint8_t bms_client_process_frame(bms_client_t *client, uint32_t id,
    uint8_t dlc, const uint8_t data[8], uint32_t now_ms,
    uint8_t follow_up[8])
{
    if (client == NULL || data == NULL || follow_up == NULL ||
        dlc != BMS_CAN_FRAME_DLC) {
        if (client != NULL) {
            client->malformed_frames++;
        }
        return 0U;
    }
    client->last_rx_ms = now_ms;
    if (id == BMS_CAN_CONFIG_RESPONSE_ID) {
        if (data[0] != BMS_CAN_PROTOCOL_VERSION) {
            client->malformed_frames++;
            return 0U;
        }
        client->last_opcode = data[1];
        client->last_transaction = data[2];
        client->last_status = data[3];
        client->ack_count++;
        if (data[1] == BMS_CAN_CMD_BEGIN && data[3] == BMS_CAN_STATUS_OK) {
            client->transaction_open = 1U;
            client->transaction_id = data[2];
        } else if ((data[1] == BMS_CAN_CMD_COMMIT ||
            data[1] == BMS_CAN_CMD_ABORT) && data[3] == BMS_CAN_STATUS_OK) {
            client->transaction_open = 0U;
        }
        if ((data[1] == BMS_CAN_CMD_READ_CONFIG ||
            data[1] == BMS_CAN_CMD_COMMIT) && data[3] == BMS_CAN_STATUS_OK) {
            if (data[5] > BMS_CAN_SUMMARY_MAX_FRAMES ||
                get_u16(&data[6]) * 100U != BMS_CAN_SUMMARY_PERIOD_MS) {
                client->malformed_frames++;
                return 0U;
            }
            client->layout.generation = data[4];
            client->layout.active_count = data[5];
            client->layout.period_ms = get_u16(&data[6]) * 100U;
            memset(client->layout.slots, 0, sizeof(client->layout.slots));
            client->layout_valid = (client->layout.active_count == 0U) ? 1U : 0U;
            client->readback_in_progress =
                (client->layout.active_count != 0U) ? 1U : 0U;
            client->readback_next_slot = 0U;
            client->readback_transaction = data[2];
            if (client->readback_in_progress != 0U) {
                bms_client_build_command(BMS_CAN_CMD_READ_SLOT, data[2],
                    0U, 0U, follow_up);
                return 1U;
            }
        }
        return 0U;
    }
    if (BMS_CAN_ID_IS_LAYOUT_RESPONSE(id)) {
        const uint8_t slot = (uint8_t)(id - BMS_CAN_LAYOUT_RESPONSE_BASE_ID);
        if (data[0] != BMS_CAN_PROTOCOL_VERSION ||
            client->readback_in_progress == 0U ||
            data[1] != client->readback_transaction || data[2] !=
                client->layout.generation || data[3] != slot ||
            slot != client->readback_next_slot) {
            client->malformed_frames++;
            return 0U;
        }
        bms_client_unpack_descriptor(get_u32(&data[4]),
            client->layout.slots[slot].signals);
        client->readback_next_slot++;
        if (client->readback_next_slot < client->layout.active_count) {
            bms_client_build_command(BMS_CAN_CMD_READ_SLOT,
                client->readback_transaction, client->readback_next_slot,
                0U, follow_up);
            return 1U;
        }
        client->readback_in_progress = 0U;
        client->layout_valid = bms_client_layout_is_valid(&client->layout);
        return 0U;
    }
    if (BMS_CAN_ID_IS_SUMMARY(id)) {
        decode_summary(client, (uint8_t)(id - BMS_CAN_SUMMARY_BASE_ID),
            data, now_ms);
    } else if (id >= BMS_CAN_BASE_ID && id <
        BMS_CAN_LAYOUT_RESPONSE_BASE_ID) {
        decode_fixed_telemetry(client, id, data);
    }
    return 0U;
}
