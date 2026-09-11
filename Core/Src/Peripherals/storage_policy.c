#include "Peripherals/storage_policy.h"

void storage_policy_initialize(storage_policy_t *policy, uint8_t enabled)
{
    if (policy == 0) {
        return;
    }
    *policy = (storage_policy_t){0};
    policy->state = (enabled != 0U) ? STORAGE_MEDIA_UNMOUNTED :
        STORAGE_MEDIA_DISABLED;
}

void storage_policy_mark_ready(storage_policy_t *policy)
{
    if (policy != 0) {
        policy->state = STORAGE_MEDIA_READY;
        policy->last_result = STORAGE_RESULT_OK;
    }
}

void storage_policy_mark_failed(storage_policy_t *policy,
    storage_result_t result)
{
    if (policy != 0) {
        policy->state = STORAGE_MEDIA_FAILED;
        policy->last_result = result;
        policy->error_count++;
    }
}

void storage_policy_request_recovery(storage_policy_t *policy)
{
    if (policy != 0 && policy->state == STORAGE_MEDIA_FAILED) {
        policy->recovery_requested = 1U;
    }
}

uint8_t storage_policy_take_recovery(storage_policy_t *policy)
{
    if (policy == 0 || policy->recovery_requested == 0U) {
        return 0U;
    }
    policy->recovery_requested = 0U;
    policy->state = STORAGE_MEDIA_UNMOUNTED;
    policy->last_result = STORAGE_RESULT_NONE;
    return 1U;
}

uint8_t storage_policy_accepts_logs(const storage_policy_t *policy)
{
    return (policy != 0 && policy->state == STORAGE_MEDIA_READY) ? 1U : 0U;
}

uint32_t storage_policy_queue_budget(uint32_t available, uint32_t maximum)
{
    return (available < maximum) ? available : maximum;
}

uint8_t storage_policy_name_index_valid(uint32_t index, uint32_t maximum)
{
    return (index >= 1U && index <= maximum) ? 1U : 0U;
}

uint8_t storage_policy_usb_ownership_allowed(uint8_t msc_enabled)
{
    return (msc_enabled != 0U) ? 1U : 0U;
}

storage_result_t storage_policy_classify_result(storage_operation_t operation,
    storage_io_outcome_t outcome)
{
    if (outcome == STORAGE_IO_OK) {
        return STORAGE_RESULT_OK;
    }
    if (outcome == STORAGE_IO_MEDIA_ABSENT) {
        return STORAGE_RESULT_MEDIA_ABSENT;
    }
    if (outcome == STORAGE_IO_CORRUPT) {
        return STORAGE_RESULT_CORRUPT;
    }
    if (outcome == STORAGE_IO_SHORT && operation == STORAGE_OPERATION_WRITE) {
        return STORAGE_RESULT_SHORT_WRITE;
    }
    switch (operation) {
        case STORAGE_OPERATION_SYNC: return STORAGE_RESULT_SYNC_ERROR;
        case STORAGE_OPERATION_CLOSE: return STORAGE_RESULT_CLOSE_ERROR;
        case STORAGE_OPERATION_UNMOUNT: return STORAGE_RESULT_UNMOUNT_ERROR;
        case STORAGE_OPERATION_MOUNT:
        case STORAGE_OPERATION_OPEN:
        case STORAGE_OPERATION_WRITE:
        default: return STORAGE_RESULT_IO_ERROR;
    }
}
