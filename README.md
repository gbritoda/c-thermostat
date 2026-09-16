# emb-thermostat

A smart thermostat written in embedded-style C that wrote to brush up on some embedded C, co-simulated against a Python script that models a room's thermal dynamics.

It's also an excuse to put some Physics concepts to use...

Runs entirely on the host

## Architecture

Strict one-way dependency (main -> logic, main -> HAL/sensor driver; logic
never depends on the HAL):

- **HAL** ([include/hal.h](include/hal.h), [src/hal_sim.c](src/hal_sim.c))
  the only place that touches I/O. On this host build it reads a framed
  byte protocol over stdin/stdout; on a real target this file (and only
  this file) would be swapped for one that talks to an ADC peripheral and
  a GPIO/PWM line.

- **Sensor driver** ([include/sensor_adc.h](include/sensor_adc.h),
  [src/sensor_adc.c](src/sensor_adc.c)) converts a raw 12-bit ADC count
  into a calibrated Celsius reading, and detects rail-stuck counts
  (disconnected/shorted sensor). Pure function, no I/O

- **Control logic** ([include/thermostat_logic.h](include/thermostat_logic.h),
  [src/thermostat_logic.c](src/thermostat_logic.c)) a hysteresis
  (bang-bang) controller with a 0.5°C band, target window `20.5–21.5°C`
  (see [include/thermostat_config.h](include/thermostat_config.h)), plus a
  latched `FAULT` state: a bad sensor reading forces the heater off and
  keeps it off, until explicitly reset.
  Zero knowledge of streams, files, hardware, or the sensor driver above
  it.

- **Main coordinator** ([src/main.c](src/main.c)) Super loop:
  read a sensor frame, convert it, compute the control decision, drive the actuator, repeat until the sensor stream ends.

## Wire protocol (host co-simulation only)

- `stdin` -> one 4-byte sensor frame per tick: `SYNC (0xAA)`, `COUNTS_HI`,
  `COUNTS_LO` (a 12-bit raw ADC count, big-endian), `CHECKSUM` (`SYNC ^
  COUNTS_HI ^ COUNTS_LO`).
- `stdout` -> one line per update: `HEATER_ON`, `HEATER_OFF`, or `FAULT`,
  flushed immediately.
- EOF on `stdin` ends the loop (models the sensor link disappearing).
- A bad checksum, or a count pinned near either ADC rail (0 or 4095),
  latches the controller into `FAULT`.

## Build & test

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

`test_thermostat_logic` and `test_sensor_adc` each link their module
directly, without `hal_sim.c`, to prove neither has an I/O dependency.

## Running the simulator

`sim/room_simulator.py` models the room as a first-order RC thermal system
(Newton's law of cooling): it loses heat to the ambient temperature
proportionally to the temperature difference, and gains heat at a fixed
rate while the heater is on. It spawns `build/thermostat`, converts the
room temperature to a raw ADC count each tick, sends it as a framed byte
packet, reads back the status line, and steps the physics forward. Output
prints to the console and logs to a CSV file. Run `--help` for the full
flag list.

**Normal run**: room starts cool, converges into the setpoint band, and
oscillates there for the rest of the run:
```sh
python3 sim/room_simulator.py
```

**Undersized heater**: heater gain too weak to overcome the loss at this
ambient temperature, so the heater saturates fully ON and the room never
reaches setpoint (a real, useful failure mode to be able to see):
```sh
python3 sim/room_simulator.py --heater-gain 0.01 --ambient -10 --ticks 500
```

**Faster oscillation, shorter run**: a larger timestep makes the
hysteresis cycling visible in far fewer ticks:
```sh
python3 sim/room_simulator.py --ticks 100 --dt 10
```

**Sensor link glitch**: corrupts one frame's checksum mid-run; watch the
controller latch into `FAULT` and stay there even though later frames are
fine:
```sh
python3 sim/room_simulator.py --ticks 60 --fault-tick 20 --fault-mode checksum
```

**Disconnected sensor (open circuit)**: sends an ADC count pinned at the
low rail, the classic signature of an open sensor circuit:
```sh
python3 sim/room_simulator.py --ticks 60 --fault-tick 20 --fault-mode rail-low
```

**Shorted sensor**: same idea, pinned at the high rail:
```sh
python3 sim/room_simulator.py --ticks 60 --fault-tick 20 --fault-mode rail-high
```

In each fault scenario, note that the heater is forced off once `FAULT`
latches, so the room's logged temperature drifts back down toward ambient
for the rest of the run.
