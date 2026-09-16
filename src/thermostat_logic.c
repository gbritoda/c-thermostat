#include "thermostat_logic.h"
#include "thermostat_config.h"

typedef enum {
    STATE_OFF,
    STATE_HEATING,
    STATE_FAULT
} ThermostatState_t;

static ThermostatState_t current_state = STATE_OFF;

void Thermostat_Init(void) {
    current_state = STATE_OFF;
}

bool Thermostat_IsFaulted(void) {
    return current_state == STATE_FAULT;
}

void Thermostat_ForceFault(void) {
    current_state = STATE_FAULT;
}

bool Thermostat_ComputeControl(float current_temp) {
    // Latched fault: once tripped, stay off until Thermostat_Init().
    if (current_state == STATE_FAULT) {
        return false;
    }

    if (current_temp < THERMOSTAT_SENSOR_MIN_C || current_temp > THERMOSTAT_SENSOR_MAX_C) {
        current_state = STATE_FAULT;
        return false;
    }

    // Hysteresis control: target window THERMOSTAT_SETPOINT_LOW_C - THERMOSTAT_SETPOINT_HIGH_C
    if (current_temp < THERMOSTAT_SETPOINT_LOW_C) {
        current_state = STATE_HEATING;
    } else if (current_temp > THERMOSTAT_SETPOINT_HIGH_C) {
        current_state = STATE_OFF;
    }
    // If temperature is in the deadband, maintain current_state

    return (current_state == STATE_HEATING);
}
