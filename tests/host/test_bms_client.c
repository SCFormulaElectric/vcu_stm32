#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "Peripherals/bms_client.h"

static void test_default_summary_decode(void)
{
    bms_client_t client;
    uint8_t follow_up[8] = {0};
    const uint8_t primary[8] = {1U, 2U, 0xE8U, 0x03U,
        0xA0U, 0x0FU, 0x85U, 0xFFU};
    const uint8_t secondary[8] = {0x80U, 0x0DU, 0x1BU, 0x10U,
        0x37U, 0x02U, 0x34U, 0x12U};

    bms_client_initialize(&client);
    assert(bms_client_process_frame(&client, BMS_CAN_SUMMARY_BASE_ID, 8U,
        primary, 100U, follow_up) == 0U);
    assert(bms_client_process_frame(&client, BMS_CAN_SUMMARY_BASE_ID + 1U,
        8U, secondary, 101U, follow_up) == 0U);
    assert(client.values.state == 2U);
    assert(client.values.soc_permille == 1000U);
    assert(client.values.pack_voltage_mv == 400000);
    assert(client.values.pack_current_ma == -12300);
    assert(client.values.minimum_cell_mv == 3456U);
    assert(client.values.maximum_cell_mv == 4123U);
    assert(client.values.maximum_temperature_dc == 567);
    assert((client.values.active_faults & 0xFFFFU) == 0x1234U);
    assert(client.summary_seen_mask == 3U);
}

static void test_layout_readback_sequence(void)
{
    bms_client_t client;
    uint8_t follow_up[8] = {0};
    uint8_t response[8] = {BMS_CAN_PROTOCOL_VERSION,
        BMS_CAN_CMD_READ_CONFIG, 7U, BMS_CAN_STATUS_OK, 4U, 2U, 50U, 0U};
    uint8_t layout_response[8] = {BMS_CAN_PROTOCOL_VERSION, 7U, 4U, 0U};
    uint8_t signals[8] = {BMS_CAN_SIGNAL_STATE,
        BMS_CAN_SIGNAL_ACTIVE_FAULTS, 0U};
    uint32_t descriptor = bms_client_pack_descriptor(signals);

    bms_client_initialize(&client);
    assert(bms_client_process_frame(&client, BMS_CAN_CONFIG_RESPONSE_ID, 8U,
        response, 10U, follow_up) == 1U);
    assert(follow_up[1] == BMS_CAN_CMD_READ_SLOT && follow_up[3] == 0U);
    memcpy(&layout_response[4], &descriptor, sizeof(descriptor));
    assert(bms_client_process_frame(&client, BMS_CAN_LAYOUT_RESPONSE_BASE_ID,
        8U, layout_response, 11U, follow_up) == 1U);
    assert(follow_up[3] == 1U);
    layout_response[3] = 1U;
    assert(bms_client_process_frame(&client,
        BMS_CAN_LAYOUT_RESPONSE_BASE_ID + 1U, 8U, layout_response, 12U,
        follow_up) == 0U);
    assert(client.layout_valid != 0U);
}

static void test_layout_validation(void)
{
    bms_client_t client;

    bms_client_initialize(&client);
    assert(bms_client_layout_is_valid(&client.layout) != 0U);
    client.layout.active_count = 17U;
    assert(bms_client_layout_is_valid(&client.layout) == 0U);
}

int main(void)
{
    test_default_summary_decode();
    test_layout_readback_sequence();
    test_layout_validation();
    return 0;
}
