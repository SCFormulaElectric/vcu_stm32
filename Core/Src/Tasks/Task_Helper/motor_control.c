#include "Tasks/Task_Helper/inverter_fault_status_policy.h"

uint8_t is_fault(const volatile fault_codes_t *faults) {
    return  (faults->INV_Post_Fault_Lo != 0) ||
            (faults->INV_Post_Fault_Hi != 0) ||
            (faults->INV_Run_Fault_Lo  != 0) ||
            (faults->INV_Run_Fault_Hi  != 0);
}

uint8_t inverter_fault_status_snapshot_is_fresh(
    const inverter_fault_status_snapshot_t *snapshot, uint32_t now,
    uint32_t timeout_ticks)
{
    return (snapshot != 0 && snapshot->valid != 0U &&
        (now - snapshot->last_tick) <= timeout_ticks) ? 1U : 0U;
}
