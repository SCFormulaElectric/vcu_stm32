#include "Tasks/DAQ/Logging/sd_card_task.h"

static storage_result_t classify_mount_result(FRESULT result)
{
    if (result == FR_NOT_READY || result == FR_NO_PATH) {
        return storage_policy_classify_result(STORAGE_OPERATION_MOUNT,
            STORAGE_IO_MEDIA_ABSENT);
    }
    if (result == FR_NO_FILESYSTEM) {
        return storage_policy_classify_result(STORAGE_OPERATION_MOUNT,
            STORAGE_IO_CORRUPT);
    }
    return storage_policy_classify_result(STORAGE_OPERATION_MOUNT,
        STORAGE_IO_ERROR);
}

static void discard_log_queue(sd_card_t *card)
{
    if (card->sd_card_q != NULL) {
        card->policy.dropped_records +=
            (uint32_t)uxQueueMessagesWaiting(card->sd_card_q);
        (void)xQueueReset(card->sd_card_q);
    }
}

static void enter_failed_state(sd_card_t *card, storage_result_t result)
{
    storage_policy_mark_failed(&card->policy, result);
    if (card->file_opened != 0U) {
        const FRESULT close_result = f_close(&card->file);
        card->file_opened = 0U;
        if (close_result != FR_OK) {
            card->policy.last_result = STORAGE_RESULT_CLOSE_ERROR;
            card->policy.error_count++;
        }
    }
    if (card->mounted != 0U) {
        const FRESULT unmount_result = f_mount(NULL, "", 1U);
        card->mounted = 0U;
        if (unmount_result != FR_OK) {
            card->policy.last_result = STORAGE_RESULT_UNMOUNT_ERROR;
            card->policy.error_count++;
        }
    }
    discard_log_queue(card);
}

static uint8_t close_and_unmount(sd_card_t *card)
{
    if (card->file_opened != 0U) {
        if (f_close(&card->file) != FR_OK) {
            card->file_opened = 0U;
            enter_failed_state(card, storage_policy_classify_result(
                STORAGE_OPERATION_CLOSE, STORAGE_IO_ERROR));
            return 0U;
        }
        card->file_opened = 0U;
    }
    if (card->mounted != 0U) {
        if (f_mount(NULL, "", 1U) != FR_OK) {
            card->mounted = 0U;
            enter_failed_state(card, storage_policy_classify_result(
                STORAGE_OPERATION_UNMOUNT, STORAGE_IO_ERROR));
            return 0U;
        }
        card->mounted = 0U;
    }
    return 1U;
}

FRESULT find_next_log_index(uint32_t *cursor, uint32_t *index_out,
    uint8_t *found)
{
    FILINFO file_info;
    char filename[32];
    uint32_t index;
    uint32_t checks;

    if (cursor == NULL || index_out == NULL || found == NULL) {
        return FR_INVALID_PARAMETER;
    }
    *found = 0U;
    index = (*cursor >= 1U) ? *cursor : 1U;
    for (checks = 0U; checks < SD_CARD_NAME_CHECKS_PER_ACTIVATION;
        checks++, index++) {
        if (storage_policy_name_index_valid(index,
            SD_CARD_MAX_LOG_INDEX) == 0U) {
            return FR_EXIST;
        }
        const int written = snprintf(filename, sizeof(filename),
            "log_%lu.txt", (unsigned long)index);
        if (written < 0 || (size_t)written >= sizeof(filename)) {
            return FR_INVALID_NAME;
        }
        {
            const FRESULT result = f_stat(filename, &file_info);
            if (result == FR_NO_FILE) {
                *index_out = index;
                *cursor = index;
                *found = 1U;
                return FR_OK;
            }
            if (result != FR_OK) {
                return result;
            }
        }
    }
    *cursor = index;
    return FR_OK;
}

static uint8_t prepare_storage(sd_card_t *card)
{
    FRESULT result;
    char filename[32];
    int written;
    uint8_t name_found = 0U;

    if (card->mounted == 0U) {
        result = f_mount(&card->file_system, "", 1U);
        if (result != FR_OK) {
            enter_failed_state(card, classify_mount_result(result));
            return 0U;
        }
        card->mounted = 1U;
    }
    result = find_next_log_index(&card->name_search_index,
        &card->log_number, &name_found);
    if (result != FR_OK) {
        enter_failed_state(card, (result == FR_EXIST) ?
            STORAGE_RESULT_NAME_EXHAUSTED : STORAGE_RESULT_IO_ERROR);
        return 0U;
    }
    if (name_found == 0U) {
        return 0U;
    }
    written = snprintf(filename, sizeof(filename), "log_%lu.txt",
        (unsigned long)card->log_number);
    if (written < 0 || (size_t)written >= sizeof(filename)) {
        enter_failed_state(card, STORAGE_RESULT_NAME_EXHAUSTED);
        return 0U;
    }
    result = f_open(&card->file, filename, FA_WRITE | FA_OPEN_APPEND);
    if (result != FR_OK) {
        enter_failed_state(card, STORAGE_RESULT_IO_ERROR);
        return 0U;
    }
    card->file_opened = 1U;
    storage_policy_mark_ready(&card->policy);
    return 1U;
}

