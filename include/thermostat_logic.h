#ifndef THERMOSTAT_LOGIC_H
#define THERMOSTAT_LOGIC_H

#include <stdbool.h>

/**
 * @brief Reset the controller to its initial state (heater off).
 */
void Thermostat_Init(void);

/**
 * @brief Compute the required heater state based on the current temperature.
 *
 * If current_temp falls outside the plausible sensor range, the controller
 * latches into a fault state: the heater is forced off and stays off on
 * every subsequent call, even if later readings look normal, until
 * Thermostat_Init() is called.
 *
 * @param current_temp The latest temperature reading.
 * @return true if the heater should be active, false otherwise.
 */
bool Thermostat_ComputeControl(float current_temp);

/**
 * @brief Check whether the controller is latched in a fault state.
 * @return true if a sensor fault has been detected since the last
 *         Thermostat_Init().
 */
bool Thermostat_IsFaulted(void);

/**
 * @brief Latch the controller into a fault state directly.
 *
 * For faults detected below the Celsius reading itself (e.g. a corrupted
 * sensor frame, or a rail-stuck ADC count); same latched behaviour as a
 * fault detected inside Thermostat_ComputeControl.
 */
void Thermostat_ForceFault(void);

#endif // THERMOSTAT_LOGIC_H