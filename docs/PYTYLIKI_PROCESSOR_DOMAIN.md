# Pytyliki: процессорная инженерия и физические вычислительные модели

## Назначение

Pytyliki — единый инженерный язык и рабочая платформа для исследования вычислительной аппаратуры на всех доступных уровнях: от типизированной программы для готового процессора до архитектуры процессора, цифровой схемы, электрической модели и математического описания физического вычислительного субстрата.

Временные эпохи и фантастические даты не являются частью спецификации. Язык должен расширяться вслед за реальными архитектурами, инструментами и проверяемыми физическими моделями, не подменяя неизвестные технологии выдуманными гарантиями.

Pytyliki не является набором несвязанных DSL. Это одна языковая система с явными доменами исполнения, типами, эффектами, целями компиляции и контрактами.

## 1. Три инженерных направления

**Processor Programming** — программирование существующего оборудования: регистры, машинные инструкции, адресные пространства, память, ABI, прерывания, MMIO, DMA, IOMMU, атомарные операции, firmware-контракты, измерения и трассировка.

**Processor Engineering** — проектирование самого вычислительного устройства: ISA, кодирование инструкций, архитектурное состояние, регистровые файлы, конвейеры, исполнительные блоки, очереди, кэш, логические элементы, автоматы, RTL, netlist, временные ограничения и формальная проверка.

**Physical Computing** — моделирование физических принципов вычисления и компонентов: электрические и полупроводниковые модели, фотонные и магнитные компоненты, спиновые и квантовые модели, тепловые и энергетические характеристики, статистические процессы, неопределённость, ядерная физика, распад, перенос излучения и модели детекторов.

Physical Computing описывает математические модели. Само наличие физической модели не даёт доступа к лабораторному прибору, источнику излучения, реактору или любому другому оборудованию.

## 2. Домены исполнения

Каждый модуль явно объявляет домен. Компилятор не должен незаметно превращать модель в firmware или программную функцию в цифровую схему.

~~~pytyliki
@extend pytyliki.hardware version "0.4";
@target riscv.application;
@domain firmware;
@hardware_effects explicit;
@unsafe deny_by_default;
~~~

| Домен | Назначение | Результат |
|---|---|---|
| Firmware | Код для конкретного процессора | Объектный код/прошивка при наличии backend |
| ArchitecturalModel | ISA и архитектурное состояние | Эталонная модель переходов |
| MicroarchitectureModel | Конвейеры, очереди, ресурсы и задержки | Модель микроархитектуры |
| StructuralLogic | Логические схемы, сигналы, такты и reset | RTL/netlist при наличии backend |
| PhysicalModel | Физические уравнения и параметры | Численная модель и отчёт |
| Simulation | Конфигурация эксперимента | Воспроизводимый запуск симулятора |
| Verification | Инварианты, свойства и покрытие | Отчёт с предпосылками и доказательствами |

Переход между доменами выполняется только через явно названный lowering или adapter. PhysicalModel не понижается в Firmware автоматически.

## 3. Типовая система

### Биты и представления

~~~pytyliki
type Bit<const N: usize>;
type SBit<const N: usize>;
type Logic<const N: usize>;           // 0, 1, X, Z
type UInt<const N: usize>;
type SInt<const N: usize>;
type Fixed<const I: usize, const F: usize>;
type Vector<const W: usize, const LANES: usize>;
type Address<const W: usize, Space>;
type Register<Bank, const W: usize>;
~~~

Ширина входит в идентичность типа. Знаковость, насыщение, переполнение, порядок байтов, сдвиги и срезы имеют определённую семантику. Неявное сужение разрядности запрещено.

### Физические величины

~~~pytyliki
type Quantity<Dimension, Unit>;
type Time = Quantity<Dimension<0, 0, 1>, Second>;
type Frequency = Quantity<Dimension<0, 0, -1>, Hertz>;
type Energy = Quantity<Dimension<2, 1, -2>, Joule>;
type Power = Quantity<Dimension<2, 1, -3>, Watt>;
type Temperature = Quantity<TemperatureDimension, Kelvin>;
type Charge = Quantity<ChargeDimension, Coulomb>;
~~~

Размерность и единица измерения — часть типа. Сложение несовместимых размерностей запрещено. Энергия не является мощностью, частота не является периодом, температура не является безразмерным числом.

### Адресные пространства и полномочия

~~~pytyliki
enum AddressSpace {
    HostVirtual,
    GuestVirtual,
    Physical,
    DeviceIOVA<Device>,
    RegisterSpace<Device>,
    InstructionSpace
}
type Capability<Permission, Space, Region>;
~~~

