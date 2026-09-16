cmake_minimum_required(VERSION 3.10)
project(emb_thermostat C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

include_directories(include)

add_executable(thermostat
    src/main.c
    src/hal_sim.c
    src/sensor_adc.c
    src/thermostat_logic.c
)

enable_testing()

# Linked without hal_sim.c: proves the control logic has no I/O dependency.
add_executable(test_thermostat_logic
    test/test_thermostat_logic.c
    src/thermostat_logic.c
)

add_test(NAME thermostat_logic COMMAND test_thermostat_logic)

# Linked without hal_sim.c: proves the ADC conversion has no I/O dependency.
add_executable(test_sensor_adc
    test/test_sensor_adc.c
    src/sensor_adc.c
)

add_test(NAME sensor_adc COMMAND test_sensor_adc)
