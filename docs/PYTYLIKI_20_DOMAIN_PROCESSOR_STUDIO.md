# Pytyliki 20-Domain Processor Studio v1

**Status:** bounded C11 reference model and CLI implemented; this is not a physical processor controller.  
**Source:** src/processor_studio.h, src/processor_studio.c, src/processor_studio_cli.c  
**Contract:** pytyliki.reaper-processor-studio.v1  
**Report schema:** pytyliki.processor-studio-report.v1

## Purpose

The Studio turns the supplied 20-domain specimen into a deterministic, inspectable processor-engineering model. It preserves domain identities, seed values, transform order, source slot offsets, register reuse, matrix fold, table sweep, tournament, low-byte signature and planned jump target.

It is a reference execution engine over bounded C structures. It is not an assembler for arbitrary instructions, an emulator for a named ISA, a kernel, a driver or a hardware control plane.

## Twenty-domain catalogue

| ID | Environment | Engineering focus | Organization | Model layer |
|---:|---|---|---|---|
| 01 | Окружение | Машинно-ориентированная | Студия разработки процессоров | Workspace |
| 02 | Пространство | Аппаратно-ориентированная | Лаборатория процессорной инженерии | Hardware model |
| 03 | Платформа | Машинная | Центр проектирования процессоров | Architecture |
| 04 | Экосистема | Аппаратная | Мастерская процессорных технологий | Ecosystem |
| 05 | Инфраструктура | Нативная | Бюро разработки микропроцессоров | Toolchain |
| 06 | Оболочка | Ассемблерная | Институт процессорных систем | Assembly |
| 07 | Система | Процессорная | Комплекс процессорной разработки | System integration |
| 08 | Контекст | Микропроцессорная | Ателье микроархитектуры | Microarchitecture |
| 09 | Прослойка | Регистровая | Лаборатория проектирования CPU | Register model |
| 10 | Интерфейс | Битовая | Центр процессорного конструирования | Bit operations |
| 11 | Рабочее пространство | Инструкционная | Студия микроэлектронной инженерии | Instruction model |
| 12 | Исполнительная площадка | Адресная | Мастерская аппаратных архитектур | Address spaces |
| 13 | Технологическая база | Микроархитектурная | Центр разработки микропроцессорных систем | Technology model |
| 14 | Программная платформа | Системная | Лаборатория процессорного моделирования | Simulation |
| 15 | Операционная среда | Фундаментальная | Бюро архитектуры вычислительных систем | System architecture |
| 16 | Инструментальная система | Базовая | Студия аппаратного программирования | Tooling |
| 17 | Вычислительное окружение | Железно-ориентированная | Центр инженерии вычислительных ядер | Compute core |
| 18 | Функциональное пространство | Машинно-приближённая | Лаборатория цифровой логики | Digital logic |
| 19 | Аппаратно-программный комплекс | Аппаратно-зависимая | Мастерская разработки CPU | Hardware/software boundary |
| 20 | Среда выполнения | Близкая к машинному коду | Инжиниринговый центр процессорных технологий | Runtime |

The C11 catalogue exposes stable IDs and English report labels alongside each domain's focus, organization and layer. The Russian names preserve the vocabulary in the original fragment.

## Register and transform semantics

The modeled register file is R0–R7, regit, PC and FLAGS. Initial state is R0=1, R1=20, regit=0, PC=0. Arithmetic is unsigned 64-bit arithmetic with defined modulo-2^64 behavior.

| IDs | Operation |
|---|---|
| 01, 04, 07, 10, 13, 16, 19 | regit XOR seed |
| 03, 06, 09, 12, 15, 18 | wrapping unsigned addition into regit |
| 02, 05, 08, 11, 14, 17, 20 | rotate-left by 3, 5, 7, 9, 11, 13, 15 bits respectively, then XOR into regit |

The rotated values follow the source's scratch-register reuse: domain 02 writes R3, 05 writes R4, 08 writes R5, 11 writes R6, 14 writes R7, 17 overwrites R3, and 20 overwrites R4.

The post-domain operations are modeled in source order:

