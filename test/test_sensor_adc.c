/*
 * Unit tests for sensor_adc.c: pure conversion logic, no I/O dependency.
 */
#include "sensor_adc.h"
#include "thermostat_config.h"
#include <assert.h>
#include <stdio.h>

static float abs_diff(float a, float b) {
    float diff = a - b;
    return diff < 0.0f ? -diff : diff;
}

static void test_mid_range_count_converts_to_expected_celsius(void) {
    float celsius;
    assert(SensorAdc_ConvertToCelsius(2048, &celsius) == true);

    float span_counts = (float)(ADC_COUNTS_MAX - 2u * ADC_RAIL_GUARD_COUNTS);
    float expected = THERMOSTAT_SENSOR_MIN_C +
                      ((2048.0f - ADC_RAIL_GUARD_COUNTS) / span_counts) *
                          (THERMOSTAT_SENSOR_MAX_C - THERMOSTAT_SENSOR_MIN_C);
    assert(abs_diff(celsius, expected) < 0.01f);
}

static void test_counts_pinned_at_rails_are_faults(void) {
    float celsius;
    assert(SensorAdc_ConvertToCelsius(0, &celsius) == false);
    assert(SensorAdc_ConvertToCelsius(ADC_COUNTS_MAX, &celsius) == false);
}

static void test_counts_inside_guard_band_are_faults(void) {
    float celsius;
    assert(SensorAdc_ConvertToCelsius(ADC_RAIL_GUARD_COUNTS, &celsius) == false);
    assert(SensorAdc_ConvertToCelsius((uint16_t)(ADC_COUNTS_MAX - ADC_RAIL_GUARD_COUNTS), &celsius) == false);
}

static void test_counts_just_past_guard_band_succeed(void) {
    float celsius;
    assert(SensorAdc_ConvertToCelsius(ADC_RAIL_GUARD_COUNTS + 1, &celsius) == true);
    assert(SensorAdc_ConvertToCelsius((uint16_t)(ADC_COUNTS_MAX - ADC_RAIL_GUARD_COUNTS - 1), &celsius) == true);
}

int main(void) {
    test_mid_range_count_converts_to_expected_celsius();
    test_counts_pinned_at_rails_are_faults();
    test_counts_inside_guard_band_are_faults();
    test_counts_just_past_guard_band_succeed();

    printf("All sensor_adc tests passed.\n");
    return 0;
}
