<p align="center">[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)</p>

<h1 align="center"><strong>🧮 Pytyliki — робоча станція процесорних систем</strong></h1>

[![C11 CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/ci.yml?branch=main&label=C11%20CI)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml) [![Windows build](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/desktop-build.yml?branch=main&label=Windows%20build)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/desktop-build.yml) [![MIT](https://img.shields.io/github/license/Ivan-VHS/Pytyliki)](LICENSE)

> **Аналіз процесора з чіткими обмеженнями. Нативні вимірювання. Докази замість здогадів.**

**Стан:** Розробка · обмежене ядро C11 · **Версія:** 0.8.0 · CMake project · **License:** [MIT](LICENSE) · **Стандарт README:** Psychoporter 666 v1.0 · **Дата перевірки:** 2026-11-25 · UTC · **Source:** [Ivan-VHS/Pytyliki](https://github.com/Ivan-VHS/Pytyliki)

<p align="center"><img src="assets/pytyliki-hero.svg" alt="Pytyliki — робоча станція процесорних систем — project overview" width="100%"></p>
<p align="center"><a href="assets/screenshots/pytyliki-workstation.png"><img src="assets/screenshots/pytyliki-workstation.png" alt="Pytyliki — робоча станція процесорних систем — application screenshot" width="100%"></a></p>

## Швидка навігація

[🚀 Demo: workstation screenshot](assets/screenshots/pytyliki-workstation.png) · [Documentation](docs/README.md) · [Source repository](https://github.com/Ivan-VHS/Pytyliki) · [Issues](https://github.com/Ivan-VHS/Pytyliki/issues)

## Призначення та межі

Локальна робоча станція процесорних систем на основі обмеженого ядра C11, графічного інтерфейсу Win32 лише для Windows і переносних CLI-інструментів. Виявлення обладнання працює тільки на читання. Статичний аналіз коду не компілює, не запускає й не переписує перевірювані файли.

## Можливості

- Інвентаризація топології CPU, ISA та пам’яті в режимі читання.
- Обмежені навантаження CPU, пам’яті й матриць зі статистикою часу; результати залежать від машини.
- Імпорт профілів, folded stacks і SVG flamegraphs.
- Статична діагностика вихідного коду, синтаксичний аналіз QCS і довідкові оцінки FLASH/SRAM для MCU.
- Версіонований C ABI v1 із буферами сторони-викликача та звітами з походженням даних.

## Архітектура та потік даних

~~~mermaid
flowchart LR
  U["User / CLI / Win32 UI"] --> C["C11 core"]
  C --> D["Read-only discovery"]
  C --> B["Bounded measurements"]
  C --> S["Static source and QCS checks"]
  D --> R["Structured evidence report"]
  B --> R
  S --> R
  C --> A["Versioned C ABI v1"]
~~~

Вихідний код, схеми, конфігурація збирання, тести та CI підтверджують реалізовану поведінку. Дорожня карта й каталоги описують плани та переліки, а не готові функції.

## Вимоги

CMake 3.16+, компілятор C11 (MSVC, GCC або Clang) і Git. Ядро та CLI переносні в межах заявленої підтримки Windows/Linux; Win32 GUI працює лише у Windows. Python — необов’язковий інструмент тестування, а не вимога середовища виконання.

## Збирання

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
~~~

Графічний інтерфейс Win32 працює лише у Windows. Переносними точками входу залишаються ядро C11 та CLI.

## Перші перевірки

~~~sh
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --partition 10000 8 256
./build/pytyliki-metrics --demo
~~~

Команди виводять локальні результати, залежні від обладнання; налаштування ОС не змінюються.

## Запуск

Для локальної перевірки використовуйте переносний CLI; графічний інтерфейс Win32 доступний у збірках для Windows.

## Інтерфейси

| Інтерфейс | Контракт / точка входу | Обмеження |
|---|---|---|
| CLI | `pytyliki / pytyliki-metrics / pytyliki-qcs` | Локальні команди; довідка доступна через `--help` і документацію можливостей. |
| Native FFI | `pytyliki_ffi / C ABI v1` | Бібліотека обирається явно; буфери належать стороні-викликачу. |
| Звіти | Версіонований JSON/text | Контракт звіту містить походження даних та обмеження. |

REST/GraphQL, WebSocket, OAuth, хмарний deployment і cookie-механізми не описуються як можливості, якщо в репозиторії немає версіонованої реалізації та тестів.

## Безпека та межі довіри

Запис до прошивки, BIOS/UEFI, мікрокоду, частот, напруги, драйверів або регістрів пристроїв не виконується. MMIO-описи Embedded C є лише симуляцією/довідковою моделлю. Для Native FFI потрібен явно заданий довірений шлях до бібліотеки.

Політика безпеки: [SECURITY.md](SECURITY.md). Не публікуйте секрети, ідентифікатори пристроїв і приватні журнали у відкритих задачах.

## Перевірка якості

Набір CTest запускається командою з розділу «Збирання»; окрема команда перевірки не потрібна.

Склад тестів визначається зареєстрованими CTest/unittest цілями та workflow. Відсоток покриття, fuzz-результат і звіт навантаження не заявляються без опублікованого артефакту.

## Докази продуктивності

Результати бенчмарків залежать від хоста й призначені для локального порівняння. Методи статистики та архіви результатів описані в документації; універсальні значення затримки, споживання пам’яті чи міжмашинного рейтингу не заявляються.

## Сумісність

Заявлено runtime-цілі Windows і Linux; Win32 GUI працює лише у Windows. Переносними точками входу є ядро C11 та CLI.

## Документація та екосистема

- [Feature matrix](docs/FEATURES.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Benchmark statistics](docs/BENCHMARK_STATISTICS.md)
- [Report envelope](docs/REPORT_ENVELOPE.md)
- [Embedded C](docs/EMBEDDED_C.md)
- [QCS](docs/QCS_NATIVE.md)
- [Documentation index](docs/README.md)
- [Reaper integration](docs/REAPER_INTEGRATION.md)

Основні технічні джерела — матриця можливостей, архітектура, статистика бенчмарків і формат звітів.

Інтеграції між проєктами використовують документовані версіоновані інтерфейси та явні адаптери. Необов’язковий адаптер не надає іншому проєкту доступу до обладнання або привілеїв середовища виконання.

## Участь у розробці

Ознайомтеся з CONTRIBUTING.md перед зміною коду або контрактів. Робіть зміни невеликими й перевірними, додавайте детерміновані тести та використовуйте Conventional Commits, якщо цього вимагає посібник.

## Ліцензія та правові умови

Поширюється за ліцензією [MIT](LICENSE). Програмне забезпечення надається «ЯК Є» в межах умов ліцензії. Назви сторонніх проєктів і торговельні марки належать їхнім правовласникам.

Цей репозиторій описує локальні програмні інтерфейси. Умови хмарного сервісу, згода на cookies, OAuth і CORS не стосуються зазначених локальних компонентів, доки окремо не буде впроваджено й задокументовано мережевий сервіс.

---

<p align="center"><sub>Psychoporter 666 README Standard v1.0 · Ivan-VHS · 2026-11-25 · UTC</sub></p>
