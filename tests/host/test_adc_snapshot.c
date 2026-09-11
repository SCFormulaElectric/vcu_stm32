#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "Peripherals/adc_snapshot.h"

static const uint16_t scan_a[ADC_SNAPSHOT_CHANNEL_COUNT] =
    {101U, 202U, 303U, 404U, 505U, 606U};
static const uint16_t scan_b[ADC_SNAPSHOT_CHANNEL_COUNT] =
    {111U, 222U, 333U, 444U, 555U, 666U};

static void assert_scan(const adc_snapshot_t *snapshot,
    const uint16_t *expected)
{
    uint32_t channel;

    for (channel = 0U; channel < ADC_SNAPSHOT_CHANNEL_COUNT; channel++) {
        assert(snapshot->channels[channel] == expected[channel]);
    }
}

static void test_initialization_and_complete_scan(void)
{
    adc_snapshot_store_t store;
    adc_snapshot_t snapshot = {0};

    adc_snapshot_store_reset(&store);
    assert(adc_snapshot_read(&store, &snapshot, 0U, 50U) ==
        ADC_SNAPSHOT_UNINITIALIZED);

    adc_snapshot_publish(&store, scan_a, 100U);
    assert(adc_snapshot_read(&store, &snapshot, 100U, 50U) == ADC_SNAPSHOT_OK);
    assert(snapshot.sample_sequence == 1U);
    assert(snapshot.captured_at_ms == 100U);
    assert_scan(&snapshot, scan_a);

    adc_snapshot_publish(&store, scan_b, 101U);
    assert(adc_snapshot_read(&store, &snapshot, 101U, 50U) == ADC_SNAPSHOT_OK);
    assert(snapshot.sample_sequence == 2U);
    assert_scan(&snapshot, scan_b);
}

static void test_timeout_boundary_and_timestamp_wrap(void)
{
    adc_snapshot_store_t store;
    adc_snapshot_t snapshot = {0};

    adc_snapshot_store_reset(&store);
    adc_snapshot_publish(&store, scan_a, 100U);
    assert(adc_snapshot_read(&store, &snapshot, 150U, 50U) == ADC_SNAPSHOT_OK);
    assert(adc_snapshot_read(&store, &snapshot, 151U, 50U) == ADC_SNAPSHOT_STALE);

    adc_snapshot_store_reset(&store);
    adc_snapshot_publish(&store, scan_a, UINT32_MAX - 20U);
    assert(adc_snapshot_read(&store, &snapshot, 29U, 50U) == ADC_SNAPSHOT_OK);
    assert(adc_snapshot_read(&store, &snapshot, 30U, 50U) == ADC_SNAPSHOT_STALE);
}

static void test_error_is_latched_until_reset(void)
{
    adc_snapshot_store_t store;
    adc_snapshot_t snapshot = {0};

    adc_snapshot_store_reset(&store);
    adc_snapshot_report_error(&store, 0x02U);
    assert(adc_snapshot_read(&store, &snapshot, 9U, 50U) == ADC_SNAPSHOT_ERROR);
    assert(snapshot.error_flags == 0x02U);

    adc_snapshot_store_reset(&store);
    adc_snapshot_publish(&store, scan_a, 10U);
    adc_snapshot_report_error(&store, 0x04U);
    assert(adc_snapshot_read(&store, &snapshot, 10U, 50U) == ADC_SNAPSHOT_ERROR);
    assert(snapshot.error_flags == 0x04U);

    adc_snapshot_publish(&store, scan_b, 11U);
    assert(adc_snapshot_read(&store, &snapshot, 11U, 50U) == ADC_SNAPSHOT_ERROR);
    assert(snapshot.sample_sequence == 2U);

    adc_snapshot_store_reset(&store);
    assert(adc_snapshot_read(&store, &snapshot, 11U, 50U) ==
        ADC_SNAPSHOT_UNINITIALIZED);
    adc_snapshot_publish(&store, scan_b, 12U);
    assert(adc_snapshot_read(&store, &snapshot, 12U, 50U) == ADC_SNAPSHOT_OK);

    assert(adc_snapshot_read(&store, &snapshot, 63U, 50U) == ADC_SNAPSHOT_STALE);
    adc_snapshot_publish(&store, scan_a, 63U);
    assert(adc_snapshot_read(&store, &snapshot, 63U, 50U) == ADC_SNAPSHOT_OK);
}

static void test_sequence_wrap_and_busy_writer(void)
{
    adc_snapshot_store_t store;
    adc_snapshot_t snapshot = {0};

    adc_snapshot_store_reset(&store);
    store.sample_sequence = UINT32_MAX;
    adc_snapshot_publish(&store, scan_a, 1U);
    assert(adc_snapshot_read(&store, &snapshot, 1U, 50U) == ADC_SNAPSHOT_OK);
    assert(snapshot.sample_sequence == 0U);

    store.publish_guard = 1U;
    assert(adc_snapshot_read(&store, &snapshot, 1U, 50U) == ADC_SNAPSHOT_BUSY);
}

static void test_invalid_arguments(void)
{
    adc_snapshot_store_t store;
    adc_snapshot_t snapshot = {0};

    adc_snapshot_store_reset(&store);
    assert(adc_snapshot_read(0, &snapshot, 0U, 50U) ==
        ADC_SNAPSHOT_INVALID_ARGUMENT);
    assert(adc_snapshot_read(&store, 0, 0U, 50U) ==
        ADC_SNAPSHOT_INVALID_ARGUMENT);
}

int main(void)
{
    test_initialization_and_complete_scan();
    test_timeout_boundary_and_timestamp_wrap();
    test_error_is_latched_until_reset();
    test_sequence_wrap_and_busy_writer();
    test_invalid_arguments();
    puts("ADC snapshot tests passed");
    return 0;
}