1. **Matrix fold P:** R5 = R3 + R4; regit XOR R5.
2. **Domain sweep:** XOR the accumulated value in each of the twenty state slots into regit. Unselected slots retain their last committed state; initially they are zero.
3. **Tournament:** round one stores max(R3, R4) in R6; round two writes max(R5, R6) to regit. A winner ID of zero denotes the synthetic P-fold candidate; IDs 17 and 20 denote direct R3/R4 candidates.
4. **Utility/control:** R1 = regit AND 0xFF; FLAGS records the non-zero predicate; R7 receives the saved virtual return PC. 0xC0000400 is emitted as a planned jump target only—the model never transfers control.

The report includes a low-byte signature and its bit parity. Parity is additional diagnostic metadata, not an ISA-specific flags encoding.

## Logical address layout

The fragment's address family is retained as integer metadata in a **logical model space**:

| Region | Base | Size |
|---|---:|---:|
| Arena | 0xC0000000 | 0x00400000 |
| Tray | 0xC0010000 | 0x00001000 |
| Overlay | 0xC0014000 | 0x00004000 |
| Domain table | 0xC0020000 | 20 × 64 bytes |

Original source offsets from 0x08 through 0xA0 are retained for trace/source correlation. The C state record contains more fields than one 64-bit source value, so its in-process array uses a 64-byte stride. Validation checks alignment, non-empty ranges, 64-bit overflow, containment and pairwise non-overlap. No address is dereferenced, mapped or passed to an operating-system memory API.

## Isolation, transactions and scheduling

The API requires explicit INSPECT, EXECUTE, PLAN, COMMIT and TRACE rights. The configured root-rights mask is a ceiling; a caller cannot grant itself a missing right. Unknown rights bits are rejected.

A run follows an optimistic transaction:

begin(expected_generation) → plan(mask) → execute(mask) → commit(expected_generation)

A stale generation, mismatched plan/execution mask, invalid domain mask or denied operation is rejected. Rollback discards pending register/domain snapshots. Commit publishes the pending state and increments the generation. Trace is a bounded 64-entry sequence-numbered ring; overflow drops the oldest event and increments a saturating counter.

The default scheduler is an advisory deterministic plan: twenty selected domains are assigned round-robin to four worker IDs (five domains per worker). AXI_BUS is only a retained queue label, not an instantiated bus. The library creates no threads, processes, DMA descriptors or hardware transactions.

## Build and run

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure

./build/pytyliki-studio --catalog
./build/pytyliki-studio --run
./build/pytyliki-studio --plan
./build/pytyliki-studio --json
~~~

On Windows, use the corresponding build/Release/pytyliki-studio.exe (or the selected CMake configuration directory).

## Public C API

processor_studio.h exposes the stable catalogue/default configuration, layout validation, capability-ceiling checks, generation-checked transactions, register/domain snapshots, caller-owned plan/trace copies and bounded JSON serialization. The module makes no heap allocations and retains no caller-owned pointer. The CLI uses fixed stack buffers.

The report is described by schemas/pytyliki.processor-studio-report.v1.schema.json.

## Reaper contract boundary

The report declares pytyliki.reaper-processor-studio.v1. Pytyliki owns the domain descriptors, processor-specific state transitions, source offsets, trace and scheduling recommendation.

Reaper remains the canonical owner of language semantics, Typed Registry, type/layout checks, canonical typed IR, Span, SourceMap, effects and ABI/lowering rules. A Reaper consumer must validate the report contract and reconstruct typed descriptors through Reaper's existing Typed Registry. The JSON contract does not itself constitute a compiler adapter, native ABI, authorization grant or hardware capability. Full source-to-Reaper-IR lowering remains a separate integration milestone.

## Safety and maturity boundary

Implemented by this change: bounded deterministic domain model, logical address validation, register-state simulation, optimistic transaction, advisory worker plan, trace ring, JSON schema/report, CLI and native tests.

Not implemented: arbitrary instruction decoding, execution of a target ISA, physical memory mapping, MMIO reads/writes, DMA submission, AXI bus traffic, thread creation, interrupt registration, privilege changes, firmware/microcode/BIOS writes, clock/voltage control, driver installation, code patching, RTL generation or synthesis.

A source-level verificated marker remains an untrusted source claim. It is not CI evidence, a cryptographic signature, review approval or hardware qualification.
