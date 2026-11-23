# XP-era game session telemetry

**Project author/developer:** Ivan-VHS

Pytyliki Watch can now turn its existing read-only CPU, RAM and swap snapshots into a bounded game-session report. This complements the XP SP3 launch profile by showing whole-host resource pressure during loading and play.

```sh
python tools/pytyliki_watch.py --interval 1 --count 60 --format jsonl --output session.jsonl
python tools/game_session_report.py session.jsonl --game-id sample-game --output report.json
```

The report accepts up to 10,000 snapshot records and reports min/median/p95/max CPU, memory and swap utilization, plus the minimum available physical memory. It intentionally marks its scope as `whole_host`: this version does not attribute counters to a particular game process or claim to reproduce XP's 32-bit address-space, affinity, DEP or timer semantics. It never launches the game or changes host settings.
