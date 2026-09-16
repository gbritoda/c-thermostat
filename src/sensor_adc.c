#include "sensor_adc.h"
#include "thermostat_config.h"

/** Converts ADC counts to Celsius.
 * @param counts The raw ADC reading.
 * @param out_celsius Pointer to the output temperature in Celsius.
 * @return true if the conversion was successful, false otherwise.
 * 
 * ADC_RAIL_GUARD_COUNTS is used to define a guard band at both ends of the ADC range to avoid rail-stuck readings.
 * The sensor going open-circuit or shorted will make the input voltage either 0V or Vcc,
 * which will result in ADC readings near 0 or ADC_COUNTS_MAX.
 * These readings are considered invalid and will return false.
 * 
 */
bool SensorAdc_ConvertToCelsius(uint16_t counts, float *out_celsius) {
    if (counts <= ADC_RAIL_GUARD_COUNTS || counts >= (ADC_COUNTS_MAX - ADC_RAIL_GUARD_COUNTS)) {
        return false;
    }

    // Linearly map the valid ADC count span to the target Celsius range:
    //   1. fraction = (counts - low_guard) / valid_count_span
    //   2. celsius  = THERMOSTAT_SENSOR_MIN_C + (fraction * celsius_span)
    // Calculation is done with the assumption that any measurement outside the valid range is a sensor fault,
    // and thus will not be converted to Celsius.
    const uint16_t span_counts = (uint16_t)(ADC_COUNTS_MAX - (2u * ADC_RAIL_GUARD_COUNTS));
    const float span_celsius = THERMOSTAT_SENSOR_MAX_C - THERMOSTAT_SENSOR_MIN_C;
    const float fraction = (float)(counts - ADC_RAIL_GUARD_COUNTS) / (float)span_counts;

    *out_celsius = THERMOSTAT_SENSOR_MIN_C + fraction * span_celsius;
    return true;
}
