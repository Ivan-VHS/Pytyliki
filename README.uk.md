
<p align="center"><picture><source media="(prefers-color-scheme: dark)" srcset="assets/readme/hero-banner-dark.svg"><source media="(prefers-color-scheme: light)" srcset="assets/readme/hero-banner-light.svg"><img src="assets/readme/hero-banner-light.svg" alt="Робоча станція процесорних систем Pytyliki" width="880"></picture></p>
<h1 align="center">Pytyliki — робоча станція процесорних систем</h1>
<p align="center"><strong>Нативне середовище C11 для безпечного дослідження процесора, обмежених локальних тестів, числових інструментів і статичної діагностики вихідного коду.</strong></p>
<p align="center"><a href="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml"><img alt="CI" src="https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/ci.yml?branch=main&style=flat-square&label=CI"></a> <a href="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/desktop-build.yml"><img alt="Windows build" src="https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/desktop-build.yml?branch=main&style=flat-square&label=Windows%20build"></a> <a href="LICENSE"><img alt="MIT" src="https://img.shields.io/github/license/Ivan-VHS/Pytyliki?style=flat-square"></a></p>

[Швидкий старт](#швидкий-старт) · [Revival](#оновлення-документації-revival) · [Візуальний огляд](#візуальний-огляд) · [Архітектура](#архітектура) · [Документація](docs/README.md)

> **Межі:** виявлення CPU й аналіз вихідного коду працюють у режимі читання. Pytyliki не змінює affinity, планування ОС, частоту чи напругу, не прошиває firmware і не розгортає embedded-код. Win32 GUI призначений лише для Windows; C11 core і CLI переносимі за наявності потрібних API.

## Швидкий старт

**Вимоги:** CMake 3.16+, компілятор C11 (MSVC, GCC або Clang) і Git.

~~~sh
git clone https://github.com/Ivan-VHS/Pytyliki.git
cd Pytyliki
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
~~~

Перші локальні перевірки:

~~~sh
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --partition 10000 8 256
./build/pytyliki-metrics --demo
~~~

У Windows виконувані файли Release розташовані в build/Release/. Ціль Win32 GUI збирається лише у Windows.

## Оновлення документації Revival

| Було | Стало |
|---|---|
| Один статичний банер | Адаптивний SVG-банер для світлої й темної тем |
| Призначення змішане з деталями реалізації | Стисле пояснення, чіткі межі та перевірюваний швидкий старт |
| Скріншот не відрізнявся від схеми | Реальний знімок застосунку й окремо позначена архітектурна ілюстрація |
| Статуси можливостей були розпорошені | Зведення з межами та посиланнями на матрицю функцій і карту архітектури |
| Навігація була прихована в довгих розділах | Індекс документації, візуальний стандарт і правила внесків |

Це оновлення документації, **а не заява про нову версію ПЗ чи реліз**. Стан реалізації визначають вихідний код, схеми, тести та CI.

## Реалізований обсяг

| Напрям | Можливість | Межа |
|---|---|---|
| Інвентаризація CPU | Ідентифікатор, топологія, пам’ять, ISA/XCR0 | Локальне читання |
| Бенчмарки | Обмежені CPU/memory-тести й приклади розподілу навантаження | Результат залежить від хоста |
| Профілі | Імпорт collapsed stacks і Speedscope; експорт folded stacks та SVG flamegraph | Не запускає profiler |
| Аналіз коду | Статична діагностика та звіти | Не компілює, не виконує й не переписує код |
| Embedded C / QCS | Оцінка ресурсів MCU та AST | Довідкова модель/симуляція й аналіз синтаксису |
| C ABI | Версійований ABI v1 | Явне завантаження довіреної бібліотеки |
| Atomic-Quantum | Обмежений класичний state-vector subset | Симуляція без квантового обладнання |

## Візуальний огляд

<table><tr><td width="50%"><a href="assets/screenshots/pytyliki-workstation.png"><img src="assets/screenshots/pytyliki-workstation.png" alt="Знімок Pytyliki у Windows" width="100%"></a><p><sub><strong>Реальний знімок</strong> — Windows desktop, 1920×1080. Це не браузерна демоверсія.</sub></p></td><td width="50%"><img src="assets/readme/workflow.svg" alt="Схема потоку Pytyliki: ввід, перевірка, модуль C11 і результат" width="100%"><p><sub><strong>Архітектурна ілюстрація</strong> — схема потоку даних, а не знімок інтерфейсу.</sub></p></td></tr></table>

## Архітектура

~~~mermaid
flowchart LR
  A[Локальний ввід / probe CPU] --> B[Валідація та ліміти]
  B --> C[Нативне ядро C11]
  C --> D[Структурований результат]
  D --> E[CLI або Win32 GUI]
  D --> F[Необов’язковий експорт звіту]
  G[Тести й platform checks] --> C
~~~

## Конфігурація

Для роботи не потрібні .env, обліковий запис, API-токен чи hosted-сервіс. Реальні опції CMake:

| Опція | Типове значення | Призначення |
|---|---|---|
| BUILD_TESTING | значення CMake | Нативні тести та необов’язкові contract checks |
| PYTYLIKI_ENABLE_SANITIZERS | OFF | ASan/UBSan для підтримуваних GCC/Clang |

## Структура репозиторію

| Шлях | Вміст |
|---|---|
| src/ | C11 core, CPU probes, CLI, Win32 GUI, QCS та interop |
| tests/ | Unit, regression, boundary і CLI smoke-тести |
| tools/ / scripts/ | Валідація, імпорт профілів та інженерні перевірки |
| docs/ | Архітектура, функціональні контракти, UI та стандарти |
| assets/readme/ | Адаптивний банер і схема потоку |
| assets/screenshots/ | Знімки застосунку |
| .github/workflows/ | CI, Windows packaging і перевірки репозиторію |

## Перевірки та статус

~~~sh
ctest --test-dir build -C Release --output-on-failure
python scripts/validate_errors.py
git diff --check
~~~

Поточні межі та плани: [матриця функцій](docs/FEATURES.md), [архітектура](docs/ARCHITECTURE.md), [карта документації](docs/INFORMATION_ARCHITECTURE.md), [план автоматизації](docs/AUTOMATION_ROADMAP.md). Лічильники каталогів і reference-моделі не є сертифікацією обладнання.

## Розробка

Перед змінами ядра, ABI або platform probes прочитайте [CONTRIBUTING.md](CONTRIBUTING.md). Правила публікації наведено в [README Visual Standard](docs/README_VISUAL_STANDARD.md), питання безпеки — у [SECURITY.md](SECURITY.md).

## Ліцензія

[MIT](LICENSE).
