#include "hal.h"
#include "thermostat_logic.h"

int main(void) {
    // 1. Initialize hardware peripherals and controller state
    HAL_Init();
    Thermostat_Init();

    // 2. Super-loop
    float current_temp;
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