void sd_card_task(void *argument)
{
    app_data_t *data = (app_data_t *)argument;
    char queue_buffer[LOG_MSG_MAX_LEN];
    TickType_t last_sync = 0U;

    for (;;) {
        TickType_t start = xTaskGetTickCount();
        uint32_t events = 0U;

        (void)xTaskNotifyWait(BITS_CLEARED_BEFORE_READ,
            BITS_CLEARED_AFTER_READ, &events, 0U);
#if VCU_USB_MSC_SD_ENABLED
        if ((events & USB_CONNECTED) != 0U &&
            storage_policy_usb_ownership_allowed(1U) != 0U &&
            close_and_unmount(&data->sd_card) != 0U) {
            sd_card_owner = USB_SD_CARD;
            data->sd_card.policy.state = STORAGE_MEDIA_USB_OWNED;
            discard_log_queue(&data->sd_card);
        }
        if ((events & USB_DISCONNECTED) != 0U &&
            sd_card_owner == USB_SD_CARD) {
            sd_card_owner = MCU_SD_CARD;
            data->sd_card.policy.state = STORAGE_MEDIA_UNMOUNTED;
        }
#else
        (void)events;
#endif

        if (data->log_level != LOG_SD_CARD) {
            if (close_and_unmount(&data->sd_card) != 0U) {
                data->sd_card.policy.state = STORAGE_MEDIA_DISABLED;
            }
            discard_log_queue(&data->sd_card);
        } else if (sd_card_owner == MCU_SD_CARD) {
            if (data->sd_card.sd_card_q == NULL) {
                if (data->sd_card.policy.state != STORAGE_MEDIA_FAILED) {
                    storage_policy_mark_failed(&data->sd_card.policy,
                        STORAGE_RESULT_IO_ERROR);
                }
            } else if (data->sd_card.policy.state == STORAGE_MEDIA_DISABLED) {
                storage_policy_initialize(&data->sd_card.policy, 1U);
                data->sd_card.name_search_index = 1U;
            }
            if (data->sd_card.sd_card_q != NULL &&
                data->sd_card.policy.state == STORAGE_MEDIA_FAILED) {
                if (storage_policy_take_recovery(&data->sd_card.policy) != 0U) {
                    data->sd_card.name_search_index = 1U;
                    data->sd_card.log_number = 0U;
                }
            }
            if (data->sd_card.sd_card_q != NULL &&
                data->sd_card.policy.state == STORAGE_MEDIA_UNMOUNTED) {
                (void)prepare_storage(&data->sd_card);
            }
            if (data->sd_card.sd_card_q != NULL &&
                data->sd_card.policy.state == STORAGE_MEDIA_READY) {
                uint32_t remaining = storage_policy_queue_budget(
                    (uint32_t)uxQueueMessagesWaiting(data->sd_card.sd_card_q),
                    SD_CARD_MESSAGES_PER_ACTIVATION);
                while (remaining-- > 0U &&
                    xQueueReceive(data->sd_card.sd_card_q, queue_buffer, 0U) ==
                    pdPASS) {
                    const UINT length = (UINT)strnlen(queue_buffer,
                        LOG_MSG_MAX_LEN);
                    UINT bytes_written = 0U;
                    const FRESULT result = f_write(&data->sd_card.file,
                        queue_buffer, length, &bytes_written);
                    if (result != FR_OK) {
                        enter_failed_state(&data->sd_card,
                            storage_policy_classify_result(
                                STORAGE_OPERATION_WRITE, STORAGE_IO_ERROR));
                        break;
                    }
                    if (bytes_written != length) {
                        enter_failed_state(&data->sd_card,
                            storage_policy_classify_result(
                                STORAGE_OPERATION_WRITE, STORAGE_IO_SHORT));
                        break;
                    }
                    data->sd_card.policy.records_written++;
                }
                if (data->sd_card.policy.state == STORAGE_MEDIA_READY &&
                    (xTaskGetTickCount() - last_sync) >=
                        pdMS_TO_TICKS(SD_CARD_SYNC_PERIOD_MS)) {
                    if (f_sync(&data->sd_card.file) != FR_OK) {
                        enter_failed_state(&data->sd_card,
                            storage_policy_classify_result(
                                STORAGE_OPERATION_SYNC, STORAGE_IO_ERROR));
                    } else {
                        last_sync = xTaskGetTickCount();
                    }
                }
            }
        }
        xEventGroupSetBits(data->idwg_group, WD_SD_CARD);
        vTaskDelayUntil(&start, pdMS_TO_TICKS(SD_CARD_DELAY_MS));
    }
}

task_entry_t create_sd_card_task(app_data_t *data)
{
    task_entry_t entry = {0};
    const BaseType_t status = xTaskCreate(sd_card_task, "SD Card",
        SD_CARD_STACK_SIZE, data, SD_CARD_PRIO, &entry.handle);

    if (status == pdPASS) {
        vTaskSuspend(entry.handle);
    }
    entry.name = "sd_card";
    return entry;
}
