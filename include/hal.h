#ifndef HAL_H
#define HAL_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Host co-simulation wire protocol:
 *
 *   stdin  <- one 4-byte sensor frame per tick:
 *               byte 0: SYNC     = 0xAA
 *               byte 1: COUNTS_HI (raw 12-bit ADC count, MSB)
 *               byte 2: COUNTS_LO (raw 12-bit ADC count, LSB)
 *               byte 3: CHECKSUM = SYNC ^ COUNTS_HI ^ COUNTS_LO
 *
 *   stdout -> one line per update: "HEATER_ON", "HEATER_OFF", or "FAULT",
 *             flushed immediately after each write.
 *
 * EOF on stdin represents the sensor link going away.
 */

/**
 * @brief Initialize hardware peripherals and I/O streams.
 */
void HAL_Init(void);

typedef enum {
    HAL_FRAME_OK,           /**< *out_counts holds a freshly read frame's value. */
    HAL_FRAME_INVALID,      /**< Sync or checksum mismatch: a corrupted frame. Recoverable -- keep reading. */
    HAL_FRAME_STREAM_CLOSED /**< EOF: the sensor link is gone. Stop reading. */
} HAL_FrameStatus_t;

/**
 * @brief Read one raw ADC sensor frame from the environment.
 * @param out_counts Destination for the raw ADC count. Only written when
 *        the return value is HAL_FRAME_OK.
 * @return The outcome of the read; see HAL_FrameStatus_t.
 */
HAL_FrameStatus_t HAL_ReadSensorFrame(uint16_t *out_counts);

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
