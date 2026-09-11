#ifndef INVERTER_FAULT_STATUS_POLICY_H
#define INVERTER_FAULT_STATUS_POLICY_H

#include <stdint.h>

typedef struct {
    uint16_t INV_Post_Fault_Lo;
    uint16_t INV_Post_Fault_Hi;
    uint16_t INV_Run_Fault_Lo;
    uint16_t INV_Run_Fault_Hi;
} fault_codes_t;

typedef struct {
    fault_codes_t fault_codes;
    uint32_t last_tick;
    uint8_t valid;
} inverter_fault_status_snapshot_t;

uint8_t is_fault(const volatile fault_codes_t *faults);
uint8_t inverter_fault_status_snapshot_is_fresh(
    const inverter_fault_status_snapshot_t *snapshot, uint32_t now,
    uint32_t timeout_ticks);

#endif /* INVERTER_FAULT_STATUS_POLICY_H */
