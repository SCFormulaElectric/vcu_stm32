#include "Tasks/Task_Helper/pedal_plausibility.h"

uint16_t pedal_normalize(uint16_t input, uint16_t min_value, uint16_t max_value)
{
    if (min_value >= max_value || input <= min_value) {
        return 0U;
    }
    if (input >= max_value) {
        return 1000U;
    }
    return (uint16_t)(((uint32_t)(input - min_value) * 1000U) /
        (uint32_t)(max_value - min_value));
}

uint8_t pedal_values_disagree(uint16_t value_1, uint16_t value_2,
    uint16_t max_difference)
{
    const uint16_t difference = (value_1 >= value_2) ?
        (uint16_t)(value_1 - value_2) : (uint16_t)(value_2 - value_1);
    return (difference > max_difference) ? 1U : 0U;
}

uint8_t pedal_condition_persisted(pedal_persistence_timer_t *timer,
    uint8_t condition, uint32_t now, uint32_t required_duration)
{
    if (timer == 0) {
        return 0U;
    }
    if (condition == 0U) {
        timer->active = 0U;
        timer->started_at = 0U;
        return 0U;
    }
    if (timer->active == 0U) {
        timer->active = 1U;
        timer->started_at = now;
        return (required_duration == 0U) ? 1U : 0U;
    }
    return ((uint32_t)(now - timer->started_at) >= required_duration) ? 1U : 0U;
}

uint8_t brake_sensor_fault_update(uint8_t current_fault,
    uint8_t inputs_valid, uint8_t sensors_disagree,
    uint8_t disagreement_persisted)
{
    if (inputs_valid == 0U) {
        return 1U;
    }
    if (sensors_disagree == 0U) {
        return 0U;
    }
    if (disagreement_persisted != 0U) {
        return 1U;
    }
    return (current_fault != 0U) ? 1U : 0U;
}

uint8_t brake_throttle_overlap_latch_update(uint8_t latched,
    uint16_t throttle_level, uint16_t brake_level,
    uint16_t throttle_trip_threshold, uint16_t throttle_reset_threshold,
    uint16_t brake_trip_threshold)
{
    if (throttle_level < throttle_reset_threshold) {
        return 0U;
    }
    if (throttle_level > throttle_trip_threshold &&
        brake_level > brake_trip_threshold) {
        return 1U;
    }
    return (latched != 0U) ? 1U : 0U;
}

uint16_t torque_request_limit_rise(uint16_t previous_request,
    uint16_t requested, uint16_t maximum_rise)
{
    if (requested <= previous_request) {
        return requested;
    }
    if ((uint16_t)(requested - previous_request) <= maximum_rise) {
        return requested;
    }
    return (uint16_t)(previous_request + maximum_rise);
}
