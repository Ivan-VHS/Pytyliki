# CLife — checkpoint and recovery

CLife v1 adds a portable lifecycle layer for source trees, diagnostic fixtures and workstation configuration files.

## Software mapping

| Simulation concept | Pytyliki implementation |
|---|---|
| Cryo fault | interrupted write, missing file, size drift or checksum mismatch |
| Consciousness snapshot | immutable archive manifest and SHA-256 file fingerprints |
| Coherence / integrity | verified file count, total bytes and per-file digest matches |
| Stasis hold | reject recovery when the archive or target path is unsafe |
| Awakening | restore a verified copy into a new directory |

No CPU registers, live process memory, voltage, clock, affinity or operating-system scheduler state is captured or changed. The quantum terminology in the supplied concept is retained only as a metaphor; CLife is conventional local file I/O and hashing.

## Commands

Run from the repository root with Python 3.10+:

```sh
python tools/clife.py snapshot .
python tools/clife.py inspect .clife/snapshots/<snapshot>.clife
python tools/clife.py verify .clife/snapshots/<snapshot>.clife
python tools/clife.py restore .clife/snapshots/<snapshot>.clife --to ../pytyliki-recovered
python -m unittest discover -s tests -p "test_clife.py" -v
```

The default archive location is .clife/snapshots/. .git, virtual environments, build trees, distribution folders, dependency caches and .clife itself are excluded. Symlinks are not followed.

## Safety and limits

- SHA-256 is checked for every file during verification and again during restore.
- Limits: 20,000 files, 128 MiB per file and 1 GiB total uncompressed data.
- Restore creates a new directory and refuses to overwrite an existing path.
- Only regular files are archived; permissions are restored without setuid/setgid bits.
- This is a source/configuration checkpoint, not a disk image or hardware-state backup.
- Do not generate the proposed 666,666,666-character stream as a backup. CLife stores only actual project files and a compact manifest.
