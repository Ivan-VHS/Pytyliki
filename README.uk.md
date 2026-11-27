# Pytyliki — робоча станція процесорних систем

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

[![C11 CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/ci.yml?branch=main&label=C11%20CI)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml) [![Windows build](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/desktop-build.yml?branch=main&label=Windows%20build)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/desktop-build.yml) [![MIT](https://img.shields.io/github/license/Ivan-VHS/Pytyliki)](LICENSE)

<p align="center"><img src="assets/readme/dashboard.svg" alt="Огляд Pytyliki" width="100%"></p>
<p align="center"><a href="assets/screenshots/pytyliki-workstation.png"><img src="assets/screenshots/pytyliki-workstation.png" alt="Pytyliki у Windows" width="100%"></a></p>

Pytyliki — локальна робоча станція для аналізу процесорних систем на основі обмеженого ядра C11, інтерфейсу Windows і переносних CLI-інструментів.

**Межі:** інвентаризація процесора й файлів, локальний аналіз із визначеними лімітами та діагностичні звіти. Виявлення обладнання працює лише на читання: застосунок не змінює планування ОС, affinity, частоти, напругу чи прошивку.

## Можливості

- Топологія CPU, ISA та пам’ять; обмежені вимірювання CPU/пам’яті й приклади розподілу навантаження.
- Аналіз періодичності збоїв потоків і обмежені плани відновлення для потоків під керуванням застосунку.
- Імпорт та аналіз профілів, folded stacks і SVG flamegraphs.
- Діагностика коду, синтаксичний аналіз QCS і довідкові оцінки FLASH/SRAM для MCU.
- Версійований C ABI та структуровані звіти з походженням даних і межами точності.

Деталі: [Можливості](docs/FEATURES.md) · [Архітектура](docs/ARCHITECTURE.md) · [Статистика бенчмарків](docs/BENCHMARK_STATISTICS.md) · [Embedded C](docs/EMBEDDED_C.md) · [QCS](docs/QCS_NATIVE.md) · [Відновлення потоків процесора](docs/PROCESSOR_THREAD_RECOVERY.md).

## Збирання й тести

Потрібні CMake 3.16+, компілятор C11 (MSVC, GCC або Clang) і Git. Win32 GUI доступний лише у Windows; ядро та CLI переносні. Python необов’язковий і використовується тільки окремими перевірками контрактів, а не runtime C11.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Перші перевірки:

```sh
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --partition 10000 8 256
./build/pytyliki-metrics --demo
```


Перегляд журналу подій Windows і обмежений аналіз історії збоїв:

```sh
./build/pytyliki --thread-recovery tests/fixtures/processor_thread_recovery.csv
./build/pytyliki --thread-recovery-demo
# Лише Windows: читання метаданих WHEA (тільки читання)
./build/pytyliki --ras-whea
```

PTFR формує рекомендації для потоків, якими керує сам застосунок. Записи WHEA залишаються системними свідченнями: серйозність помилки та її зв’язок із worker не виводяться автоматично.
\n## Специфікації та інтеграція

- [Індекс документації](docs/README.md) · [Релізи](docs/RELEASE.md) · [Формат звіту](docs/REPORT_ENVELOPE.md)
- [Інтеграція з Reaper](docs/REAPER_INTEGRATION.md) — необов’язкова CI-перевірка політик, а не runtime-залежність застосунку.
- [Спільні стандарти екосистеми](https://github.com/Ivan-VHS/DAK/blob/main/docs/SHARED_STANDARDS.md)

[Внесок у проєкт](CONTRIBUTING.md) · [Безпека](SECURITY.md) · [MIT](LICENSE)
