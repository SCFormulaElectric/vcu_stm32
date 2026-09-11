#ifndef PEDAL_PLAUSIBILITY_H
#define PEDAL_PLAUSIBILITY_H

#include <stdint.h>

typedef struct {
    uint32_t started_at;
    uint8_t active;
} pedal_persistence_timer_t;

uint16_t pedal_normalize(uint16_t input, uint16_t min_value, uint16_t max_value);
uint8_t pedal_values_disagree(uint16_t value_1, uint16_t value_2,
    uint16_t max_difference);
uint8_t pedal_condition_persisted(pedal_persistence_timer_t *timer,
    uint8_t condition, uint32_t now, uint32_t required_duration);
uint8_t brake_sensor_fault_update(uint8_t current_fault,
    uint8_t inputs_valid, uint8_t sensors_disagree,
    uint8_t disagreement_persisted);
uint8_t brake_throttle_overlap_latch_update(uint8_t latched,
    uint16_t throttle_level, uint16_t brake_level,
    uint16_t throttle_trip_threshold, uint16_t throttle_reset_threshold,
    uint16_t brake_trip_threshold);
uint16_t torque_request_limit_rise(uint16_t previous_request,
    uint16_t requested, uint16_t maximum_rise);

#endif /* PEDAL_PLAUSIBILITY_H */
