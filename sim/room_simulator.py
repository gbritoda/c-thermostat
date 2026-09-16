#!/usr/bin/env python3
"""
Room thermal co-simulator for the embedded thermostat.

Models the room as a first-order RC thermal system (Newton's law of
cooling): the room loses heat to the ambient environment proportional to
the temperature difference, and gains heat at a fixed rate while the
heater is on.

    T[n+1] = T[n] + dt * (heater_gain * heater_on - loss_coeff * (T[n] - T_ambient))

Each tick, the current room temperature is written to the thermostat
binary's stdin and the resulting HEATER_ON/HEATER_OFF/FAULT status is read
back from its stdout, per the wire protocol documented in include/hal.h.

--fault-tick optionally injects a single out-of-range sensor glitch (a
garbled reading, not a change to the room's real temperature) to exercise
the controller's latched fault path.
"""
import argparse
import csv
import subprocess
import sys
from pathlib import Path


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--binary",
        default=str(Path(__file__).resolve().parent.parent / "build" / "thermostat"),
        help="Path to the compiled thermostat binary",
    )
    parser.add_argument("--ticks", type=int, default=300, help="Number of simulation ticks")
    parser.add_argument("--dt", type=float, default=5.0, help="Seconds simulated per tick")
    parser.add_argument("--ambient", type=float, default=5.0, help="Ambient outdoor temperature, C")
    parser.add_argument("--start-temp", type=float, default=18.0, help="Initial room temperature, C")
    parser.add_argument("--heater-gain", type=float, default=0.05, help="Heater warming rate, C/s, while on")
    parser.add_argument("--loss-coeff", type=float, default=0.002, help="Heat loss coefficient, 1/s")
    parser.add_argument(
        "--log",
        default=str(Path(__file__).resolve().parent / "room_log.csv"),
        help="CSV file to write the tick-by-tick log to",
    )
    parser.add_argument(
        "--fault-tick",
        type=int,
        default=-1,
        help="Tick at which to inject a single out-of-range sensor glitch (-1 disables)",
    )
    parser.add_argument(
        "--fault-value",
        type=float,
        default=999.0,
        help="Out-of-range reading to send at --fault-tick",
    )
    return parser.parse_args()


def main():
    args = parse_args()
    binary_path = Path(args.binary)
    if not binary_path.exists():
        sys.exit(f"thermostat binary not found at {binary_path} -- build it first (cmake --build build)")

    proc = subprocess.Popen(
        [str(binary_path)],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        text=True,
        bufsize=1,  # line buffered
    )

    temp = args.start_temp
    rows = []
    print(f"{'tick':>5} {'temp_C':>8} {'status':>10}")
    try:
        for tick in range(args.ticks):
            # A fault-injected reading is a garbled sample from the sensor,
            # not a real change in room temperature -- the physics below
            # always advances from the true `temp`, never from this.
            reported_temp = args.fault_value if tick == args.fault_tick else temp
            proc.stdin.write(f"{reported_temp:.4f}\n")
            proc.stdin.flush()

            response = proc.stdout.readline()
            if not response:
                print("thermostat process closed its output early", file=sys.stderr)
                break
            status = response.strip()
            heater_on = status == "HEATER_ON"

            print(f"{tick:5d} {temp:8.3f} {status:>10}")
            rows.append((tick, f"{temp:.4f}", status))

            gain = args.heater_gain if heater_on else 0.0
            loss = args.loss_coeff * (temp - args.ambient)
            temp += args.dt * (gain - loss)
    finally:
        proc.stdin.close()
        exit_code = proc.wait(timeout=5)

    with open(args.log, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["tick", "temp_C", "status"])
        writer.writerows(rows)

    print(f"\nSimulation done. thermostat exited with code {exit_code}. Log written to {args.log}")


if __name__ == "__main__":
    main()
