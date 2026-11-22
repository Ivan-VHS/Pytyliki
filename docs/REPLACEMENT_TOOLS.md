# Original replacement tools

The new tools/pytyliki_watch.py utility is an independently written, read-only host monitor. It reports CPU load from counter deltas, physical memory, swap, Linux interrupt totals, logical CPU count and process count where the platform exposes them.

Run:

    python tools/pytyliki_watch.py --count 5 --interval 1 --format jsonl --output reports/host.jsonl

The monitor uses Python's standard library only. Linux data comes from /proc; Windows data comes from read-only Win32 counter APIs. The first CPU sample has no utilization percentage because a delta is not yet available. Interrupt totals and process count may be unavailable on Windows. Other operating systems return an explicit unsupported-platform error.

This is not a clone of Intel XTU, Ryzen Master, HWiNFO, htop, btop or another product. It does not change clock speeds, voltage, fan curves, affinity, scheduler state or firmware. No third-party source code, assets, interface layout or trademarks are included.

The new source and tests are original Pytyliki contributions, copyright (c) 2026 Ivan-VHS, under the repository's MIT License. The existing LICENSE file remains authoritative.

Test:

    python -m unittest discover -s tests -p "test_pytyliki_watch.py" -v
