#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "Peripherals/can_rx_policy.h"

static uint8_t filter_matches(const can_rx_filter_words_t *filter,
    uint16_t id, uint16_t low_bits)
{
    const uint16_t id_high = (uint16_t)(id << 5U);
    return (((id_high & filter->mask_high) ==
        (filter->id_high & filter->mask_high)) &&
        ((low_bits & filter->mask_low) ==
        (filter->id_low & filter->mask_low))) ? 1U : 0U;
}

static void test_frame_classes(void)
{
    assert(can_rx_classify_frame(0x0A0U, 8U, 0U, 0U) ==
        CAN_RX_CLASS_USED_INVERTER_STATUS);
    assert(can_rx_classify_frame(0x0BFU, 8U, 0U, 0U) ==
        CAN_RX_CLASS_UNUSED_INVERTER_STATUS);
    assert(can_rx_classify_frame(0x0ABU, 8U, 0U, 0U) ==
        CAN_RX_CLASS_CRITICAL_FAULT_STATUS);
    assert(can_rx_classify_frame(0x0C2U, 6U, 0U, 0U) ==
        CAN_RX_CLASS_PARAMETER_RESPONSE);
    assert(can_rx_classify_frame(0x180U, 0U, 0U, 0U) ==
        CAN_RX_CLASS_DASHBOARD);
    assert(can_rx_classify_frame(0x18FU, 8U, 0U, 0U) ==
        CAN_RX_CLASS_DASHBOARD);
    assert(can_rx_classify_frame(BMS_CAN_SUMMARY_BASE_ID, 8U, 0U, 0U) ==
        CAN_RX_CLASS_BMS_SUMMARY);
    assert(can_rx_classify_frame(BMS_CAN_CONFIG_RESPONSE_ID, 8U, 0U, 0U) ==
        CAN_RX_CLASS_BMS_CONTROL);
    assert(can_rx_classify_frame(BMS_CAN_BASE_ID, 8U, 0U, 0U) ==
        CAN_RX_CLASS_BMS_TELEMETRY);
    assert(can_rx_classify_frame(0x09FU, 8U, 0U, 0U) ==
        CAN_RX_CLASS_REJECTED);
    assert(can_rx_classify_frame(0x0C0U, 8U, 0U, 0U) ==
        CAN_RX_CLASS_REJECTED);
    assert(can_rx_classify_frame(0x190U, 8U, 0U, 0U) ==
        CAN_RX_CLASS_REJECTED);
}

static void test_rejection_and_dlc(void)
{
    assert(can_rx_classify_frame(0x0ABU, 8U, 1U, 0U) ==
        CAN_RX_CLASS_REJECTED);
    assert(can_rx_classify_frame(0x0ABU, 8U, 0U, 1U) ==
        CAN_RX_CLASS_REJECTED);
    assert(can_rx_classify_frame(0x0ABU, 7U, 0U, 0U) ==
        CAN_RX_CLASS_MALFORMED);
    assert(can_rx_classify_frame(0x0C2U, 8U, 0U, 0U) ==
        CAN_RX_CLASS_MALFORMED);
    assert(can_rx_classify_frame(0x180U, 9U, 0U, 0U) ==
        CAN_RX_CLASS_MALFORMED);
    assert(can_rx_classify_frame(BMS_CAN_SUMMARY_BASE_ID, 7U, 0U, 0U) ==
        CAN_RX_CLASS_BMS_MALFORMED);
}

static void test_filter_masks(void)
{
    can_rx_filter_words_t inverter;
    can_rx_filter_words_t parameter;
    can_rx_filter_words_t dashboard;
    can_rx_filter_words_t bms;

    can_rx_make_standard_data_filter(0x0A0U, 0x7E0U, &inverter);
    can_rx_make_standard_data_filter(0x0C2U, 0x7FFU, &parameter);
    can_rx_make_standard_data_filter(0x180U, 0x7F0U, &dashboard);
    can_rx_make_standard_data_filter(BMS_CAN_BASE_ID, 0x700U, &bms);
    assert(inverter.mask_low == CAN_RX_FILTER_IDE_RTR_MASK_LOW);
    assert(filter_matches(&inverter, 0x0A0U, 0U) == 1U);
    assert(filter_matches(&inverter, 0x0BFU, 0U) == 1U);
    assert(filter_matches(&inverter, 0x0C0U, 0U) == 0U);
    assert(filter_matches(&parameter, 0x0C2U, 0U) == 1U);
    assert(filter_matches(&parameter, 0x0C3U, 0U) == 0U);
    assert(filter_matches(&dashboard, 0x180U, 0U) == 1U);
    assert(filter_matches(&dashboard, 0x18FU, 0U) == 1U);
    assert(filter_matches(&dashboard, 0x190U, 0U) == 0U);
    assert(filter_matches(&bms, 0x600U, 0U) == 1U);
    assert(filter_matches(&bms, 0x6FFU, 0U) == 1U);
    assert(filter_matches(&bms, 0x5FFU, 0U) == 0U);
    /* bxCAN low bits: IDE=bit2, RTR=bit1. */
    assert(filter_matches(&inverter, 0x0ABU, 0x0004U) == 0U);
    assert(filter_matches(&inverter, 0x0ABU, 0x0002U) == 0U);
}

static void test_critical_latest_value_routing(void)
{
    uint32_t latest_fault_marker = 0U;

    if (can_rx_classify_frame(0x0ABU, 8U, 0U, 0U) ==
        CAN_RX_CLASS_CRITICAL_FAULT_STATUS) {
        latest_fault_marker = 1U;
    }
    if (can_rx_classify_frame(0x0A0U, 8U, 0U, 0U) ==
        CAN_RX_CLASS_CRITICAL_FAULT_STATUS) {
        latest_fault_marker = 99U;
    }
    if (can_rx_classify_frame(0x0ABU, 8U, 0U, 0U) ==
        CAN_RX_CLASS_CRITICAL_FAULT_STATUS) {
        latest_fault_marker = 2U;
    }
    assert(latest_fault_marker == 2U);
}

int main(void)
{
    test_frame_classes();
    test_rejection_and_dlc();
    test_filter_masks();
    test_critical_latest_value_routing();
    puts("CAN RX policy tests passed");
    return 0;
}
