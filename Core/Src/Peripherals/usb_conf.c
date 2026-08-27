#include "Peripherals/usb_conf.h"
#include "app.h"
#include "Tasks/Task_Helper/handles.h"
volatile sd_card_owner_t sd_card_owner;

__weak uint8_t CDC_Transmit_FS(uint8_t *buffer, uint16_t length)
{
    (void)buffer;
    (void)length;
    return 1U;
}

/* Call this from CDC_Receive_FS. Input is deliberately bounded by the queue;
 * overflowing bytes are dropped rather than blocking an interrupt. */
void VCU_USB_CDC_Receive(uint8_t *buffer, uint32_t length)
{
    if (buffer == NULL || app.cli_queue == NULL) {
        return;
    }
    BaseType_t higher_priority_task_woken = pdFALSE;
    for (uint32_t i = 0; i < length; i++) {
        (void)xQueueSendFromISR(app.cli_queue, &buffer[i], &higher_priority_task_woken);
    }
    portYIELD_FROM_ISR(higher_priority_task_woken);
}
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (app.task_entries[sd_card_task_index].handle != NULL) {
        xTaskNotifyFromISR(app.task_entries[sd_card_task_index].handle, USB_CONNECTED, eSetBits, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    (void)hpcd;
}

void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (app.task_entries[sd_card_task_index].handle != NULL) {
        xTaskNotifyFromISR(app.task_entries[sd_card_task_index].handle, USB_DISCONNECTED, eSetBits, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    (void)hpcd;
}
