# Pytyliki: процессорное ядро Reaper на реальном кремнии

> Статус реализации: **C11 silicon-runtime v1**. Аппаратные пути включаются явно и только для проверенной платы. Host-тесты используют HAL-симулятор. Документ не утверждает, что Pytyliki уже запускалась на конкретном MCU или что универсальная карта MMIO существует.

[English overview](EMBEDDED_SILICON.md) · [Embedded C workbench](EMBEDDED_C.md)

## 1. Что реализовано

В `src/silicon_runtime.h` и `src/silicon_runtime.c` добавлен отдельный C11 runtime с версионированным ABI. Он связывает портрет процессора, аппаратный HAL, MMIO allowlist, счётчик циклов, state machine, fault trace, квоты потоков, детерминированный выбор маршрута, CRC-проверку patch-слотов, экспорт таблиц, fixed-point арифметику и верификацию машинных потоков.

| Уровень | Реализация v1 | Граница |
|---|---|---|
| Cortex-M | DMB/DSB/ISB, WFI, опциональный DWT CYCCNT | DWT включается только при `PL_SILICON_ENABLE_CORTEX_M_DWT` и сборке под M-profile |
| RISC-V | `fence`, опциональный `fence.i`, `rdcycle` / согласованная пара `rdcycleh` | доступ к CSR cycle может быть запрещён режимом привилегий/прошивкой |
| x86/x86_64 | `mfence`, сериализация через CPUID, LFENCE/RDTSC | TSC — сырые тики TSC, не гарантированные core cycles |
| MMIO | 8/16/32-битные volatile access callbacks, exact-address allowlist, маски записи и барьеры | карта регистров принадлежит конкретной плате/SoC; универсальной карты нет |
| Машинные потоки | whitelist verifier для Thumb-16 и RV32I, RPR32 bounded interpreter | native/JIT trampoline запрещены; x86-декодер пока не реализован |
| Faults | атомарная SPSC-кольцевая очередь и portrait flags | несколько одновременных producers требуют отдельного MPSC/IRQ-port решения |
| Patch | staging и CRC-16 verification | нет исполнения патча, записи Flash, изменения vector table или cache maintenance |

## 2. Сборка и smoke-check

Обычная сборка Pytyliki остаётся переносимым C11 приложением:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Проверка профиля собранной архитектуры не обращается к периферии:

```sh
./build/pytyliki --embedded silicon
./build/pytyliki --embedded profiles
./build/pytyliki --embedded profile cortex-m4-reference
```

`--embedded silicon` показывает архитектуру, под которую собран бинарный файл, и доступность backend по compile-time условиям. Это не обнаружение подключённой платы, не чтение CPUID/CSR и не тест физического MMIO.

## 3. Архитектура: HAL вместо выдуманной универсальной платы

`pl_silicon_runtime` содержит `pl_silicon_config`, `pl_silicon_hal` и `pl_silicon_portrait`. HAL — набор callback-ов для чтения циклов, чтения/записи MMIO, барьеров и ожидания прерывания. Так логика тестируется на host без разыменования аппаратных адресов.

Для конкретной платы приложение обязано:

1. Выбрать точный MCU/SoC, stepping, memory map и errata по документации производителя.
2. Инициализировать clocks, power domains, MPU/MMU и startup/vector table средствами BSP/startup-кода платы.
3. Создать HAL и заполнить конфигурацию фактической частотой/шириной цикломера. `cycle_hz=0` допустим, если частота не откалибрована; в этом случае перевод циклов в секунды делать нельзя.
4. Зарегистрировать каждый допустимый MMIO-регистр по имени, точному адресу, ширине, правам, writable mask и требуемому барьеру.
5. Проверить доступность атомарных операций из ISR и убедиться в отсутствии вложенных producers у SPSC-кольца.
6. Пройти аппаратные smoke-тесты на целевой плате и проверить watchdog, fault handlers, linker map, stack/heap и startup.

Базовая библиотека не угадывает адрес GPIO/RCC/SCB/PLIC/CLINT, не переключает clocks, не пишет Flash и не устанавливает firmware или драйверы.

## 4. Архитектурные барьеры

`pl_silicon_native_barrier()` реализует архитектурные барьеры там, где доступен compiler/ISA backend:

- ARM: `dmb sy`, `dsb sy`, `isb sy`.
- RISC-V: `fence rw,rw`, `fence iorw,iorw`; `fence.i` только если сборка заявляет `__riscv_zifencei`.
- x86/x86_64: `mfence` для memory ordering; CPUID serializing sequence для ISB-подобной границы.

Это не взаимозаменяемые понятия. DMB/FENCE задают порядок наблюдения операций, DSB ждёт завершения операций, ISB/fence.i синхронизирует последующее выполнение инструкций. Выбор барьера должен соответствовать периферийному протоколу и ISA manual. Если backend не поддерживается, функция возвращает `PL_SILICON_EUNSUPPORTED`, а не подменяет барьер пустой инструкцией.

