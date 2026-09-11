#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "Tasks/Task_Helper/inverter_fault_status_policy.h"

static void test_complete_snapshot_decision(void)
{
    inverter_fault_status_snapshot_t snapshot = {0};

    assert(inverter_fault_status_snapshot_is_fresh(&snapshot, 100U, 250U) ==
        0U);
    snapshot.valid = 1U;
    snapshot.last_tick = 50U;
    snapshot.fault_codes.INV_Run_Fault_Hi = 0x0100U;
    assert(inverter_fault_status_snapshot_is_fresh(&snapshot, 300U, 250U) ==
        1U);
    assert(is_fault(&snapshot.fault_codes) != 0U);
    snapshot.fault_codes.INV_Run_Fault_Hi = 0U;
    assert(is_fault(&snapshot.fault_codes) == 0U);
    assert(inverter_fault_status_snapshot_is_fresh(&snapshot, 301U, 250U) ==
        0U);
}

static void test_wraparound_age(void)
{
    inverter_fault_status_snapshot_t snapshot = {0};

    snapshot.valid = 1U;
    snapshot.last_tick = UINT32_MAX - 9U;
    assert(inverter_fault_status_snapshot_is_fresh(&snapshot, 10U, 20U) ==
        1U);
    assert(inverter_fault_status_snapshot_is_fresh(&snapshot, 11U, 20U) ==
        0U);
}

int main(void)
{
    test_complete_snapshot_decision();
    test_wraparound_age();
    puts("inverter fault snapshot tests passed");
    return 0;
}
