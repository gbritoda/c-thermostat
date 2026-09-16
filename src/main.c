#include "hal.h"
#include "sensor_adc.h"
#include "thermostat_logic.h"

int main(void) {
    // 1. Initialize hardware peripherals and controller state
    HAL_Init();
    Thermostat_Init();

    // 2. Super-loop
    uint16_t counts;
    HAL_FrameStatus_t status;
    while ((status = HAL_ReadSensorFrame(&counts)) != HAL_FRAME_STREAM_CLOSED) {
        float current_temp;
        bool heater_cmd = false;

        if (status == HAL_FRAME_OK && SensorAdc_ConvertToCelsius(counts, &current_temp)) {
            heater_cmd = Thermostat_ComputeControl(current_temp);
        } else {
            // Corrupted frame or rail-stuck ADC reading: same latched
            // fault path as an out-of-range Celsius value.
            Thermostat_ForceFault();
        }

        if (Thermostat_IsFaulted()) {
            HAL_ReportFault();
        } else {
            HAL_SetHeaterState(heater_cmd);
        }
    }

    return 0;
}
