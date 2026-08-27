#include "Tasks/DAQ/Logging/sd_card_task.h"

// Task: SD Card
void sd_card_task(void *argument)
{
    app_data_t *data = (app_data_t *)argument;
    char q_buffer[LOG_MSG_MAX_LEN];
    UINT bw;
    TickType_t last_sync = 0;
    static char log_file_created_this_boot = 0;
    char mcu_mounted = 0;

    for (;;)
    {
        TickType_t start = xTaskGetTickCount();

        uint32_t events = 0;
        xTaskNotifyWait(BITS_CLEARED_BEFORE_READ, BITS_CLEARED_AFTER_READ, &events, 0);
        
        // Interrupts from USB connecting or disconnecting
        // find the interrupt stuff in usb_conf.c
        if (events & USB_CONNECTED)
        {
            if (data->sd_card.file_opened)
            {
                f_close(&data->sd_card.file);
                data->sd_card.file_opened = 0;
            }
            if (mcu_mounted) {
                f_mount(NULL, "", 1);
            }
            sd_card_owner = USB_SD_CARD;
            mcu_mounted = 0;
        }
        if (events & USB_DISCONNECTED)
        {
            FRESULT res = f_mount(&data->sd_card.file_system, "", 1);
            configASSERT(res == FR_OK);
            sd_card_owner = MCU_SD_CARD;
            mcu_mounted = 1;
        }

        // Regular process stuff
        if (sd_card_owner == MCU_SD_CARD)
        {
            if (!mcu_mounted)
            {
                FRESULT res = f_mount(&data->sd_card.file_system, "", 1);
                configASSERT(res == FR_OK);
                mcu_mounted = 1;
            }

            if (!log_file_created_this_boot)
            {
                data->sd_card.log_number = find_next_log_index();
                log_file_created_this_boot = 1;
            }

            if (!data->sd_card.file_opened)
            {
                char filename[32];
                snprintf(filename, sizeof(filename), "log_%lu.txt", data->sd_card.log_number);
                FRESULT res = f_open(&data->sd_card.file, filename, FA_WRITE | FA_OPEN_APPEND);
                configASSERT(res == FR_OK);
                data->sd_card.file_opened = 1;
            }

            while (xQueueReceive(data->sd_card.sd_card_q, q_buffer, 0) == pdTRUE)
            {
                UINT len = strnlen(q_buffer, LOG_MSG_MAX_LEN);
                FRESULT res = f_write(&data->sd_card.file, q_buffer, len, &bw);
                if (res != FR_OK || bw != len)
                    break;
            }

            if ((xTaskGetTickCount() - last_sync) > pdMS_TO_TICKS(SD_CARD_SYNC_PERIOD_MS))
            {
                f_sync(&data->sd_card.file);
                last_sync = xTaskGetTickCount();
            }
        }
        xEventGroupSetBits(data->idwg_group, WD_SD_CARD);
        vTaskDelayUntil(&start, pdMS_TO_TICKS(SD_CARD_DELAY_MS));
    }
}




uint32_t find_next_log_index(void) {
    FILINFO fno;
    char filename[32];
    uint32_t index = 1;
    while (1)
    {
        snprintf(filename, sizeof(filename), "log_%lu.txt", index);
        if (f_stat(filename, &fno) != FR_OK)
        {
            return index;
        }
        index++;
    }
}

task_entry_t create_sd_card_task(app_data_t *data) {
    task_entry_t entry = {0};
    BaseType_t status = xTaskCreate(
        sd_card_task,            
        "SD Card",
        SD_CARD_STACK_SIZE,
        data,
        SD_CARD_PRIO,
        &entry.handle
    );
    
    configASSERT(status == pdPASS);
    vTaskSuspend(entry.handle);
    entry.name = "sd_card";
    return entry;
}
