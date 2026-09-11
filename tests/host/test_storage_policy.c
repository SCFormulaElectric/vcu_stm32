#include <assert.h>
#include <stdio.h>

#include "Peripherals/storage_policy.h"

static void assert_failure_and_recovery(storage_result_t result)
{
    storage_policy_t policy;

    storage_policy_initialize(&policy, 1U);
    storage_policy_mark_failed(&policy, result);
    assert(policy.state == STORAGE_MEDIA_FAILED);
    assert(policy.last_result == result);
    assert(policy.error_count == 1U);
    assert(storage_policy_accepts_logs(&policy) == 0U);
    assert(storage_policy_take_recovery(&policy) == 0U);
    storage_policy_request_recovery(&policy);
    assert(storage_policy_take_recovery(&policy) == 1U);
    assert(policy.state == STORAGE_MEDIA_UNMOUNTED);
    assert(policy.last_result == STORAGE_RESULT_NONE);
}

static void test_failure_classes(void)
{
    assert_failure_and_recovery(STORAGE_RESULT_MEDIA_ABSENT);
    assert_failure_and_recovery(STORAGE_RESULT_CORRUPT);
    assert_failure_and_recovery(STORAGE_RESULT_IO_ERROR);
    assert_failure_and_recovery(STORAGE_RESULT_SHORT_WRITE);
    assert_failure_and_recovery(STORAGE_RESULT_SYNC_ERROR);
    assert_failure_and_recovery(STORAGE_RESULT_CLOSE_ERROR);
    assert_failure_and_recovery(STORAGE_RESULT_UNMOUNT_ERROR);
}

static void test_enable_ready_disable_behavior(void)
{
    storage_policy_t policy;

    storage_policy_initialize(&policy, 0U);
    assert(policy.state == STORAGE_MEDIA_DISABLED);
    assert(storage_policy_accepts_logs(&policy) == 0U);
    storage_policy_initialize(&policy, 1U);
    assert(policy.state == STORAGE_MEDIA_UNMOUNTED);
    storage_policy_mark_ready(&policy);
    assert(policy.state == STORAGE_MEDIA_READY);
    assert(storage_policy_accepts_logs(&policy) == 1U);
}

static void test_bounded_helpers_and_usb_contract(void)
{
    assert(storage_policy_queue_budget(0U, 8U) == 0U);
    assert(storage_policy_queue_budget(5U, 8U) == 5U);
    assert(storage_policy_queue_budget(20U, 8U) == 8U);
    assert(storage_policy_name_index_valid(1U, 9999U) == 1U);
    assert(storage_policy_name_index_valid(9999U, 9999U) == 1U);
    assert(storage_policy_name_index_valid(10000U, 9999U) == 0U);
    assert(storage_policy_usb_ownership_allowed(0U) == 0U);
    assert(storage_policy_usb_ownership_allowed(1U) == 1U);
    assert(storage_policy_classify_result(STORAGE_OPERATION_MOUNT,
        STORAGE_IO_MEDIA_ABSENT) == STORAGE_RESULT_MEDIA_ABSENT);
    assert(storage_policy_classify_result(STORAGE_OPERATION_MOUNT,
        STORAGE_IO_CORRUPT) == STORAGE_RESULT_CORRUPT);
    assert(storage_policy_classify_result(STORAGE_OPERATION_WRITE,
        STORAGE_IO_ERROR) == STORAGE_RESULT_IO_ERROR);
    assert(storage_policy_classify_result(STORAGE_OPERATION_WRITE,
        STORAGE_IO_SHORT) == STORAGE_RESULT_SHORT_WRITE);
    assert(storage_policy_classify_result(STORAGE_OPERATION_SYNC,
        STORAGE_IO_ERROR) == STORAGE_RESULT_SYNC_ERROR);
}

int main(void)
{
    test_failure_classes();
    test_enable_ready_disable_behavior();
    test_bounded_helpers_and_usb_contract();
    puts("storage policy tests passed");
    return 0;
}
