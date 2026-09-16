#ifndef SENSOR_ADC_H
#define SENSOR_ADC_H

#include <stdbool.h>
#include <stdint.h>

/* 12-bit ADC. */
#define ADC_COUNTS_MAX 4095u

/* Counts within this many steps of either rail are treated as an
 * open/shorted sensor rather than a real reading. */
#define ADC_RAIL_GUARD_COUNTS 16u

/**
 * @brief Convert a raw ADC count to a calibrated Celsius reading.
 *
 * Linearly maps the valid counts span (rail guard excluded on both ends)
 * to [THERMOSTAT_SENSOR_MIN_C, THERMOSTAT_SENSOR_MAX_C].
 *
 * @param counts Raw ADC count, 0..ADC_COUNTS_MAX.
 * @param out_celsius Destination for the converted temperature.
 * @return true if counts was in the valid span and *out_celsius was
 *         written; false if counts is pinned at/near a rail (sensor
 *         fault), in which case *out_celsius is left unmodified.
 */
bool SensorAdc_ConvertToCelsius(uint16_t counts, float *out_celsius);

#endif // SENSOR_ADC_H
