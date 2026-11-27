# Pytyliki: 100 low-level engineering rules

**Contract:** `pytyliki.low-level-rules.v1`  
**Implementation:** C11 target/memory/bus/IRQ contract plus existing memory, lifecycle, transaction and trace foundations.  
**Status:** mixed, evidence-backed maturity; not all 100 are shipped features.

## Interpretation

The source list combines engineering goals with metaphors and physically impossible claims. A syscall is not a voltage change, source text cannot grant privilege, a branch is not quantum superposition, reads cannot exceed light speed, a checksum is not a signature, and a software stop is not a physical circuit break.

| Status | Meaning |
|---|---|
| `implemented` | The stated bounded behavior exists in source and tests. |
| `partial` | A model or subset exists; full target semantics remain incomplete. |
| `target-specific` | Requires a concrete ISA, OS, board or trusted backend. |
| `not-physical` | The literal claim is metaphorical/impossible; only its valid engineering analogue is retained. |
| `planned` | A concrete requirement, not an implementation claim. |

## Implemented low-level C11 contract

The new `low_level_contract` module provides:

- `pl_ll_target`: ABI version, target ID, address and word widths, byte order, bus width, cache line, CPU count, clock envelope and supported power states.
- `pl_ll_memory_map`: fixed-capacity, non-overlapping regions with read/write/execute/device/DMA/cacheability flags and access alignment.
- `pl_ll_bus`: explicit backend access with separate RAM and MMIO capabilities, width/range/permission checks, device fences and audit callbacks.
- `pl_ll_bus_request_clock` and `pl_ll_bus_request_power_state`: capability-gated, target-bounded requests through trusted platform callbacks; no arbitrary voltage-write interface.
- `pl_ll_irq_registry`: fixed-capacity vector registry whose handlers declare bounded, non-blocking and no-allocation constraints.

No physical backend is shipped. A caller must supply target callbacks and capabilities. MMIO requires a device region, permissions, MMIO capability and backend; device access also requires a fence. W+X regions are rejected. This API does not map physical pages, install IRQs, switch CPU context, allocate DMA memory, load drivers or patch executable code.

## Existing foundations reused

| Subsystem | Existing implementation |
|---|---|
| Memory | Typed views, checked slices, arenas, pools and secure release |
| Embedded lifecycle | Boot/idle/active/degraded/halted, health gates, jitter, quotas and bounded trace |
| Embedded references | MCU profiles, FLASH/SRAM budgets and masked register bank |
| Runtime transactions | Staging, generation checks, commit, rollback and migration |
| Trace provenance | Units, windows, sample counts, uncertainty and source digest |
| Native ABI | Versioned C ABI and caller-owned buffers |

Existing code is evidence only for its actual behavior. It does not prove a physical kernel, production scheduler, full MMU, crypto suite, driver stack or DSP engine.

## The 100 rules

### 1–20. Foundation and physical model

