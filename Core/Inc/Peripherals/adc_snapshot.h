#ifndef ADC_SNAPSHOT_H
#define ADC_SNAPSHOT_H

#include <stdint.h>

#define ADC_SNAPSHOT_CHANNEL_COUNT 6U
#define ADC_SNAPSHOT_READ_RETRIES  3U

typedef enum {
    ADC_SNAPSHOT_OK = 0,
    ADC_SNAPSHOT_UNINITIALIZED,
    ADC_SNAPSHOT_STALE,
    ADC_SNAPSHOT_ERROR,
    ADC_SNAPSHOT_BUSY,
    ADC_SNAPSHOT_INVALID_ARGUMENT
} adc_snapshot_status_t;

typedef struct {
    uint16_t channels[ADC_SNAPSHOT_CHANNEL_COUNT];
    uint32_t sample_sequence;
    uint32_t captured_at_ms;
    uint32_t error_flags;
} adc_snapshot_t;

typedef struct {
    volatile uint32_t publish_guard;
    volatile uint32_t sample_sequence;
    volatile uint32_t captured_at_ms;
    volatile uint32_t error_flags;
    volatile uint16_t channels[ADC_SNAPSHOT_CHANNEL_COUNT];
    volatile uint8_t initialized;
} adc_snapshot_store_t;

void adc_snapshot_store_reset(adc_snapshot_store_t *store);
void adc_snapshot_publish(adc_snapshot_store_t *store,
    const volatile uint16_t *completed_scan, uint32_t captured_at_ms);
void adc_snapshot_report_error(adc_snapshot_store_t *store,
    uint32_t error_flags);
adc_snapshot_status_t adc_snapshot_read(const adc_snapshot_store_t *store,
    adc_snapshot_t *snapshot, uint32_t now_ms, uint32_t max_age_ms);

#endif /* ADC_SNAPSHOT_H */
