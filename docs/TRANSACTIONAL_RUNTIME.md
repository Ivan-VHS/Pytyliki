# Transactional Runtime Contract v1

Pytyliki exposes a bounded C11 transaction engine through `src/runtime_transaction.h`.

## Scope and safety

The engine stages and commits **in-memory byte snapshots** used by reports, plans, fixtures or other caller-owned data. It does not write processor firmware, ROM, MMIO registers, drivers, calibration tables, or physical hardware. The word `flash` means “commit the staged snapshot”; it is not a hardware flash operation.

## Operations

| Operation | Contract |
|---|---|
| `patch` | Replace/insert/remove a bounded byte range in staged data |
| `rebind` | Change a symbolic 64-bit binding |
| `remap` | Build a new byte sequence from validated source indexes |
| `hotSwap` | Stage a complete replacement payload |
| `migrate` | Run an explicit caller-supplied schema migration; output is copied before return |
| `flash` / `reflash` | Commit only if the expected generation matches |
| `rollback` | Restore staged bytes from the last committed snapshot |
| `sync` | Commit against the current generation |
| `emit` / `yield` | Send structured event/checkpoint callbacks |
| `throw` / `catch` | Record a recoverable error and copy/clear it through the catch API |
| `panic` / `assert` | Mark the transaction failed; no process abort is performed |
| `return` | Propagate a `pl_rt_status` to the caller |
| `async` / `await` | Create a cooperative deferred task; the commit occurs when awaited, without creating a thread |

## Integrity and bounds

- Maximum staged payload: 16 MiB.
- CRC32 is checked again at commit.
- Optimistic generation checks reject stale commits with `PL_RT_CONFLICT`.
- Failed transactions cannot commit until the error is caught or the staged image is rolled back.
- The caller owns the transaction object and must call `pl_rt_destroy`.

## Build verification

The module is part of `pytyliki_core`; CTest target: `runtime-transaction-contract-tests`.
Run the repository's standard CMake configure/build/CTest sequence.
