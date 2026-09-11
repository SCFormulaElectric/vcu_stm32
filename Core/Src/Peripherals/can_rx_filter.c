#include "Peripherals/can_rx_filter.h"

#include "Peripherals/can_rx_policy.h"

#define CAN1_FILTER_BANK_FIRST       0U
#define CAN2_FILTER_BANK_FIRST       14U

HAL_StatusTypeDef can_rx_configure_filters(CAN_HandleTypeDef *hcan)
{
    static const uint16_t ids[] = {
        CAN_ID_MC_BROADCAST_MIN,
        CAN_ID_MC_PARAMETER_RESPONSE,
        CAN_ID_DASHBOARD_MIN,
        BMS_CAN_BASE_ID
    };
    static const uint16_t masks[] = {0x7E0U, 0x7FFU, 0x7F0U, 0x700U};
    uint32_t index;

    if (hcan == NULL) {
        return HAL_ERROR;
    }
    for (index = 0U; index < 4U; index++) {
        CAN_FilterTypeDef config = {0};
        can_rx_filter_words_t words;

        can_rx_make_standard_data_filter(ids[index], masks[index], &words);
        config.FilterBank = CAN1_FILTER_BANK_FIRST + index;
        config.FilterMode = CAN_FILTERMODE_IDMASK;
        config.FilterScale = CAN_FILTERSCALE_32BIT;
        config.FilterIdHigh = words.id_high;
        config.FilterIdLow = words.id_low;
        config.FilterMaskIdHigh = words.mask_high;
        config.FilterMaskIdLow = words.mask_low;
        config.FilterFIFOAssignment = CAN_FILTER_FIFO0;
        config.FilterActivation = ENABLE;
        /* Banks 0..13 remain owned by CAN1; CAN2 starts at bank 14. */
        config.SlaveStartFilterBank = CAN2_FILTER_BANK_FIRST;
        if (HAL_CAN_ConfigFilter(hcan, &config) != HAL_OK) {
            return HAL_ERROR;
        }
    }
    return HAL_OK;
}
