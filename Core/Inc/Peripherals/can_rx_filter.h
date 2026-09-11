#ifndef CAN_RX_FILTER_H
#define CAN_RX_FILTER_H

#include "stm32f4xx_hal.h"

HAL_StatusTypeDef can_rx_configure_filters(CAN_HandleTypeDef *hcan);

#endif /* CAN_RX_FILTER_H */
