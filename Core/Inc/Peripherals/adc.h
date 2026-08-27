#ifndef ADC_H
#define ADC_H
#include <stdint.h>

#define ADC_CHANNEL_COUNT 6
/* ADC1 channels are PC0..PC5, in this order. Keep the pedal/temperature
 * indices below aligned with this hardware map. */

extern uint16_t adc_buffer[ADC_CHANNEL_COUNT];
#endif
