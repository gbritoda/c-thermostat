#include "hal.h"
#include "thermostat_logic.h"

int main(void) {
    // 1. Initialize hardware peripherals and controller state
    // TODO: I want the HAL simulation to be more lower level
    // Instead of just printing to stdout maybe a simulation of PWM or something
    HAL_Init();
    Thermostat_Init();

    float current_temp;
    // 2. Super-loop
    while (HAL_ReadTemperature(&current_temp)) {
        bool heater_cmd = Thermostat_ComputeControl(current_temp);
        if (Thermostat_IsFaulted()) {
            HAL_ReportFault();
        } else {
            HAL_SetHeaterState(heater_cmd);
        }
    }

    return 0;
}
