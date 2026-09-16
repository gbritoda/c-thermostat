#include "hal.h"
#include <stdio.h>

#define FRAME_SYNC 0xAAu

void HAL_Init(void) {
    // Initialization code for simulated hardware
    // None at the moment as you can see...
}

HAL_FrameStatus_t HAL_ReadSensorFrame(uint16_t *out_counts) {
    int sync = fgetc(stdin);
    if (sync == EOF) {
        // Clean end of stream, between frames: the sensor link is gone.
        return HAL_FRAME_STREAM_CLOSED;
    }

    int counts_hi = fgetc(stdin);
    int counts_lo = fgetc(stdin);
    int checksum = fgetc(stdin);
    if (counts_hi == EOF || counts_lo == EOF || checksum == EOF) {
        // Stream ended mid-frame: treat the same as a clean close, since
        // the co-simulator never sends a partial frame on purpose.
        return HAL_FRAME_STREAM_CLOSED;
    }

    uint8_t expected_checksum = (uint8_t)(sync ^ counts_hi ^ counts_lo);
    if ((uint8_t)sync != FRAME_SYNC || (uint8_t)checksum != expected_checksum) {
        return HAL_FRAME_INVALID;
    }

    *out_counts = (uint16_t)((counts_hi << 8) | counts_lo);
    return HAL_FRAME_OK;
}

void HAL_SetHeaterState(bool state) {
    if (state) {
        printf("HEATER_ON\n");
    } else {
        printf("HEATER_OFF\n");
    }
    // Essential: flush the buffer so the co-simulator receives it instantly
    fflush(stdout);
}

void HAL_ReportFault(void) {
    printf("FAULT\n");
    fflush(stdout);
}
