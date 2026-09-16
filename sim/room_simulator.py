#!/usr/bin/env python3
"""
Room thermal co-simulator for the embedded thermostat.

Models the room as a first-order RC thermal system (Newton's law of
cooling): the room loses heat to the ambient environment proportional to
the temperature difference, and gains heat at a fixed rate while the
heater is on.

    T[n+1] = T[n] + dt * (heater_gain * heater_on - loss_coeff * (T[n] - T_ambient))

Each tick, the current room temperature is converted to a raw 12-bit ADC
count and sent to the thermostat binary as a 4-byte framed sensor packet
(SYNC, COUNTS_HI, COUNTS_LO, CHECKSUM) on stdin; the resulting
HEATER_ON/HEATER_OFF/FAULT status is read back as a text line from its
stdout. See the wire protocol documented in include/hal.h.

--fault-tick optionally corrupts a single tick's frame to exercise the
controller's latched fault path -- see --fault-mode for the three ways a
real sensor link can fail that this simulates.
"""
import argparse
import csv
import struct
import subprocess
import sys
from pathlib import Path

# Must stay in sync with include/sensor_adc.h and include/hal.h.
FRAME_SYNC = 0xAA
ADC_COUNTS_MAX = 4095
ADC_RAIL_GUARD_COUNTS = 16
SENSOR_MIN_C = -40.0
SENSOR_MAX_C = 85.0


def celsius_to_counts(temp_c):
    """Inverse of SensorAdc_ConvertToCelsius() in src/sensor_adc.c."""
    span_counts = ADC_COUNTS_MAX - 2 * ADC_RAIL_GUARD_COUNTS
    frac = (temp_c - SENSOR_MIN_C) / (SENSOR_MAX_C - SENSOR_MIN_C)
    counts = ADC_RAIL_GUARD_COUNTS + frac * span_counts
    # Clamp to the valid (non-fault) span so normal physics never
    # accidentally trips a rail fault on its own.
    counts = max(ADC_RAIL_GUARD_COUNTS + 1, min(ADC_COUNTS_MAX - ADC_RAIL_GUARD_COUNTS - 1, round(counts)))
    return counts


def build_frame(counts, corrupt_checksum=False):
    hi = (counts >> 8) & 0xFF
    lo = counts & 0xFF
    checksum = FRAME_SYNC ^ hi ^ lo
    if corrupt_checksum:
        checksum ^= 0xFF
    return struct.pack(">BHB", FRAME_SYNC, counts, checksum)


def parse_args():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
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
        help="Tick at which to inject a sensor fault via --fault-mode (-1 disables)",
    )
    parser.add_argument(
        "--fault-mode",
        choices=["checksum", "rail-low", "rail-high"],
        default="checksum",
        help=(
            "checksum: corrupt one frame's checksum (link glitch); "
            "rail-low/rail-high: send an ADC count pinned at a rail "
            "(disconnected/shorted sensor)"
        ),
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
        bufsize=0,
    )

    temp = args.start_temp
    rows = []
    print(f"{'tick':>5} {'temp_C':>8} {'status':>10}")
    try:
        for tick in range(args.ticks):
            if tick == args.fault_tick:
                if args.fault_mode == "checksum":
                    frame = build_frame(celsius_to_counts(temp), corrupt_checksum=True)
                elif args.fault_mode == "rail-low":
                    frame = build_frame(0)
                else:  # rail-high
                    frame = build_frame(ADC_COUNTS_MAX)
            else:
                frame = build_frame(celsius_to_counts(temp))

            proc.stdin.write(frame)
            proc.stdin.flush()

            response = proc.stdout.readline()
            if not response:
                print("thermostat process closed its output early", file=sys.stderr)
                break
            status = response.decode("ascii").strip()
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