Числовой адрес сам по себе не разрешает доступ. Право связывается с конкретным диапазоном, устройством, видом операции, временем жизни и происхождением capability. Разрешения чтения, записи, отображения, исполнения, отправки DMA и управления прерыванием различаются.

Такт имеет смысл только относительно объявленного clock domain. Асинхронные сигналы проходят через явно описанный synchronizer, async FIFO или проверенный протокол CDC.

## 4. Целевая архитектура и ISA

Цель компиляции задаётся версионированным профилем, а не маркетинговым названием CPU.

~~~pytyliki
target_profile RiscvApplication {
    architecture = "riscv";
    profile = "RVA23";
    profile_version = "1.0";
    xlen = 64;
    endian = little;
    abi = "lp64d";
    privilege = [M, S, U];
    required_extensions = profile_required("RVA23");
    optional_extensions = [];
}
register_bank Gpr64 {
    width = 64;
    count = 32;
    zero_register = x0;
    program_counter = pc;
}
isa_instruction ADD {
    encoding = target.encoding("ADD");
    operands = [rd: Gpr, rs1: Gpr, rs2: Gpr];
    transition { rd <- rs1 + rs2; }
    effects = [ArchitecturalStateWrite];
    exceptions = [];
}
~~~

Профиль задаёт допустимые расширения, зависимости, конфликты, ABI, режимы привилегий и источник доказательств. Неизвестное обязательное свойство блокирует генерацию кода. Архитектурный регистр, физический регистр и поле кодирования инструкции — разные сущности.

Каждая инструкция имеет формат кодирования, ограничения операндов, предусловия, переход архитектурного состояния, эффекты памяти, исключения, привилегии и требования к расширениям ISA.

## 5. Микроархитектура

~~~pytyliki
microarchitecture CorePipeline {
    stages = [Fetch, Decode, Rename, Dispatch, Execute, Memory, Commit];
    resources = {
        issue_width: 4,
        reorder_entries: 128,
        load_queue_entries: 48,
        store_queue_entries: 32
    };
    speculation = {
        branch_predictor: target_defined;
        recovery: precise_commit;
    };
}
~~~

Модель описывает задержки, пропускную способность, занятость ресурсов, зависимости, очереди, конфликты, промахи кэша и восстановление после спекулятивных ошибок. Микроархитектурное состояние не становится архитектурным автоматически.

## 6. Память, MMIO, DMA и когерентность

~~~pytyliki
memory_map ReferenceMemory for target("reference-board") {
    region SRAM {
        base: 0x2000_0000;
        size: 128_KiB;
        permissions: [read, write];
        memory_type: normal_non_cacheable;
    }
    region DEVICE_REGISTERS {
        base: 0x4000_0000;
        size: 64_KiB;
        permissions: [device_read, device_write];
        memory_type: device;
    }
}

type Frame = system::array<64, 16, RAM>;
device dma_ring_worker(channel: DmaChannel) {
    on ready { submit_next_owned_descriptor(channel)?; }
    on complete(descriptor) {
        sync_for_cpu(descriptor.buffer)?;
        release_descriptor(descriptor)?;
    }
}
~~~

Декларация карты памяти не отображает физические страницы и не даёт полномочий на запись. Проверяются переполнение адресов, перекрытия, выравнивание, права, атрибуты памяти и принадлежность целевой плате.

DMA-буфер имеет явный жизненный цикл: CPU-owned → prepared-for-device → device-owned → completion-pending → synchronized-for-CPU → released. Допустимые переходы зависят от направления, когерентности и контракта устройства. Flush не является универсальной заменой DMA synchronization.

Язык различает порядок компилятора, CPU, interconnect и устройств. Acquire/release, атомики и fence имеют область действия и целевой lowering.

## 7. Прерывания и устройства

~~~pytyliki
table irq_dispatch [target = RiscvApplication] {
    | VECTOR | PRIORITY | HANDLER       | CONTEXT |
    | 7      | HIGH     | timer_handler | irq_safe |
}
fn timer_handler(context: InterruptContext<Machine>)
    -> Result<(), IrqError>
    effects [InterruptSafe, MemoryWrite<LocalQueue>]
{
    require context.stack_budget >= 256_bytes;
    enqueue_bounded(TIMER_EVENTS, TimerEvent::Tick)?;
    return Ok(());
}
~~~

Обработчик объявляет источник, приоритет, вложенность, сохраняемые регистры, бюджет стека, WCET-контракт и эффекты. Блокирующие операции и неограниченная работа запрещены в interrupt context.

## 8. Логика, сигналы, такты и reset

~~~pytyliki
logic_module RegisterSlice64 {
    input clk: Clock;
    input reset_n: Reset<ActiveLow>;
    input enable: Logic<1>;
    input data_in: Logic<64>;
    output data_out: Logic<64>;
    sequential on rising_edge(clk) {
        if reset_n == 0 { data_out <= 0; }
        else if enable == 1 { data_out <= data_in; }
    }
}
~~~

