# Pytyliki — рабочая станция процессорных систем

C11 · Windows/Linux · MIT · [CI](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml)
**Разработчик:** [Ivan-VHS](https://github.com/Ivan-VHS)

Нативная модульная станция для обнаружения CPU, ограниченных нагрузок, статической проверки исходников, численных инструментов и локальной DSP. GUI и CLI используют общее ядро C11.

| Модуль | Функции | Граница безопасности |
|---|---|---|
| CPU | модель, топология, память, ISA/XCR0 | только чтение |
| Тесты | CPU, память, матрицы, разбиение задач | ограниченные измерения |
| Исходники | статическая диагностика, отчёты | без сборки/запуска |
| DSP | PCM16 WAV, фильтр, компрессия, метры | локальные буферы |
| Память | views, slices, арены, pools | фиксированные лимиты |
| Embedded C | профили MCU, FLASH/SRAM, MMIO | только симуляция |
| Rules/TRACE | рекомендации и происхождение измерений | не сертификация |
| QCS | lexer, parser, AST | только синтаксис |
| ABI | versioned C ABI v1 | явный выбор библиотеки |

Сборка: CMake 3.16+, компилятор C11, Git.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

| Каталог | Записей |
|---|---:|
| Возможности системных языков | 66 |
| Профили CPU/MCU/виртуальных ISA | 14 |
| Функции системной лаборатории | 33 |

Разбиение задач: `--divide A B`, `--partition ITEMS WORKERS GRAIN`, `--partition-auto ITEMS GRAIN`. План ограничен и не запускает потоки и не меняет affinity.

[Функции](docs/FEATURES.md) · [Архитектура](docs/ARCHITECTURE.md) · [Разбиение](docs/DIVISION_ENGINE.md) · [TRACE](docs/TRACE_MEASUREMENT_PROVENANCE.md) · [Индекс](docs/README.md)

[English](README.md) · [Українська](README.uk.md) · [简体中文](README.zh-CN.md) · [Релизы](https://github.com/Ivan-VHS/Pytyliki/releases) · [MIT](LICENSE)
