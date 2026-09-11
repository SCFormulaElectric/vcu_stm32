#include "Peripherals/can_rx_policy.h"

can_rx_frame_class_t can_rx_classify_frame(uint32_t id, uint8_t dlc,
    uint8_t is_extended, uint8_t is_remote)
{
    if (is_extended != 0U || is_remote != 0U || id > 0x7FFU) {
        return CAN_RX_CLASS_REJECTED;
    }
    if (id >= CAN_ID_MC_BROADCAST_MIN && id <= CAN_ID_MC_BROADCAST_MAX) {
        if (dlc != CAN_DLC_MC_STATUS) {
            return CAN_RX_CLASS_MALFORMED;
        }
        if (id == CAN_ID_MC_FAULT_CODES) {
            return CAN_RX_CLASS_CRITICAL_FAULT_STATUS;
        }
        if (id == CAN_ID_MC_TEMPERATURE_1 ||
            id == CAN_ID_MC_TEMPERATURE_2 ||
            id == CAN_ID_MC_TEMPERATURE_3) {
            return CAN_RX_CLASS_USED_INVERTER_STATUS;
        }
        return CAN_RX_CLASS_UNUSED_INVERTER_STATUS;
    }
    if (id == CAN_ID_MC_PARAMETER_RESPONSE) {
        return (dlc == CAN_DLC_MC_PARAMETER_RESPONSE) ?
            CAN_RX_CLASS_PARAMETER_RESPONSE : CAN_RX_CLASS_MALFORMED;
    }
    if (BMS_CAN_ID_IS_TELEMETRY(id)) {
        if (dlc != BMS_CAN_FRAME_DLC) {
            return CAN_RX_CLASS_BMS_MALFORMED;
        }
        if (id == BMS_CAN_CONFIG_RESPONSE_ID ||
            BMS_CAN_ID_IS_LAYOUT_RESPONSE(id)) {
            return CAN_RX_CLASS_BMS_CONTROL;
        }
        if (BMS_CAN_ID_IS_SUMMARY(id)) {
            return CAN_RX_CLASS_BMS_SUMMARY;
        }
        return CAN_RX_CLASS_BMS_TELEMETRY;
    }
    if (CAN_ID_IS_DASHBOARD(id)) {
        /* Dashboard payload lengths are team-defined and no authoritative
         * per-ID contract is present. Enforce the CAN maximum until supplied. */
        return (dlc <= 8U) ? CAN_RX_CLASS_DASHBOARD :
            CAN_RX_CLASS_MALFORMED;
    }
    return CAN_RX_CLASS_REJECTED;
}

void can_rx_make_standard_data_filter(uint16_t id, uint16_t id_mask,
    can_rx_filter_words_t *filter)
{
    if (filter == 0) {
        return;
    }
    filter->id_high = (uint16_t)(id << 5U);
    filter->id_low = 0U;
    filter->mask_high = (uint16_t)(id_mask << 5U);
    /* bxCAN low-half bits 2 and 1 are IDE and RTR respectively. Both are
     * explicitly compared against zero to accept only standard DATA frames. */
    filter->mask_low = CAN_RX_FILTER_IDE_RTR_MASK_LOW;
}
