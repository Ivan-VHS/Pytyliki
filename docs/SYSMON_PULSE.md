# sysmon-pulse

`sysmon-pulse` is a read-only Linux terminal monitor for aggregate CPU activity,
RAM, swap and interrupt counts. It reads kernel counters from `/proc`; it does
not require elevated privileges and does not write to the machine.

## Run

```sh
python tools/sysmon_pulse.py
python tools/sysmon_pulse.py --once
python tools/sysmon_pulse.py --interval 2 --no-color
python tools/sysmon_pulse.py --once --json
```

CPU utilisation is calculated from two successive aggregate `/proc/stat`
samples. The first sample therefore reports “warming up”. RAM and swap display
used and total capacity. Interrupt totals and the five busiest labels are
reported when `/proc/interrupts` is readable.

The monitor is Linux-specific. The parser functions accept fixture text so the
formatting and arithmetic can be tested without a live host.
