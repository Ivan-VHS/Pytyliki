
<p align="center"><picture><source media="(prefers-color-scheme: dark)" srcset="assets/readme/hero-banner-dark.svg"><source media="(prefers-color-scheme: light)" srcset="assets/readme/hero-banner-light.svg"><img src="assets/readme/hero-banner-light.svg" alt="Рабочая станция процессорных систем Pytyliki" width="880"></picture></p>
<h1 align="center">Pytyliki — рабочая станция процессорных систем</h1>
<p align="center"><strong>Нативная C11-среда для безопасного изучения процессора, ограниченных локальных тестов, численных инструментов и статической диагностики исходного кода.</strong></p>
<p align="center"><a href="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml"><img alt="CI" src="https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/ci.yml?branch=main&style=flat-square&label=CI"></a> <a href="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/desktop-build.yml"><img alt="Сборка Windows" src="https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/desktop-build.yml?branch=main&style=flat-square&label=Windows%20build"></a> <a href="LICENSE"><img alt="MIT" src="https://img.shields.io/github/license/Ivan-VHS/Pytyliki?style=flat-square"></a></p>

[Быстрый старт](#быстрый-старт) · [Revival](#обновление-документации-revival) · [Визуальный обзор](#визуальный-обзор) · [Архитектура](#архитектура) · [Документация](docs/README.md)

> **Границы:** обнаружение CPU и анализ исходников работают в режиме чтения. Pytyliki не меняет affinity, планирование ОС, частоту или напряжение, не прошивает firmware и не разворачивает embedded-код. Win32 GUI предназначен только для Windows; C11 core и CLI переносимы при наличии системных API.

## Быстрый старт

**Требования:** CMake 3.16+, компилятор C11 (MSVC, GCC или Clang) и Git.

~~~sh
git clone https://github.com/Ivan-VHS/Pytyliki.git
cd Pytyliki
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
~~~

Первые smoke-проверки:

~~~sh
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --partition 10000 8 256
./build/pytyliki-metrics --demo
~~~

В Windows исполняемые файлы Release находятся в build/Release/. Цель Win32 GUI собирается только в Windows.

## Обновление документации Revival

| Было | Стало |
|---|---|
| Один статичный баннер | Адаптивный SVG-баннер для светлой и тёмной тем |
| Назначение смешано с деталями реализации | Краткое описание, явные границы и проверяемый быстрый старт |
| Скриншот не отличался от схемы | Реальный снимок приложения и отдельно обозначенная архитектурная иллюстрация |
| Статусы возможностей разбросаны | Сводка с границами и ссылками на матрицу функций и карту архитектуры |
| Навигация спрятана в длинных разделах | Индекс документации, визуальный стандарт и правила контрибьюции |

Это обновление документации, **не заявление о новой версии ПО или выпуске**. Реализацию определяют исходный код, схемы, тесты и CI.

## Реализованный объём

| Область | Возможность | Граница |
|---|---|---|
| Инвентаризация CPU | Идентификатор, топология, память, ISA/XCR0 | Локальное чтение |
| Бенчмарки | Ограниченные CPU/memory-тесты и разделение нагрузки | Результат зависит от хоста |
| Профили | Импорт collapsed stacks и Speedscope; экспорт folded stacks и SVG flamegraph | Не запускает profiler |
| Анализ исходников | Статическая диагностика и отчёты | Не компилирует, не исполняет и не переписывает код |
| Embedded C / QCS | Оценка ресурсов MCU и AST | Справочная модель/симуляция и анализ синтаксиса |
| C ABI | Версионированный ABI v1 | Явная загрузка доверенной библиотеки |
| Atomic-Quantum | Ограниченный классический state-vector subset | Симуляция без квантового оборудования |

## Визуальный обзор

<table><tr><td width="50%"><a href="assets/screenshots/pytyliki-workstation.png"><img src="assets/screenshots/pytyliki-workstation.png" alt="Снимок Pytyliki в Windows" width="100%"></a><p><sub><strong>Реальный снимок</strong> — Windows desktop, 1920×1080. Это не браузерная демоверсия.</sub></p></td><td width="50%"><img src="assets/readme/workflow.svg" alt="Схема потока Pytyliki: ввод, проверка, модуль C11 и результат" width="100%"><p><sub><strong>Архитектурная иллюстрация</strong> — схема данных, а не снимок интерфейса.</sub></p></td></tr></table>

## Архитектура

~~~mermaid
flowchart LR
  A[Локальный ввод / probe CPU] --> B[Валидация и лимиты]
  B --> C[Нативное ядро C11]
  C --> D[Структурированный результат]
  D --> E[CLI или Win32 GUI]
  D --> F[Необязательный экспорт отчёта]
  G[Тесты и platform checks] --> C
~~~

## Конфигурация

Для работы не требуются .env, аккаунт, API-токен или hosted-сервис. Реальные опции CMake:

| Опция | По умолчанию | Назначение |
|---|---|---|
| BUILD_TESTING | значение CMake | Нативные тесты и необязательные contract checks |
| PYTYLIKI_ENABLE_SANITIZERS | OFF | ASan/UBSan для поддерживаемых GCC/Clang |

## Структура репозитория

| Путь | Содержимое |
|---|---|
| src/ | C11 core, CPU probes, CLI, Win32 GUI, QCS и interop |
| tests/ | Unit, regression, boundary и CLI smoke-тесты |
| tools/ / scripts/ | Валидация, импорт профилей и инженерные проверки |
| docs/ | Архитектура, функциональные контракты, UI и стандарты |
| assets/readme/ | Адаптивный баннер и схема потока |
| assets/screenshots/ | Снимки приложения |
| .github/workflows/ | CI, Windows packaging и проверки репозитория |

## Проверки и статус

~~~sh
ctest --test-dir build -C Release --output-on-failure
python scripts/validate_errors.py
git diff --check
~~~

Текущие границы и планы: [матрица функций](docs/FEATURES.md), [архитектура](docs/ARCHITECTURE.md), [карта документации](docs/INFORMATION_ARCHITECTURE.md), [план автоматизации](docs/AUTOMATION_ROADMAP.md). Счётчики каталогов и reference-модели не являются сертификацией оборудования.

## Разработка

Перед изменениями ядра, ABI или platform probes прочитайте [CONTRIBUTING.md](CONTRIBUTING.md). Правила публикации находятся в [README Visual Standard](docs/README_VISUAL_STANDARD.md), вопросы безопасности — в [SECURITY.md](SECURITY.md).

## Лицензия

[MIT](LICENSE).