## 5. Реальные счётчики циклов

`pl_silicon_native_cycle_read()` содержит compile-time backend:

- Cortex-M: DWT CYCCNT, только при `PL_SILICON_ENABLE_CORTEX_M_DWT`.
- RISC-V: `rdcycle`; на RV32 читаются high/low/high до совпадения верхней половины.
- x86/x86_64: `lfence; rdtsc; lfence` или MSVC intrinsics.

На Cortex-M `pl_silicon_cortex_m_dwt_enable()` проверяет `NOCYCCNT`, устанавливает `TRCENA`, очищает CYCCNT и включает счётчик с DSB/ISB. Регистр может быть заблокирован отладочной конфигурацией или отсутствовать у конкретного ядра.

На x86 TSC не следует называть тактами ядра без доказанной связи с частотой core. TSC часто работает от постоянной reference frequency и может отличаться от частоты исполнения. Учитывайте invariant TSC, миграцию между CPU, виртуализацию и калибровку. На RISC-V `cycle` может быть отключён политикой привилегий. Для точного профилирования используйте документированный PMU/таймер SoC и его правила доступа.

`pl_silicon_cycle_engine` считает unsigned-разность с учётом переполнения счётчика шириной 16/32/64 бит. Между двумя выборками не должно пройти больше одного полного периода счётчика; иначе число оборотов не восстановить из одной пары значений. Slow-motion — это decimation логических trace-шагов, а не изменение частоты CPU.

## 6. MMIO и права записи

MMIO-дескриптор задаёт точное имя и адрес. Банк отклоняет неправильное выравнивание, пересекающиеся диапазоны, неизвестную ширину, дубликаты, неподдерживаемые барьеры и некорректную writable mask. Чтение/запись возможны только при наличии callback-а HAL и подходящего права.

`volatile` сообщает компилятору о наблюдаемом доступе, но сам по себе не является межъядерным барьером, не создаёт атомарности и не доказывает, что адрес принадлежит периферии. Поэтому volatile-разыменование находится внутри opt-in backend, а прикладная логика проходит через allowlist.

`PL_SILICON_ENABLE_DIRECT_MMIO` включает native volatile callback-ы. Не включайте его в обычную Windows/Linux workstation-сборку. Для firmware-сборки это лишь низкоуровневый доступ: всё равно нужны точная SoC map, MPU/MMU permissions, доступ к шине, read/write side effects и review каждого регистра. Регистры W1C, read-to-clear, FIFO и key sequences требуют специальных API вместо общего read-modify-write.

## 7. Состояния, faults и «боль» процессора

Портрет содержит архитектуру, версию ABI, CPUID/MARCHID, состояние, последнюю отметку циклов, текущую/пиковую боль и fault flags. State program разрешает переход только при совпадении исходного состояния, требуемых/запрещённых fault bits, порога боли и условий готовности.

`STANDBY_WFI` и `SAFE_HALT` не исполняются автоматически при изменении состояния. Только явный вызов `pl_silicon_wait()` вызывает HAL wait callback и только после перехода в `STANDBY_WFI`. `SAFE_HALT` требует `UNRECOVERABLE`; на x86 native `HLT` намеренно не предоставлен. Fault handlers должны сначала зафиксировать минимальный контекст и обеспечить watchdog/recovery policy; тяжёлые логи и форматирование нельзя делать в ISR.

`pl_silicon_fault_ring` — bounded SPSC очередь: один producer и один consumer. При заполнении событие отбрасывается и увеличивается `dropped`; ISR не ждёт освобождения места. Если fault могут писать несколько IRQ/core, необходима MPSC-очередь или критическая секция, проверенная для целевой платформы. Перед ISR-применением проверьте `atomic_is_lock_free()` для атомиков и ABI компилятора. `pl_silicon_fault_record()` обновляет общий portrait и вызывает HAL clock callback, поэтому он не является ISR-safe интерфейсом; в обработчике используйте заранее подготовленный snapshot и только `pl_silicon_fault_ring_push()` при доказанной lock-free SPSC-конфигурации.

Pain causes — причины наблюдаемой задержки: BUS_WAIT, IRQ_JITTER, DMA_QUEUE, DIE_TEMP. В v1 сумма боли и peak — диагностическая модель в тактах, а не аппаратная метрика температуры/очереди. Источник каждого события должен быть привязан к реальному таймеру/счётчику/сенсору и его единицам.

## 8. Бюджет потоков и детерминизм

`pl_silicon_thread_budget` реализует admission-control: поток получает логический slice, пока не превысит `quota_cycles`, затем выдерживает `breath_cycles`. Это не планировщик ОС, не preemptive scheduler и не управление affinity. Его нужно вызывать из уже существующего RTOS/кооперативного диспетчера; время `now_cycle` и `step_cost_cycles` должны относиться к одному аппаратному источнику.

