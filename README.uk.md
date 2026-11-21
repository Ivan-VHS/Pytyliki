# Pytyliki — робоча станція процесорних систем

C11 · Windows/Linux · MIT · [CI](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml)
**Розробник:** [Ivan-VHS](https://github.com/Ivan-VHS)

Нативна модульна станція для виявлення CPU, обмежених навантажень, статичного аналізу коду, чисельних інструментів і локальної DSP. GUI та CLI використовують спільне ядро C11.

| Модуль | Функції | Межа безпеки |
|---|---|---|
| CPU | модель, топологія, пам’ять, ISA/XCR0 | лише читання |
| Тести | CPU, пам’ять, матриці, планування частин | обмежені вимірювання |
| Код | статична діагностика, звіти | без збирання/запуску |
| DSP | PCM16 WAV, фільтр, компресія | локальні буфери |
| Пам’ять | views, slices, арени, pools | фіксовані межі |
| Embedded C | профілі MCU, FLASH/SRAM, MMIO | лише симуляція |
| Rules/TRACE | рекомендації та походження вимірювань | не сертифікація |
| QCS | lexer, parser, AST | лише синтаксис |
| ABI | версійований C ABI v1 | явний вибір бібліотеки |

Збирання: CMake 3.16+, компілятор C11, Git.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

| Каталог | Записів |
|---|---:|
| Можливості системних мов | 66 |
| Профілі CPU/MCU/віртуальних ISA | 14 |
| Функції системної лабораторії | 33 |

Планування: `--divide A B`, `--partition ITEMS WORKERS GRAIN`, `--partition-auto ITEMS GRAIN`. План обмежений; він не запускає потоки й не змінює affinity.

[Функції](docs/FEATURES.md) · [Архітектура](docs/ARCHITECTURE.md) · [Планування](docs/DIVISION_ENGINE.md) · [TRACE](docs/TRACE_MEASUREMENT_PROVENANCE.md) · [Індекс](docs/README.md)

[English](README.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md) · [Релізи](https://github.com/Ivan-VHS/Pytyliki/releases) · [MIT](LICENSE)
