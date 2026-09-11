#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "Tasks/Task_Helper/pedal_plausibility.h"

static void test_normalization(void)
{
    assert(pedal_normalize(399U, 400U, 3681U) == 0U);
    assert(pedal_normalize(400U, 400U, 3681U) == 0U);
    assert(pedal_normalize(3681U, 400U, 3681U) == 1000U);
    assert(pedal_normalize(2000U, 400U, 3600U) == 500U);
    assert(pedal_normalize(1000U, 1000U, 1000U) == 0U);
}

static void test_disagreement_boundary(void)
{
    assert(pedal_values_disagree(500U, 600U, 100U) == 0U);
    assert(pedal_values_disagree(500U, 601U, 100U) == 1U);
    assert(pedal_values_disagree(601U, 500U, 100U) == 1U);
}

static void test_disagreement_persistence(void)
{
    pedal_persistence_timer_t timer = {0};

    assert(pedal_condition_persisted(&timer, 1U, 1000U, 100U) == 0U);
    assert(pedal_condition_persisted(&timer, 1U, 1099U, 100U) == 0U);
    assert(pedal_condition_persisted(&timer, 1U, 1100U, 100U) == 1U);
    assert(pedal_condition_persisted(&timer, 0U, 1101U, 100U) == 0U);
    assert(timer.active == 0U);

    assert(pedal_condition_persisted(&timer, 1U, UINT32_MAX - 15U, 100U) == 0U);
    assert(pedal_condition_persisted(&timer, 1U, 84U, 100U) == 1U);
}

static void test_brake_sensor_fault_transitions(void)
{
    uint8_t fault = 0U;

    fault = brake_sensor_fault_update(fault, 0U, 0U, 0U);
    assert(fault == 1U);
    fault = brake_sensor_fault_update(fault, 1U, 1U, 0U);
    assert(fault == 1U);
    fault = brake_sensor_fault_update(0U, 1U, 1U, 0U);
    assert(fault == 0U);
    fault = brake_sensor_fault_update(fault, 1U, 1U, 1U);
    assert(fault == 1U);
    fault = brake_sensor_fault_update(fault, 1U, 0U, 0U);
    assert(fault == 0U);
}

static void test_overlap_latch(void)
{
    uint8_t latched = 0U;

    latched = brake_throttle_overlap_latch_update(
        latched, 250U, 101U, 250U, 50U, 100U);
    assert(latched == 0U);
    latched = brake_throttle_overlap_latch_update(
        latched, 251U, 100U, 250U, 50U, 100U);
    assert(latched == 0U);

    latched = brake_throttle_overlap_latch_update(
        latched, 251U, 101U, 250U, 50U, 100U);
    assert(latched == 1U);

    latched = brake_throttle_overlap_latch_update(
        latched, 200U, 0U, 250U, 50U, 100U);
    assert(latched == 1U);
    latched = brake_throttle_overlap_latch_update(
        latched, 50U, 0U, 250U, 50U, 100U);
    assert(latched == 1U);
    latched = brake_throttle_overlap_latch_update(
        latched, 49U, 0U, 250U, 50U, 100U);
    assert(latched == 0U);
}

static void test_torque_rise_limit(void)
{
    assert(torque_request_limit_rise(0U, 100U, 25U) == 25U);
    assert(torque_request_limit_rise(100U, 125U, 25U) == 125U);
    assert(torque_request_limit_rise(100U, 126U, 25U) == 125U);
    assert(torque_request_limit_rise(200U, 50U, 25U) == 50U);
    assert(torque_request_limit_rise(200U, 0U, 25U) == 0U);
}

int main(void)
{
    test_normalization();
    test_disagreement_boundary();
    test_disagreement_persistence();
    test_brake_sensor_fault_transitions();
    test_overlap_latch();
    test_torque_rise_limit();
    puts("pedal plausibility tests passed");
    return 0;
}