| # | Source concept | Engineering implementation | Status |
|---:|---|---|---|
| 1 | begin iron ) | Reset vector, early CPU and memory initialization, and explicit readiness gates. | `target-specific` |
| 2 | () core -- | Versioned target profile declares ISA, address and word widths, endian, bus width and core count. | `implemented` |
| 3 | [ system ]- | System calls are not voltage writes; device, GPIO, clock and power are separate capabilities. | `not-physical` |
| 4 | stop option ; | Stop is a validated idle, degraded or halted lifecycle transition, not a physical circuit break. | `implemented` |
| 5 | ] level . | Privilege comes from CPU and OS state plus trusted capabilities, never source-selected atomic depth. | `target-specific` |
| 6 | { selective / | Branches are deterministic typed predicates; quantum superposition is not a CPU branch primitive. | `not-physical` |
| 7 | } operation P)- | Thermal settling is separate from operation completion and must use target telemetry. | `partial` |
| 8 | ?? massive <> | Use bounded arenas, pools, typed views and separately declared address spaces. | `implemented` |
| 9 | < start >? | Startup requires a declared entrypoint and CPU, memory, clock and interrupt health gates. | `implemented` |
| 10 | grown new-work > | Admit code as deterministic modules with budgets; live mutation is never implicit. | `planned` |
| 11 | > concrete //* | Every device address belongs to a non-overlapping region with permissions and alignment. | `implemented` |
| 12 | ## utility _ | Intrinsics and hardware utilities come from a target capability catalogue. | `partial` |
| 13 | # unify TAB | Synchronization uses memory-order rules and target fences, not electron-wide synchronization. | `implemented` |
| 14 | setup brachpass ^^ | Branch hints and predictor access are optional target features. | `target-specific` |
| 15 | !+ hyper tournament ^ | Scheduling uses bounded costs and deterministic tie-breaks; energy and latency remain separate. | `planned` |
| 16 | CC based rules ^^^ | Resource policy uses measured thermal/power telemetry and safe thresholds. | `partial` |
| 17 | level jump to run *( | Context/privilege transitions save state and delegate to OS or target ABI. | `target-specific` |
| 18 | TRUE connective *+ | Boolean conjunction remains typed logic; cross-core visibility uses synchronization, not entanglement. | `not-physical` |
| 19 | verison ///. * | Language, AST, ABI, target and artifact versions are separate identities. | `implemented` |
| 20 | verificated /// !! | Checksums detect corruption; authenticity requires signatures and trusted attestation. | `partial` |

### 21–40. Memory and resources

| # | Source concept | Engineering implementation | Status |
|---:|---|---|---|
| 21 | Memory reservation | Allocation uses explicit allocators, pools and quotas with overflow checks. | `implemented` |
| 22 | Pointers as coordinates | Pointers carry bounded extent, type, alignment, permissions and provenance. | `partial` |
| 23 | No garbage collector | Low-level lifetimes use explicit release, arenas and pools. | `implemented` |
| 24 | Stack grows down | Stack direction, bounds, guards, alignment and reserve belong to target/linker contract. | `target-specific` |
| 25 | Cache is silicon memory | Cache line and cacheability are target metadata; maintenance operations need backends. | `partial` |
| 26 | Registers hold state | Registers require typed width, reset, masks and side-effect policy. | `partial` |
| 27 | Bus is nervous system | Bus access checks width, alignment, region and capability through trusted backend. | `implemented` |
| 28 | Interrupts are pain | Fixed IRQ dispatch requires bounded, non-blocking, no-allocation handler contracts. | `implemented` |
| 29 | Clock is heartbeat | Clock is target metadata; jitter is measured with units and wrap bounds. | `partial` |
| 30 | Energy is souls | Energy uses physical units, sensor provenance and uncertainty; no soul metric exists. | `not-physical` |
| 31 | Heat is code sin | Temperature is sensor telemetry, not inferred from code style. | `partial` |
| 32 | Bits are particles | Bit operations use fixed-width types, masks and defined overflow semantics. | `implemented` |
| 33 | Byte is molecule | Serialized bytes are octets with explicit lengths and ABI encoding. | `implemented` |
| 34 | Word is atom | Target word width is explicit, not inferred from host compiler. | `implemented` |
| 35 | Addresses to infinity | Address width, region ends, spans and arithmetic overflow are checked. | `implemented` |
| 36 | No virtual memory | Virtual and physical addresses are distinct; MMU translation is target-specific. | `planned` |
| 37 | Magnetic write protection | Region and MPU/MMU permissions protect writes; W+X is rejected by default. | `implemented` |
| 38 | Read faster than light | Latency is measured; caches, wait states and uncertainty are recorded. | `not-physical` |
| 39 | Writes change history | Bounded trace records transitions and reports overflow; no cosmic persistence is implied. | `implemented` |
| 40 | Clear reality | Secure wipe applies only to allocator-owned memory and tracked high-water ranges. | `implemented` |

### 41–60. Logic and execution

| # | Source concept | Engineering implementation | Status |
|---:|---|---|---|
| 41 | Conditions test fate | Conditions are typed booleans with explicit evaluation and error semantics. | `partial` |
| 42 | Loops reverse time | Loops use bounded iterations, deadlines, cancellation and wrap-safe timers. | `implemented` |
| 43 | Functions are rituals | Calls specify argument/return layout, clobbers, stack alignment and ABI version. | `implemented` |
| 44 | Recursion mirrors | Recursion requires a bounded depth and stack budget on finite-stack targets. | `planned` |
| 45 | Parallel worlds | Parallel tasks need bounded admission, dependencies and synchronization. | `partial` |
| 46 | Locks bind chaos | Lock contracts define owner, ordering and timeout; ISR paths must not block. | `planned` |
| 47 | Mutexes unlock doors | Mutexes define owner, acquire/release, recursion and priority-inheritance policy. | `planned` |
| 48 | Semaphores are signals | Counting semaphores define bounded counts, wake order and overflow behavior. | `planned` |
| 49 | Queues wait forever | Queues define fixed capacity, ownership, backpressure, ordering and loss policy. | `partial` |
| 50 | Priority weighs souls | Priority is numeric policy with fairness and starvation rules. | `planned` |
| 51 | Scheduler is god | Hard real-time claims require WCET, interrupt and schedulability evidence. | `planned` |
| 52 | Context in ether | A context snapshot names registers, flags, PC, SP and ownership. | `target-specific` |
| 53 | Task teleportation | Context switching belongs to an OS/target backend with prescribed save/restore order. | `target-specific` |
| 54 | Stream is Ariadne thread | Streams specify element type, capacity, backpressure, cancellation and errors. | `partial` |
| 55 | Process is universe | Isolation requires address spaces, privilege and quotas from OS/MMU/MPU. | `target-specific` |
| 56 | Daemon is background spirit | Background services have explicit lifecycle, health, cancellation and quotas. | `planned` |
| 57 | Service worship | Services expose versioned request/response, timeout, health and failure contracts. | `partial` |
| 58 | Driver translator | Drivers are explicit target adapters; discovery never loads drivers implicitly. | `planned` |
| 59 | Kernel heart | The embedded lifecycle model is a reference model, not a production kernel. | `implemented` |
| 60 | Shell skin | CLI and imported files are untrusted; parsing is bounded and inspection never executes source. | `implemented` |

### 61–80. Security and integrity

| # | Source concept | Engineering implementation | Status |
|---:|---|---|---|
| 61 | Encryption is noise | Use vetted AEAD with unique nonces, opaque key handles and authenticated metadata. | `planned` |
| 62 | Key is author DNA | Secrets are provider-owned handles, never author identity or source literals. | `planned` |
| 63 | Hash is fingerprint | A digest identifies bytes; it is not a publisher signature. | `partial` |
| 64 | Signature is oath | Artifacts require trusted signing providers, canonical bytes and trust-anchor validation. | `planned` |
| 65 | Certificate is indulgence | Certificate validation checks issuer, chain, use, expiry and revocation. | `planned` |
| 66 | Firewall wall | Network filters use default-deny rules with direction, protocol, state and audit. | `planned` |
| 67 | Virus is design error | Imported code and binaries remain data; static scanning never executes samples. | `partial` |
| 68 | Antivirus immunity | Scanning is one signal; use least privilege, sandboxing, signatures and recovery. | `planned` |
| 69 | Exploit cracks armor | Use W^X, bounds checks, stack guards, parser limits and least privilege. | `partial` |
| 70 | Vulnerability weakness | Records include affected versions, preconditions, impact, remediation and tests. | `planned` |
| 71 | Patch plaster | Data updates use staging, validation, generation checks, commit and rollback. | `implemented` |
| 72 | Update evolution | Schema migrations are versioned and reversible with compatibility checks. | `implemented` |
| 73 | Bug is design | Defects become reproducible diagnostics and regression tests. | `implemented` |
| 74 | Feature is law | Features have stable IDs, maturity, limits and source/test evidence. | `implemented` |
| 75 | Docs are mythology | Capabilities link authoritative code, build steps, examples and maturity. | `implemented` |
| 76 | Comments are whispers | Comments explain intent, constraints and trade-offs, not obvious code. | `implemented` |
| 77 | Logs are chronicles | Logs and traces are bounded, structured, report loss and exclude secrets. | `partial` |
| 78 | Debugging confession | Debug reads are separately authorized and read-only by default. | `partial` |
| 79 | Testing is judgment | CTest, native tests, malformed cases and sanitizers verify contracts; hardware needs HIL. | `implemented` |
| 80 | Deploy new life | Releases use reproducible builds, provenance, signatures and rollback; no firmware deploy claimed. | `planned` |

### 81–100. Aesthetics and performance

| # | Source concept | Engineering implementation | Status |
|---:|---|---|---|
| 81 | Sharp transitions | State transitions are explicit, validated and return typed errors. | `implemented` |
| 82 | Immaculate mastering | Changed code follows style, warnings, tests, docs and reviewed diffs. | `implemented` |
| 83 | No intro build | Startup removes needless work but never skips initialization or readiness gates. | `partial` |
| 84 | Layered slices | Separate syntax, typed contract, Hardware IR, target backend, tooling and runtime. | `partial` |
| 85 | Thunderous syncopation | Jitter, cadence, throughput and latency are measured with units, windows and uncertainty. | `implemented` |
| 86 | Hidden gems | Optimizations need baseline data and regression checks; no hidden self-modifying transform. | `partial` |
| 87 | Punchy mix | Performance is measured alongside correctness, memory, energy and thermal constraints. | `partial` |
| 88 | Stereo width | Channels are independent typed buffers/streams; no native stereo DSP is claimed. | `planned` |
| 89 | Dynamic range | Numeric paths define format, saturation, overflow, quantization and headroom. | `partial` |
| 90 | Frequency response | Frequency response is a DSP characteristic requiring measured audio tests. | `planned` |
| 91 | Distortion | Nonlinear transforms require explicit curves, limits and reference tests. | `planned` |
| 92 | Compression | Compression declares output capacity, expansion limit and malformed-input behavior. | `planned` |
| 93 | Limiting | Workers, memory, queues, recursion, samples and runtime are bounded by quotas. | `implemented` |
| 94 | EQ | Profiles expose latency, throughput, energy, thermal and reliability trade-offs. | `partial` |
| 95 | Reverb | Audio reverb is separate from trace history; replay needs bounded provenance. | `partial` |
| 96 | Delay | Delay uses monotonic deadlines, bounded waits and cancellation. | `partial` |
| 97 | Chorus | Fan-out duplicates immutable events to bounded consumers with backpressure. | `planned` |
| 98 | Flanger | A flanger requires a bounded delay line, interpolation and modulation policy. | `planned` |
| 99 | Phaser | Phase operations declare numeric format, accumulator bounds and measured response. | `planned` |
| 100 | Silence | Avoid needless I/O, allocation and synchronization; no-op elimination preserves behavior. | `partial` |

## Safety and ABI invariants

1. Source declarations never grant MMIO, DMA, debug, clock, power, privilege or code-patch capabilities.
2. An access must fit one declared region and respect its permissions, transfer width and alignment.
3. Device I/O is routed through an explicitly supplied backend; MMIO requires backend fences.
4. Clock and power requests require explicit capabilities and target-supported envelopes. There is no raw voltage/pin-control API.
5. IRQ dispatch uses a fixed-capacity table. Handler flags are reviewable contracts, not automatic proof of WCET or ISR safety.
6. W+X regions are rejected by default; executable patching is outside this API.
7. Backend callbacks exchange width-limited values in the target's declared byte order.
8. If a post-write MMIO fence fails, the device may already have received the write. Treat completion as uncertain and use target-specific recovery.

## Verification

The native test uses a deterministic fake backend, not physical hardware. It checks target validation, memory-map boundaries, overlap and W+X rejection, read-only regions, alignment, width limits, separate RAM/MMIO capabilities, device fences, clock/power limits, audit callbacks and IRQ contracts.

A passing fake-backend test does not qualify a physical board; each target needs datasheet review, adapter-specific fault injection and hardware-in-the-loop tests.