# Pytyliki — рабочая станция процессорных систем

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

[![C11 CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/ci.yml?branch=main&label=C11%20CI)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml) [![Сборка Windows](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/desktop-build.yml?branch=main&label=Windows%20build)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/desktop-build.yml) [![MIT](https://img.shields.io/github/license/Ivan-VHS/Pytyliki)](LICENSE)

<p align="center"><img src="assets/readme/dashboard.svg" alt="Обзор Pytyliki" width="100%"></p>
<p align="center"><a href="assets/screenshots/pytyliki-workstation.png"><img src="assets/screenshots/pytyliki-workstation.png" alt="Pytyliki в Windows" width="100%"></a></p>

Pytyliki — локальная рабочая станция для анализа процессорных систем на базе ограниченного ядра C11, интерфейса Windows и переносимых CLI-инструментов.

**Границы:** инвентаризация процессора и файлов, локальный анализ с заданными лимитами и диагностические отчёты. Сбор аппаратных данных выполняется только на чтение. Отдельная обратимая affinity-аренда может сузить маску только текущего worker Pytyliki до процессора, уже разрешённого ОС; глобальное планирование, online/offline CPU, частоты, напряжение и прошивка не меняются.

## Возможности

- Топология CPU, ISA и память; ограниченные измерения CPU/памяти и примеры распределения нагрузки.
- Анализ периодичности сбоев потоков и ограниченные планы восстановления для потоков под управлением приложения.
- Read-only адаптеры Windows WHEA и Linux EDAC с SHA-256 журналом доказательств.
- Обратимая affinity-аренда текущего worker и Swift/TypeScript потребители отчётов.
- Импорт и анализ профилей, folded stacks и SVG flamegraphs.
- Диагностика исходников, синтаксический анализ QCS и справочные оценки FLASH/SRAM для MCU.
- Версионированный C ABI и структурированные отчёты с происхождением данных и ограничениями.

Подробнее: [Возможности](docs/FEATURES.md) · [Архитектура](docs/ARCHITECTURE.md) · [Статистика бенчмарков](docs/BENCHMARK_STATISTICS.md) · [Embedded C](docs/EMBEDDED_C.md) · [QCS](docs/QCS_NATIVE.md) · [Восстановление процессорных потоков](docs/PROCESSOR_THREAD_RECOVERY.md) · [Платформенные адаптеры](docs/PLATFORM_ADAPTERS.md).

## Сборка и тесты

Требуются CMake 3.16+, компилятор C11 (MSVC, GCC или Clang) и Git. WHEA-адаптер доступен в Windows; Linux EDAC собирается опционально с Rust stable/Cargo. Swift и Node.js 22 нужны только для отдельных потребителей доказательств.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Первые проверки:

```sh
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --partition 10000 8 256
./build/pytyliki-metrics --demo
```

## Спецификации и интеграция

- [Индекс документации](docs/README.md) · [Релизы](docs/RELEASE.md) · [Формат отчёта](docs/REPORT_ENVELOPE.md)
- [Интеграция с Reaper](docs/REAPER_INTEGRATION.md) — необязательная CI-проверка политик, не runtime-зависимость приложения.
- [Общие стандарты экосистемы](https://github.com/Ivan-VHS/DAK/blob/main/docs/SHARED_STANDARDS.md)

[Вклад в проект](CONTRIBUTING.md) · [Безопасность](SECURITY.md) · [MIT](LICENSE)
