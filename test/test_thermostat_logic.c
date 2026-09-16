/*
 * Unit tests for thermostat_logic.c, compiled and linked without hal_sim.c
 * to demonstrate that the control logic has zero I/O dependency.
 */
#include "thermostat_logic.h"
#include "thermostat_config.h"
#include <assert.h>
#include <stdio.h>

static void test_starts_off_and_heats_below_low_setpoint(void) {
    Thermostat_Init();
    assert(Thermostat_ComputeControl(THERMOSTAT_SETPOINT_LOW_C + 1.0f) == false);
    assert(Thermostat_ComputeControl(THERMOSTAT_SETPOINT_LOW_C - 0.1f) == true);
}

static void test_stays_on_through_deadband(void) {
    Thermostat_Init();
    assert(Thermostat_ComputeControl(THERMOSTAT_SETPOINT_LOW_C - 0.1f) == true);
    // Midpoint of the deadband: should keep heating rather than switch off.
    float midpoint = (THERMOSTAT_SETPOINT_LOW_C + THERMOSTAT_SETPOINT_HIGH_C) / 2.0f;
    assert(Thermostat_ComputeControl(midpoint) == true);
}

static void test_turns_off_above_high_setpoint_and_stays_off_in_deadband(void) {
    Thermostat_Init();
    assert(Thermostat_ComputeControl(THERMOSTAT_SETPOINT_LOW_C - 0.1f) == true);
    assert(Thermostat_ComputeControl(THERMOSTAT_SETPOINT_HIGH_C + 0.1f) == false);

    float midpoint = (THERMOSTAT_SETPOINT_LOW_C + THERMOSTAT_SETPOINT_HIGH_C) / 2.0f;
    assert(Thermostat_ComputeControl(midpoint) == false);
}

static void test_init_resets_state(void) {
    Thermostat_Init();
    assert(Thermostat_ComputeControl(THERMOSTAT_SETPOINT_LOW_C - 0.1f) == true);

    Thermostat_Init();
    float midpoint = (THERMOSTAT_SETPOINT_LOW_C + THERMOSTAT_SETPOINT_HIGH_C) / 2.0f;
    assert(Thermostat_ComputeControl(midpoint) == false);
}

static void test_boundary_readings_are_not_faults(void) {
    // The sensor range boundaries are valid (if extreme) readings, not
    // faults: MIN is well below the setpoint band (heater ON), MAX is well
    // above it (heater OFF).
    Thermostat_Init();
    assert(Thermostat_ComputeControl(THERMOSTAT_SENSOR_MIN_C) == true);
    assert(Thermostat_IsFaulted() == false);
    assert(Thermostat_ComputeControl(THERMOSTAT_SENSOR_MAX_C) == false);
    assert(Thermostat_IsFaulted() == false);
}

static void test_out_of_range_reading_trips_fault(void) {
    Thermostat_Init();
    assert(Thermostat_IsFaulted() == false);

    assert(Thermostat_ComputeControl(THERMOSTAT_SENSOR_MAX_C + 1.0f) == false);
    assert(Thermostat_IsFaulted() == true);
}

static void test_fault_latches_despite_later_good_readings(void) {
    Thermostat_Init();
    assert(Thermostat_ComputeControl(THERMOSTAT_SENSOR_MIN_C - 1.0f) == false);
    assert(Thermostat_IsFaulted() == true);

    // A perfectly normal, heater-demanding reading must NOT clear the fault.
    assert(Thermostat_ComputeControl(THERMOSTAT_SETPOINT_LOW_C - 0.1f) == false);
    assert(Thermostat_IsFaulted() == true);
}

static void test_init_clears_fault(void) {
    Thermostat_Init();
    assert(Thermostat_ComputeControl(THERMOSTAT_SENSOR_MAX_C + 1.0f) == false);
    assert(Thermostat_IsFaulted() == true);

    Thermostat_Init();
    assert(Thermostat_IsFaulted() == false);
}

static void test_force_fault_latches_like_a_bad_reading(void) {
    Thermostat_Init();
    assert(Thermostat_IsFaulted() == false);

    Thermostat_ForceFault();
    assert(Thermostat_IsFaulted() == true);
    assert(Thermostat_ComputeControl(THERMOSTAT_SETPOINT_LOW_C - 0.1f) == false);
    assert(Thermostat_IsFaulted() == true);

    Thermostat_Init();
    assert(Thermostat_IsFaulted() == false);
}

int main(void) {
    test_starts_off_and_heats_below_low_setpoint();
    test_stays_on_through_deadband();
    test_turns_off_above_high_setpoint_and_stays_off_in_deadband();
    test_init_resets_state();
    test_boundary_readings_are_not_faults();
    test_out_of_range_reading_trips_fault();
    test_fault_latches_despite_later_good_readings();
    test_init_clears_fault();
    test_force_fault_latches_like_a_bad_reading();

    printf("All thermostat_logic tests passed.\n");
    return 0;
}
