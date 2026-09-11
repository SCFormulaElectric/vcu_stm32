#ifndef STORAGE_POLICY_H
#define STORAGE_POLICY_H

#include <stdint.h>

typedef enum {
    STORAGE_MEDIA_DISABLED = 0,
    STORAGE_MEDIA_UNMOUNTED,
    STORAGE_MEDIA_READY,
    STORAGE_MEDIA_FAILED,
    STORAGE_MEDIA_USB_OWNED
} storage_media_state_t;

typedef enum {
    STORAGE_RESULT_NONE = 0,
    STORAGE_RESULT_OK,
    STORAGE_RESULT_MEDIA_ABSENT,
    STORAGE_RESULT_CORRUPT,
    STORAGE_RESULT_IO_ERROR,
    STORAGE_RESULT_NAME_EXHAUSTED,
    STORAGE_RESULT_SHORT_WRITE,
    STORAGE_RESULT_SYNC_ERROR,
    STORAGE_RESULT_CLOSE_ERROR,
    STORAGE_RESULT_UNMOUNT_ERROR,
    STORAGE_RESULT_USB_OWNERSHIP_DISABLED
} storage_result_t;

typedef enum {
    STORAGE_OPERATION_MOUNT = 0,
    STORAGE_OPERATION_OPEN,
    STORAGE_OPERATION_WRITE,
    STORAGE_OPERATION_SYNC,
    STORAGE_OPERATION_CLOSE,
    STORAGE_OPERATION_UNMOUNT
} storage_operation_t;

typedef enum {
    STORAGE_IO_OK = 0,
    STORAGE_IO_MEDIA_ABSENT,
    STORAGE_IO_CORRUPT,
    STORAGE_IO_ERROR,
    STORAGE_IO_SHORT
} storage_io_outcome_t;

typedef struct {
    volatile storage_media_state_t state;
    volatile storage_result_t last_result;
    volatile uint32_t error_count;
    volatile uint32_t dropped_records;
    volatile uint32_t records_written;
    volatile uint8_t recovery_requested;
} storage_policy_t;

void storage_policy_initialize(storage_policy_t *policy, uint8_t enabled);
void storage_policy_mark_ready(storage_policy_t *policy);
void storage_policy_mark_failed(storage_policy_t *policy,
    storage_result_t result);
void storage_policy_request_recovery(storage_policy_t *policy);
uint8_t storage_policy_take_recovery(storage_policy_t *policy);
uint8_t storage_policy_accepts_logs(const storage_policy_t *policy);
uint32_t storage_policy_queue_budget(uint32_t available, uint32_t maximum);
uint8_t storage_policy_name_index_valid(uint32_t index, uint32_t maximum);
uint8_t storage_policy_usb_ownership_allowed(uint8_t msc_enabled);
storage_result_t storage_policy_classify_result(storage_operation_t operation,
    storage_io_outcome_t outcome);

#endif /* STORAGE_POLICY_H */
