# Pytyliki — робоча станція процесорних систем

[CI](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml) · [Збирання Windows](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/desktop-build.yml) · C11 · Windows/Linux · MIT

Pytyliki — модульна нативна робоча станція для виявлення характеристик процесора, планування обмежених навантажень, статичної діагностики коду, числових інструментів і локальної DSP. Windows GUI та CLI використовують спільне ядро C11.

## Реальний знімок Windows-застосунку

<p align="center"><img src="assets/screenshots/pytyliki-workstation.png" alt="Робоча станція Pytyliki у Windows" width="100%"></p>
<sub>Справжній знімок запущеного застосунку з desktop-build workflow, 1920×1080.</sub>

## Можливості

| Модуль | Поточні функції | Межа безпеки |
|---|---|---|
| CPU | модель, топологія, пам’ять, ISA/XCR0 і профілі процесорів | виявлення лише для читання |
| Тести | CPU, пам’ять, матриці та перевірене розбиття навантаження | обмежені локальні вимірювання |
| Код | статична діагностика та структуровані звіти | без збирання й виконання перевірюваного коду |
| DSP | PCM16 WAV, фільтрація, компресія, сатурація та вимірювання | локальні буфери; перевірка меж RIFF |
| Пам’ять | типізовані views, checked slices, арени та пули | фіксовані ліміти й перевірка діапазонів |
| Embedded C | профілі MCU, оцінки FLASH/SRAM та модель MMIO | лише симуляція й довідкові дані |
| Rules / APSIA / TRACE | поради щодо навантажень і походження вимірювань | докази, а не сертифікація |
| QCS | lexer, parser, AST і діагностика | лише синтаксичний аналіз |
| Interop | версійований C ABI v1 | явний вибір довіреної бібліотеки |

## Збирання та тести

Потрібні CMake 3.16+, компілятор C11 і Git.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Windows workflow збирає GUI та CLI, запускає CTest і публікує архів Windows x64. До архіву входять `pytyliki-gui.exe`, `pytyliki.exe`, `pytyliki-apsia.exe` та `pytyliki-metrics.exe`.

## Приклади CLI

```sh
pytyliki --profile
pytyliki --suite
pytyliki --systems
pytyliki --embedded profiles
pytyliki-metrics --demo

pytyliki --divide -17 5
pytyliki --partition 10000 8 256
pytyliki --partition-auto 10000 256
```

Плани розбиття детерміновані, обмежені й рекомендаційні. Вони описують неперервні діапазони робіт, але не запускають потоки, не змінюють CPU affinity і не обіцяють неперевірене прискорення.

## Каталоги

| Каталог | Записів |
|---|---:|
| Можливості системної мови | 66 |
| Профілі CPU / MCU / віртуальних ISA | 14 |
| Функції системної лабораторії | 33 |

[Можливості](docs/FEATURES.md) · [Архітектура](docs/ARCHITECTURE.md) · [Ділення та розбиття](docs/DIVISION_ENGINE.md) · [TRACE](docs/TRACE_MEASUREMENT_PROVENANCE.md) · [Індекс документації](docs/README.md)

[English](README.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md) · [Релізи](https://github.com/Ivan-VHS/Pytyliki/releases) · [MIT](LICENSE)
