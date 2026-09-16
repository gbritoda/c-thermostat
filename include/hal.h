#ifndef HAL_H
#define HAL_H

#include <stdbool.h>

/*
 * Host co-simulation wire protocol (stdin/stdout):
 *   stdin  <- one temperature reading per line, as a float in degrees C.
 *   stdout -> one line per update: "HEATER_ON", "HEATER_OFF", or "FAULT",
 *             flushed immediately after each write.
 * The stream closing (EOF) represents the sensor/link going away.
 */

/**
 * @brief Initialize hardware peripherals and I/O streams.
 */
void HAL_Init(void);

/**
 * @brief Read the current temperature from the environment.
 * @param out_temp Destination for the temperature reading, in degrees Celsius.
 * @return true if a reading was obtained, false if the sensor/stream is no
 *         longer available (EOF or malformed data), in which case *out_temp
 *         is left unmodified.
 */
bool HAL_ReadTemperature(float *out_temp);

/**
 * @brief Control the state of the heater actuator.
 * @param state True to turn heater ON, false to turn OFF.
 */
void HAL_SetHeaterState(bool state);

/**
 * @brief Report that the controller has latched into a fault state.
 */
void HAL_ReportFault(void);

#endif // HAL_H