`pl_silicon_determinism_route` закрепляет `route_id` для пары `(input_hash,state_hash)` и сообщает о расхождении предложенного маршрута. Коллизии хеша возможны; для safety-critical решений используйте полное сравнение входного tuple или криптографический digest, а не 32-битный hash как доказательство идентичности.

## 9. Машинный код и модель исполнения

`pl_silicon_verify_native_stream()` проверяет выравнивание и whitelist безопасных арифметических инструкций для Thumb-16 и RV32I. Управляющие переходы, системные/CSR, load/store и непроверенные opcodes отклоняются. Thumb-2 32-битные инструкции не входят в v1. Для x86/x86_64 верификатор пока возвращает `PL_SILICON_EUNSUPPORTED`: надёжная граница инструкций требует полноценного декодера и правил для prefixes, ModRM/SIB, control flow и instruction boundaries.

Проверка потока не означает, что он безопасен для исполнения в произвольном контексте. Она не проверяет ABI, calling convention, memory safety, privilege, stack, control-flow integrity, code provenance или concurrency.

RPR32 — отдельный 32-битный виртуальный формат с фиксированными полями opcode/rd/rs/imm16. `VERIFY_ONLY` проверяет и продвигает позицию без выполнения. `RPR32_INTERPRET` изменяет только shadow GPR и обращается к MMIO по exact allowlist. `NATIVE_ASM` и `JIT_TRAMPOLINE` всегда отклоняются. Не смешивайте этот формат с Thumb/RV32I машинным кодом.

## 10. Patch slots, таблицы и fixed-point

Patch bank хранит до четырёх SRAM-hosted payload-слотов по 128 байт, вычисляет CRC-16/CCITT и проверяет целостность. CRC обнаруживает случайное повреждение, но не подтверждает подлинность и не защищает от злонамеренной подмены: для аутентичности нужен отдельный доверенный механизм цифровой подписи/проверки. Это staging в памяти процесса, а не live patching исполняемого кода. Нет записи во Flash, исполнения патча, переключения vector table, MPU permission change или cache invalidation.

Экспорт таблиц сериализует заголовок и записи в little-endian, считает CRC тела и требует достаточную ёмкость destination. Для DMA/UART/SPI вызывающая сторона обязана обеспечить alignment, cache clean/invalidate, ownership и DMA barriers согласно SoC.

`pl_silicon_qmul_saturating()` использует signed 64-bit промежуточное произведение и насыщение в int32; деление определено с усечением к нулю. Это не универсальная Q15/Q31 конвенция: масштаб, rounding и overflow должны быть частью контракта конкретного сигнала.

## 11. Пример HAL и allowlist

```c
pl_silicon_config config = {
    PL_SILICON_ARCH_ARM_CORTEX_M,
    PL_SILICON_ABI_VERSION,
    0u,                 /* CPUID/part number задаётся BSP */
    84000000u,          /* только фактическая подтверждённая частота */
    32u
};

pl_silicon_hal hal = board_silicon_hal(); /* callbacks вашей платы */
pl_silicon_runtime runtime;
pl_silicon_mmio_bank mmio;

if (pl_silicon_runtime_init(&runtime, &config, &hal) != PL_SILICON_OK)
    board_safe_reset();

pl_silicon_mmio_bank_init(&mmio);
if (pl_silicon_mmio_add(&mmio, "GPIOA.ODR", BOARD_GPIOA_ODR, 32u,
        PL_SILICON_ACCESS_READ | PL_SILICON_ACCESS_WRITE,
        BOARD_GPIOA_ODR_WRITABLE_MASK,
        PL_SILICON_BARRIER_DMB) != PL_SILICON_OK)
    board_safe_reset();
```

`board_silicon_hal()` и `BOARD_GPIOA_ODR` — точки интеграции BSP, не символы Pytyliki и не выдуманные адреса. Адрес/маска должны браться из datasheet, reference manual и errata конкретной ревизии.

## 12. Ограничения и следующие hardware-specific этапы

До применения на изделии нужны отдельные проверенные BSP для конкретных плат: startup и linker script, vector table, clock tree, watchdog, fault register decoding, MPU/MMU, DMA/cache coherency, interrupt priority rules, device-specific MMIO register semantics, low-power wakeup, PMU calibration, stack watermarking и hardware-in-loop tests. Для x86 требуется audited instruction decoder. Для JIT/native code execution нужны отдельная executable-memory policy, W^X, cache maintenance, signature/provenance verification и threat model; v1 сознательно не включает эти функции.

**Никакой тест на host не подтверждает работу с физическим кремнием.** В CI проверяются C11 логика, fake HAL, границы доступа, форматы и инварианты; target validation должна выполняться на обозначенной плате с опубликованной аппаратной конфигурацией и измерительными данными.