Logic использует четыре состояния: 0, 1, X и Z. Для схемы проверяются ширины, число драйверов, полнота reset, защёлки, clock crossing, fan-out и временные ограничения.

## 9. Автоматы, графы и параллелизм

~~~pytyliki
fsm ProcessorLifecycle initial RESET {
    state RESET: on reset_complete -> goto INITIALIZE;
    state INITIALIZE: on memory_ready and clock_ready -> goto IDLE;
    state IDLE: on work_available -> goto ACTIVE;
    state ACTIVE: on fault -> goto DEGRADED;
    state DEGRADED: on recovery_verified -> goto IDLE;
}
dag ComputeGraph {
    node read_stage: read_region(INPUT);
    node validate_stage: verify_bounds();
    node transform_stage: transform_bounded();
    node commit_stage: commit_output(OUTPUT);
    read_stage -> validate_stage;
    validate_stage -> transform_stage;
    transform_stage -> commit_stage;
}
~~~

FSM задаёт состояния и допустимые переходы. DAG сам по себе не разрешает параллельное исполнение: проверяются read/write sets, эффекты, память, ресурсы, порядок и зависимости отказа. Для потоков обязательны capacity, backpressure, overflow policy, cancellation и error contract.

## 10. Физические и ядерные модели

Конструкция **physical_model** предназначена для математически определённых моделей. В ней задаются входы, физические параметры, единицы измерения, предположения, уравнения, численные методы, неопределённость и происхождение данных.

~~~pytyliki
type Nuclide<const Z: u16, const N: u16> {
    atomic_number = Z;
    neutron_count = N;
    mass_number = Z + N;
}

physical_model radioactive_decay {
    input initial_atoms: Quantity<Count>;
    input elapsed: Quantity<Time>;
    input half_life: Quantity<Time>;
    require initial_atoms >= 0_count;
    require elapsed >= 0_s;
    require half_life > 0_s;

    derive decay_constant = ln(2.0) / half_life;
    derive expected_atoms =
        initial_atoms * exp(-decay_constant * elapsed);
    derive expected_decays = initial_atoms - expected_atoms;

    uncertainty propagate_poisson_and_parameter_error;
    invariant expected_atoms >= 0_count;
    invariant expected_decays >= 0_count;
    provenance dataset("nuclear/decay-reference.v1");
}
~~~

Ядерная физика является предметной вычислительной областью, а не набором команд управления установкой. Она охватывает нуклиды, состояния распада, периоды полураспада, активность, ветвления распада, сечения взаимодействия, перенос излучения в материалах, отклик детекторов, фон, статистику счёта и распространение неопределённости.

Для конечной выборки фактическое число распадов — случайная величина. Модель обязана различать математическое ожидание и случайную реализацию.

~~~pytyliki
physical_model decay_network {
    nodes = [Parent, DaughterA, DaughterB];
    transitions = [
        decay(Parent -> DaughterA, branch = 0.7),
        decay(Parent -> DaughterB, branch = 0.3)
    ];
    require branch_sum(Parent) == 1.0;
    require all_rates_nonnegative;
    solve bounded_bateman_system;
    uncertainty propagate_branching_and_half_life_error;
}

physical_model detector_response {
    input particle_energy: Quantity<Energy>;
    input material: MaterialProfile;
    input detector: DetectorProfile;
    derive interaction_probability = response_model(material, particle_energy);
    derive measured_signal = detector.response(interaction_probability);
    uncertainty combine_counting_noise_and_calibration_error;
    provenance [
        dataset("materials/materials.v1"),
        dataset("detectors/calibration.v1")
    ];
}
~~~

Сечения, коэффициенты ослабления, спектры и калибровки привязываются к источникам, диапазону применимости, температуре, составу материала, энергии и неопределённости. Неизвестные каналы остаются неизвестными.

~~~pytyliki
simulation DecayUncertainty {
    model = radioactive_decay;
    deterministic_seed = 0x5EED;
    samples = 100000;
    record_inputs = true;
    record_model_version = true;
    output = [mean, variance, confidence_interval];
}
~~~

Повторяемость требует фиксировать seed, генератор случайных чисел, версию модели, параметры, исходные данные, число выборок и правила оценки интервалов.

## 11. Верификация и неопределённость

~~~pytyliki
verify radioactive_decay {
    assert half_life > 0_s;
    assert expected_atoms <= initial_atoms;
    assert expected_atoms >= 0_count;
    assert expected_decays + expected_atoms ~= initial_atoms
        within numerical_tolerance;
    cover elapsed == 0_s;
    cover elapsed == half_life;
}
~~~

