#include "hal.h"
#include <stdio.h>

void HAL_Init(void) {
    // Initialization code for simulated hardware
    // None at the moment as you can see...
}

bool HAL_ReadTemperature(float *out_temp) {
    float temp;
    // Read the temperature reading sent from the co-simulator via stdin.
    if (scanf("%f", &temp) != 1) {
        // EOF or malformed input: sensor/stream is gone.
        return false;
    }
    *out_temp = temp;
    return true;
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
