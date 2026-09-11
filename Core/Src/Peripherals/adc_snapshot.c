#include "Peripherals/adc_snapshot.h"

static void adc_snapshot_memory_barrier(void)
{
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
}

void adc_snapshot_store_reset(adc_snapshot_store_t *store)
{
    uint32_t channel;

    if (store == 0) {
        return;
    }
    store->publish_guard = 0U;
    store->sample_sequence = 0U;
    store->captured_at_ms = 0U;
    store->error_flags = 0U;
    store->initialized = 0U;
    for (channel = 0U; channel < ADC_SNAPSHOT_CHANNEL_COUNT; channel++) {
        store->channels[channel] = 0U;
    }
    adc_snapshot_memory_barrier();
}

void adc_snapshot_publish(adc_snapshot_store_t *store,
    const volatile uint16_t *completed_scan, uint32_t captured_at_ms)
{
    uint32_t channel;

    if (store == 0 || completed_scan == 0) {
        return;
    }
    store->publish_guard++;
    adc_snapshot_memory_barrier();
    for (channel = 0U; channel < ADC_SNAPSHOT_CHANNEL_COUNT; channel++) {
        store->channels[channel] = completed_scan[channel];
    }
    store->captured_at_ms = captured_at_ms;
    store->sample_sequence++;
    store->initialized = 1U;
    adc_snapshot_memory_barrier();
    store->publish_guard++;
}

void adc_snapshot_report_error(adc_snapshot_store_t *store,
    uint32_t error_flags)
{
    if (store == 0) {
        return;
    }
    store->publish_guard++;
    adc_snapshot_memory_barrier();
    store->error_flags |= (error_flags != 0U) ? error_flags : 1U;
    adc_snapshot_memory_barrier();
    store->publish_guard++;
}

adc_snapshot_status_t adc_snapshot_read(const adc_snapshot_store_t *store,
    adc_snapshot_t *snapshot, uint32_t now_ms, uint32_t max_age_ms)
{
    uint32_t attempt;

    if (store == 0 || snapshot == 0) {
        return ADC_SNAPSHOT_INVALID_ARGUMENT;
    }

    for (attempt = 0U; attempt < ADC_SNAPSHOT_READ_RETRIES; attempt++) {
        uint32_t channel;
        const uint32_t guard_before = store->publish_guard;

        if ((guard_before & 1U) != 0U) {
            continue;
        }
        adc_snapshot_memory_barrier();
        for (channel = 0U; channel < ADC_SNAPSHOT_CHANNEL_COUNT; channel++) {
            snapshot->channels[channel] = store->channels[channel];
        }
        snapshot->sample_sequence = store->sample_sequence;
        snapshot->captured_at_ms = store->captured_at_ms;
        snapshot->error_flags = store->error_flags;
        {
            const uint8_t initialized = store->initialized;
            uint32_t guard_after;

            adc_snapshot_memory_barrier();
            guard_after = store->publish_guard;
            if (guard_before != guard_after || (guard_after & 1U) != 0U) {
                continue;
            }
            if (snapshot->error_flags != 0U) {
                return ADC_SNAPSHOT_ERROR;
            }
            if (initialized == 0U) {
                return ADC_SNAPSHOT_UNINITIALIZED;
            }
            if ((uint32_t)(now_ms - snapshot->captured_at_ms) > max_age_ms) {
                return ADC_SNAPSHOT_STALE;
            }
            return ADC_SNAPSHOT_OK;
        }
    }
    return ADC_SNAPSHOT_BUSY;
}
