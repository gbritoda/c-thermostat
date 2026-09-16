#ifndef THERMOSTAT_CONFIG_H
#define THERMOSTAT_CONFIG_H

/* Hysteresis band: heater turns on below LOW, off above HIGH. */
#define THERMOSTAT_SETPOINT_LOW_C  20.5f
#define THERMOSTAT_SETPOINT_HIGH_C 21.5f

/* Plausible sensor range. Readings outside this window are treated as a
 * sensor fault (disconnected/shorted probe, glitch, etc.) rather than a
 * real temperature. */
#define THERMOSTAT_SENSOR_MIN_C (-40.0f)
#define THERMOSTAT_SENSOR_MAX_C (85.0f)

#endif // THERMOSTAT_CONFIG_H
