# 🧮 Pytyliki — рабочая станция процессорных систем

<p align="center">[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)</p>

[![C11 CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/ci.yml?branch=main&label=C11%20CI)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml) [![Windows build](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/Pytyliki/desktop-build.yml?branch=main&label=Windows%20build)](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/desktop-build.yml) [![MIT](https://img.shields.io/github/license/Ivan-VHS/Pytyliki)](LICENSE)

> **Анализ процессора с жёсткими лимитами. Нативные измерения. Доказательства вместо догадок.**

**Состояние:** Разработка · ограниченное ядро C11 · **Версия:** 0.8.0 · CMake project · **License:** [MIT](LICENSE) · **Стандарт README:** Psychoporter 666 v1.0 · **Дата проверки:** 2026-11-25 · UTC · **Source:** [Ivan-VHS/Pytyliki](https://github.com/Ivan-VHS/Pytyliki)

<p align="center"><img src="assets/pytyliki-hero.svg" alt="Pytyliki — рабочая станция процессорных систем — project overview" width="100%"></p>
<p align="center"><a href="assets/screenshots/pytyliki-workstation.png"><img src="assets/screenshots/pytyliki-workstation.png" alt="Pytyliki — рабочая станция процессорных систем — application screenshot" width="100%"></a></p>

## Быстрая навигация

[🚀 Demo: workstation screenshot](assets/screenshots/pytyliki-workstation.png) · [Documentation](docs/README.md) · [Source repository](https://github.com/Ivan-VHS/Pytyliki) · [Issues](https://github.com/Ivan-VHS/Pytyliki/issues)

## Назначение и границы

Локальная рабочая станция процессорных систем на базе ограниченного ядра C11, графического интерфейса Win32 только для Windows и переносимых CLI-инструментов. Обнаружение оборудования выполняется только на чтение. Статический анализ исходников не компилирует, не запускает и не переписывает проверяемые файлы.

## Возможности

- Инвентаризация топологии CPU, ISA и памяти в режиме чтения.
- Ограниченные нагрузки CPU, памяти и матриц со статистикой времени; результаты зависят от машины.
- Импорт профилей, folded stacks и SVG flamegraphs.
- Статическая диагностика исходников, синтаксический анализ QCS и справочные оценки FLASH/SRAM для MCU.
- Версионированный C ABI v1 с буферами вызывающей стороны и отчётами с происхождением данных.

## Архитектура и поток данных

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

Исходный код, схемы, конфигурация сборки, тесты и CI подтверждают реализованное поведение. Дорожная карта и каталоги описывают планы и перечни, а не поставляемые функции.

## Требования

CMake 3.16+, компилятор C11 (MSVC, GCC или Clang) и Git. Ядро и CLI переносимы в пределах заявленной поддержки Windows/Linux; Win32 GUI работает только в Windows. Python — необязательный инструмент тестов, а не требование среды выполнения.

## Сборка

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
~~~

Графический интерфейс Win32 работает только в Windows. Переносимыми точками входа остаются ядро C11 и CLI.

## Первые проверки

~~~sh
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --partition 10000 8 256
./build/pytyliki-metrics --demo
~~~

Команды выводят локальные результаты, зависящие от оборудования; настройки ОС не меняются.

## Запуск

Для локальной проверки используйте переносимый CLI; графический интерфейс Win32 доступен в сборках для Windows.

## Интерфейсы

| Интерфейс | Контракт / точка входа | Ограничения |
|---|---|---|
| CLI | `pytyliki / pytyliki-metrics / pytyliki-qcs` | Локальные команды; справка доступна через `--help` и документацию возможностей. |
| Native FFI | `pytyliki_ffi / C ABI v1` | Библиотека выбирается явно; буферы принадлежат вызывающей стороне. |
| Отчёты | Версионированный JSON/text | Контракт отчёта включает происхождение данных и ограничения. |

REST/GraphQL, WebSocket, OAuth, облачный deployment и cookie-механизмы не описываются как возможности, если в репозитории нет версионированной реализации и тестов.

## Безопасность и границы доверия

Запись в прошивку, BIOS/UEFI, микрокод, частоты, напряжение, драйверы или регистры устройств не выполняется. MMIO-описания Embedded C являются только симуляцией/справочной моделью. Для Native FFI требуется явно указанный доверенный путь к библиотеке.

Политика безопасности: [SECURITY.md](SECURITY.md). Не публикуйте секреты, идентификаторы устройств и приватные журналы в открытых задачах.

## Проверка качества

Набор CTest запускается командой из раздела «Сборка»; отдельная команда проверки не требуется.

Состав тестов определяется зарегистрированными CTest/unittest целями и workflow. Процент покрытия, fuzz-результат и нагрузочный отчёт не заявляются без опубликованного артефакта.

## Доказательства производительности

Результаты бенчмарков зависят от хоста и предназначены для локального сравнения. Методы статистики и архивы результатов описаны в документации; универсальные значения задержки, памяти или рейтинга между машинами не заявляются.

## Совместимость

Заявлены runtime-цели Windows и Linux; Win32 GUI работает только в Windows. Переносимыми точками входа являются ядро C11 и CLI.

## Документация и экосистема

- [Feature matrix](docs/FEATURES.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Benchmark statistics](docs/BENCHMARK_STATISTICS.md)
- [Report envelope](docs/REPORT_ENVELOPE.md)
- [Embedded C](docs/EMBEDDED_C.md)
- [QCS](docs/QCS_NATIVE.md)
- [Documentation index](docs/README.md)
- [Reaper integration](docs/REAPER_INTEGRATION.md)

Основные технические источники — матрица возможностей, архитектура, статистика бенчмарков и формат отчётов.

Интеграции между проектами используют документированные версионированные интерфейсы и явные адаптеры. Необязательный адаптер не предоставляет другому проекту права доступа к оборудованию или среде выполнения.

## Участие в разработке

Изучите [CONTRIBUTING.md](CONTRIBUTING.md) перед изменением кода или контрактов. Сохраняйте изменения небольшими и проверяемыми, добавляйте детерминированные тесты, используйте Conventional Commits, если это требуется руководством.

## Лицензия и правовые условия

Распространяется по лицензии [MIT](LICENSE). Программное обеспечение предоставляется «КАК ЕСТЬ» в пределах условий лицензии. Названия сторонних проектов и товарные знаки принадлежат их правообладателям.

Этот репозиторий описывает локальные программные интерфейсы. Условия облачного сервиса, согласие на cookies, OAuth и CORS не относятся к указанным локальным компонентам, пока отдельно не появится документированный сетевой сервис.

---

<p align="center"><sub>Psychoporter 666 README Standard v1.0 · Ivan-VHS · 2026-11-25 · UTC</sub></p>
