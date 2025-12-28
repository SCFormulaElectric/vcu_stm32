#include "Peripherals/usb_conf.h"
#include "app.h"
#include "Tasks/Task_Helper/handles.h"
volatile sd_card_owner_t sd_card_owner;
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(app.task_entries[sd_card_task_index], USB_CONNECTED, eSetBits, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    (void)hpcd;
}

void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskNotifyFromISR(app.task_entries[sd_card_task_index], USB_DISCONNECTED, eSetBits, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    (void)hpcd;
}