Платформа должна сочетать unit/golden tests, property-based tests, fuzzing, differential execution, model checking, bounded model checking, проверку эквивалентности, RTL assertions, coverage, численную устойчивость и распространение неопределённости.

Отчёт содержит предположения, границы, seed, версии инструментов, целевой профиль, исходные хеши и покрытие. Метка в исходнике не заменяет результат CI или формального доказательства.

## 12. Компилятор и связь с Reaper

Reaper остаётся владельцем общей языковой и компиляторной основы: канонического типизированного IR, Span, SourceMap, общих преобразований, ABI-контрактов и согласованной диагностики.

Pytyliki владеет процессорной предметной областью: hardware AST, target profiles, ISA, физическими и логическими моделями, hardware effects, специфическими проверками и адаптерами для симуляции и синтеза.

~~~text
Pytyliki source
 -> lexer and source spans
 -> parser and domain AST
 -> target/name resolution
 -> width, unit and capability checks
 -> effect and clock-domain checks
 -> Hardware IR / Physical Model IR
 -> Reaper canonical typed IR adapter
 -> target backend
 -> object / RTL / simulation model / verification report
~~~

Адресное пространство, разрядность, clock domain, эффекты, владение, физические единицы, неопределённость и source origin сохраняются на всех этапах.

## 13. Инструментарий

- Processor Studio — архитектура CPU и целевые профили.
- ISA Forge — форматы инструкций, декодирование и тесты переходов состояния.
- Microarchitecture Lab — конвейеры, очереди, ресурсы и timing.
- Logic Foundry — структурная логика, RTL и netlist.
- Memory Inspector — адресные пространства, memory map, cache и DMA.
- CycleScope — cycle-level trace и анализ задержек.
- Substrate Simulator — named model и deterministic replay.
- Physical Analyzer — единицы, численные ошибки и uncertainty.
- Detector Lab — отклик, шум, калибровка и статистика.
- Hardware Verifier — формальные свойства и отчёты доказательств.

Инструмент существует только после реализации, тестирования и регистрации в capability contract. Название в архитектурном документе не считается поставленной функциональностью.

## 14. Границы безопасности

По умолчанию обнаружение CPU read-only; физические адреса являются описаниями, не отображениями; MMIO — декларациями, не доступом к устройствам; DMA — моделью владения, не передачей. Firmware, микрокод, BIOS/UEFI, частота, напряжение и драйверы не меняются. Симулятор не подключается к реальной установке автоматически. Внешние наборы данных и backend подключаются только явным доверенным контрактом.

Для лабораторных, радиационных и иных критически значимых систем вычислительная модель отделена от физического контура управления. Верификация кода не является сертификацией установки или разрешением на эксперимент.

## 15. Этапы реализации

1. Синтаксис: physical_model, аппаратные декларации, стабильные AST IDs и source spans.
2. Семантический frontend: имена, типы, размерности, регионы, capabilities, effects и target profiles.
3. Hardware IR: lossless lowering и адаптер к Reaper typed IR.
4. ISA Core: кодирование, decoder, architectural transition model и golden tests.
5. Детерминированный emulator: исполнение ISA, события и replay.
6. Microarchitecture: latency, throughput, ресурсы и timing model.
7. StructuralLogic: RTL adapter, lint, CDC и формальная проверка.
8. PhysicalModel: уравнения, единицы, численные методы и provenance.
9. Scientific domains: ядерная физика, материалы, детекторы и uncertainty.
10. Backend qualification: отдельно для каждого процессора, платы, RTL toolchain или симулятора.

## 16. Текущий статус

Текущий аппаратный frontend распознаёт структуру верхнеуровневых конструкций. Добавление physical_model расширяет каталог синтаксических форм, но не означает, что вложенные выражения уже типизированы или исполняются.

| Возможность | Статус |
|---|---|
| 21 верхнеуровневая hardware-конструкция | Структурный синтаксис |
| Processor/physical-domain source specimen | Проверка Iron Core framing |
| Физические и ядерные типы | Спецификация/пример, не typechecker |
| Размерностный анализ | Не реализован |
| ISA semantic compiler | Не реализован этим изменением |
| Hardware IR lowering to Reaper | Контракт и план |
| Deterministic ISA emulator | Не реализован этим изменением |
| RTL generation | Не реализован этим изменением |
| Physical/nuclear simulator | Не реализован этим изменением |
| Physical device access | Не предоставляется |

Приёмка последующих этапов требует исполняемых тестов, независимых эталонов и измеримых доказательств. Статус нельзя повышать с structural или planned на implemented только из-за наличия документации и примеров.

**PYTYLIKI — FROM PROCESSOR INSTRUCTIONS TO PHYSICAL COMPUTING MODELS.**
