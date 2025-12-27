#ifndef USB_H
#define USB_H
#include "stm32f4xx_hal.h"

void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd);
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd);

typedef enum {
    USB_SD_CARD,
    MCU_SD_CARD
} sd_card_owner_t;

extern sd_card_owner_t sd_card_owner;
#endif