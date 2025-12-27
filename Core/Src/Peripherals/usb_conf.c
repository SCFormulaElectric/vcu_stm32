#include "Peripherals/usb_conf.h"

sd_card_owner_t sd_card_owner;
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
{
    (void)hpcd;
    sd_card_owner = USB_SD_CARD;
}

void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
{
    (void)hpcd;
    sd_card_owner = MCU_SD_CARD;
}