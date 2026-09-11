#include "Peripherals/adc.h"
#include "main.h"

volatile uint16_t adc_dma_buffer[ADC_DMA_BUFFER_COUNT];

static adc_snapshot_store_t adc_snapshot_store;

void adc_acquisition_init(void)
{
    uint32_t index;

    for (index = 0U; index < ADC_DMA_BUFFER_COUNT; index++) {
        adc_dma_buffer[index] = 0U;
    }
    adc_snapshot_store_reset(&adc_snapshot_store);
}

adc_snapshot_status_t adc_acquisition_read(adc_snapshot_t *snapshot,
    uint32_t now_ms)
{
    return adc_snapshot_read(&adc_snapshot_store, snapshot, now_ms,
        ADC_SNAPSHOT_MAX_AGE_MS);
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == &hadc1) {
        adc_snapshot_publish(&adc_snapshot_store, &adc_dma_buffer[0],
            HAL_GetTick());
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == &hadc1) {
        adc_snapshot_publish(&adc_snapshot_store,
            &adc_dma_buffer[ADC_CHANNEL_COUNT], HAL_GetTick());
    }
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == &hadc1) {
        /* Acquisition errors are latched until adc_acquisition_init(), which is
         * called only during boot. A later good scan does not silently recover
         * a safety-critical ADC/DMA error. */
        adc_snapshot_report_error(&adc_snapshot_store, HAL_ADC_GetError(hadc));
    }
}
