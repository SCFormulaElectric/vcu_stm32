#ifndef USB_H
#define USB_H
#include "stm32f4xx_hal.h"
#include "ff.h"
#include "queue.h"
#define USB_CONNECTED    (1 << 0)
#define USB_DISCONNECTED (1 << 1)

void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd);
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd);

typedef enum {
    USB_SD_CARD,
    MCU_SD_CARD
} sd_card_owner_t;

typedef struct {
    FATFS file_system;
    FIL file;
    volatile char file_opened;
    uint32_t log_number;
    QueueHandle_t sd_card_q;
} sd_card_t;

extern volatile sd_card_owner_t sd_card_owner;

/* Implemented by the generated USB CDC class when enabled. The weak fallback
 * keeps the VCU safe and linkable on builds without the USB middleware. */
uint8_t CDC_Transmit_FS(uint8_t *buffer, uint16_t length);
void VCU_USB_CDC_Receive(uint8_t *buffer, uint32_t length);

#endif
