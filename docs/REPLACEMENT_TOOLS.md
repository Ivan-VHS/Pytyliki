# Pytyliki Watch — integrated host telemetry

Pytyliki Watch is the read-only telemetry and trend-analysis workbench in the Pytyliki processor-systems workstation. It has three integrated stages:

1. Collection — platform adapters read OS counters without changing processor, scheduler, firmware or cooling state.
2. Analysis — counter deltas, memory pressure, swap pressure, load averages and configurable thresholds are normalized into a stable report model.
3. Presentation — table output is intended for operators, JSONL/CSV for pipelines, and the versioned analysis report for archiving and later comparison.

Examples:

    python tools/pytyliki_watch.py --count 10 --interval 1
    python tools/pytyliki_watch.py --count 60 --interval 1 --format jsonl --output reports/session.jsonl --report reports/session-summary.json
    python tools/pytyliki_watch.py --cpu-limit 85 --memory-limit 88 --swap-limit 20 --report reports/policy.json

The report schema is pytyliki.telemetry-analysis.v1. It includes a non-identifying host profile ID, sample statistics, p95 estimates, configurable threshold alerts and explicit collection limits. The profile ID is a hash of platform and processor-class metadata; it does not include the account name, hostname or network addresses.

Linux reads /proc/stat, /proc/meminfo, /proc/loadavg and process-directory entries. Windows reads system CPU times and memory counters through Win32. The first sample has no CPU percentage because a delta is not yet available. Per-process accounting, hardware sensor access and fan/clock controls are intentionally outside this read-only application.

This is an original Pytyliki tool, not a clone of any hardware-monitoring product. It contains no third-party source, product assets or copied interface. Copyright (c) 2026 Ivan-VHS. The repository MIT LICENSE remains authoritative.

Run tests:

    python -m unittest discover -s tests -p "test_pytyliki_watch.py" -v
