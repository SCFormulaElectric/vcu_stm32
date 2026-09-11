#ifndef CAN_RX_POLICY_H
#define CAN_RX_POLICY_H

#include <stdint.h>

#include "Peripherals/can_protocol.h"

#define CAN_RX_FILTER_IDE_RTR_MASK_LOW  0x0006U

typedef enum {
    CAN_RX_CLASS_REJECTED = 0,
    CAN_RX_CLASS_MALFORMED,
    CAN_RX_CLASS_CRITICAL_FAULT_STATUS,
    CAN_RX_CLASS_PARAMETER_RESPONSE,
    CAN_RX_CLASS_USED_INVERTER_STATUS,
    CAN_RX_CLASS_UNUSED_INVERTER_STATUS,
    CAN_RX_CLASS_DASHBOARD,
    CAN_RX_CLASS_BMS_TELEMETRY,
    CAN_RX_CLASS_BMS_SUMMARY,
    CAN_RX_CLASS_BMS_CONTROL,
    CAN_RX_CLASS_BMS_MALFORMED
} can_rx_frame_class_t;

typedef struct {
    uint16_t id_high;
    uint16_t id_low;
    uint16_t mask_high;
    uint16_t mask_low;
} can_rx_filter_words_t;

can_rx_frame_class_t can_rx_classify_frame(uint32_t id, uint8_t dlc,
    uint8_t is_extended, uint8_t is_remote);
void can_rx_make_standard_data_filter(uint16_t id, uint16_t id_mask,
    can_rx_filter_words_t *filter);

#endif /* CAN_RX_POLICY_H */
