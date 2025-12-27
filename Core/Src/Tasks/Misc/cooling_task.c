#include "Tasks/Misc/cooling_task.h"

// Task: Cooling
static const struct {
    float r_value;
    float temp;
} 
thermistorTempLookup[] = {
    {332776.0f, -40.0f},
    {96481.0f,  -20.0f}, 
    {32566.0f,    0.0f}, 
    {12486.0f,   20.0f}, 
    {10000.0f,   25.0f}, 
    {5331.0f,    40.0f}, 
    {2490.0f,    60.0f}, 
    {1071.0f,    85.0f}, 
    {678.0f,    100.0f}, 
    {338.0f,    120.0f}  
};

void cooling_task(void *argument) {
    for (;;) {
        TickType_t start = xTaskGetTickCount();
        uint16_t reading1 = adc_buffer[THERMISTOR_PIN1];
        uint16_t reading2 = adc_buffer[THERMISTOR_PIN2];
        
        float convertedVoltage = ADC_TO_VOLTS(reading1);
        float before_radiator_resistance = VOLTAGE_DIVIDER_RESISTANCE(convertedVoltage);
        float temp_before_Radiator = thermistorToCelsius(before_radiator_resistance);
        
        float convertedVoltageAfter = ADC_TO_VOLTS(reading2);
        float after_radiator_resistance = VOLTAGE_DIVIDER_RESISTANCE(convertedVoltageAfter);
        float temp_after_Radiator = thermistorToCelsius(after_radiator_resistance);
        
        // printing shenangins because no floating points
        // Convert to integer representation (one decimal place)
        int temp_before_int = (int)(temp_before_Radiator * 10); 
        int temp_after_int  = (int)(temp_after_Radiator * 10); 


        serial_print("Temperature before Radiator: %d.%d\r\n", temp_before_int / 10, temp_before_int % 10);
        serial_print("Temperature after Radiator: %d.%d\r\n", temp_after_int / 10, temp_after_int % 10);
    
        xEventGroupSetBits(data->idwg_group, WD_COOLING);
        vTaskDelayUntil(&start, pdMS_TO_TICKS(COOLING_DELAY_MS));
    }
}

float evaluateExpression(float x) {
    float exponent = -1.11 * pow(10, -4) * x;
    float result = 76.9 * exp(exponent);
    return result;
}

float thermistorToCelsius(const float reading) {
    for (int i = 1; i < (int)(sizeof(thermistorTempLookup) / sizeof(thermistorTempLookup[0])); ++i) {
        if (reading >= thermistorTempLookup[i].r_value) {
            float t1 = thermistorTempLookup[i-1].temp;
            float t2 = thermistorTempLookup[i].temp;
            float r1 = thermistorTempLookup[i-1].r_value;
            float r2 = thermistorTempLookup[i].r_value;

            float ratio = (reading - r2) / (r1 - r2);
            return t2 + ratio * (t1 - t2);
        }
    }
    return (float)thermistorTempLookup[sizeof(thermistorTempLookup) / sizeof(thermistorTempLookup[0]) - 1].temp;
}


task_entry_t create_cooling_task(app_data_t *data) {
    task_entry_t entry = {0};
    BaseType_t status = xTaskCreate(
        cooling_task,            
        "Cooling",               // Task name (string)
        COOLING_STACK_SIZE,                     // Stack size (words, adjust as needed)
        data,                    // Task parameters
        Cooling_PRIO,    // Priority (adjust as needed)
        &entry.handle             // Task handle
    );
    configASSERT(status == pdPASS);
    vTaskSuspend(entry.handle);
    entry.name = "cooling";
    return entry;
}
