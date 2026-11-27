# OCTA-8 + CLife federation

Pytyliki remains a native C11 processor and measurement core. The shared `ivan-vhs.octa8.v1` JSON envelope defines eight kinds: null, boolean, signed 64-bit integer, finite number, UTF-8 text, bytes, sequence and record. `src/octa8.h` provides the stable C11 kind identifiers and lightweight value-view checks without changing the existing ABI v1.

## CLife

Run `python tools/clife.py snapshot .`, then `verify ARCHIVE`; recovery uses `restore ARCHIVE --to NEW_DIRECTORY`. Snapshots are bounded, exclude build/VCS/cache output and carry a per-file SHA-256 manifest. Restore refuses to overwrite or merge into an existing directory.

The C11 core retains ownership of processor inventory, numeric kernels, memory and DSP. Reaper may orchestrate using the JSON/ABI boundary; it does not replace C code or silently load libraries.
