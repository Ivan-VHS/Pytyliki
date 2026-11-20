# Pytyliki — рабочая станция процессорных систем

<p align="center">
  <a href="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml"><img alt="CI" src="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml/badge.svg"></a>
  <img alt="C11" src="https://img.shields.io/badge/core-ISO%20C11-2563EB?style=flat-square">
  <img alt="Windows и Linux" src="https://img.shields.io/badge/platforms-Windows%20%7C%20Linux-DC2626?style=flat-square">
  <img alt="MIT" src="https://img.shields.io/badge/license-MIT-334155?style=flat-square">
</p>

**Модульная рабочая станция на C11 для обнаружения процессора, ограниченных измерений, статического анализа исходников и локальной DSP-обработки.** В проект входят нативный интерфейс Windows и переносимое CLI. Базовый принцип — исследовать, измерять и формировать отчёт, не изменяя оборудование.

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

## Предпросмотр интерфейса

<p align="center"><img src="assets/pytyliki-workstation-preview.svg" alt="Иллюстративный макет интерфейса Pytyliki Win32" width="100%"></p>
<sub>Схематическая реконструкция по элементам Win32 из исходного кода, а не снимок работающего приложения. Реальный скриншот пока не добавлен.</sub>

## Рабочие модули

- **Инвентаризация CPU:** модель, архитектура, логические потоки, память и ISA-возможности. Для AVX учитывается состояние XCR0, управляемое ОС.
- **Ограниченный бенчмарк:** процессорные, memory- и matrix-нагрузки со временем выполнения и проверкой целостности. Результаты не являются сертификацией оборудования.
- **Статический анализ исходников:** проверка поддерживаемых файлов и экспорт замечаний без компиляции, запуска или перезаписи кода.
- **Audio / DSP:** импорт и экспорт PCM16 WAV, тестовый тон, фильтр низких частот, компрессия, сатурация, усиление и измерение уровней.
- **Ядро памяти:** типизированные представления, проверяемые срезы, ограниченные арены, контрольные точки отката, телеметрия и пулы фиксированных блоков.
- **Метрический движок:** нормы и расстояния векторов, ортогональность, взвешенные квадратичные формы, геометрия и дисперсия.
- **Embedded C:** справочные профили MCU, расчёт FLASH/SRAM, симуляция MMIO с масками и генерация стартового шаблона. Это не физический доступ к регистрам и не прошивальщик.
- **Инженерные каталоги:** 66 записей языковых возможностей, 14 профилей CPU/MCU/виртуальных ISA, маршрутизатор APSIA и лаборатория из 33 системных функций.
- **Интероперабельность:** Pytyliki C ABI v1 для нативных потребителей.
- **QCS frontend:** нативные лексер C11, frame-парсер, компактный AST и CLI для проверки доменов и границ; это анализ синтаксиса, не компиляция или управление оборудованием ([спецификация](docs/QCS_NATIVE.md)).

## Сборка и тесты

Требуются CMake 3.16+, компилятор C11 и Git. CI проверяет Windows и Linux; графический Win32-интерфейс доступен только в Windows.

```bash
git clone https://github.com/Ivan-VHS/Pytyliki.git
cd Pytyliki
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

Полезные команды:

```bash
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --systems
./build/pytyliki --embedded profiles
./build/pytyliki-metrics --demo
./build/pytyliki --json report.json
```

В Windows используйте, например, `build/Release/pytyliki.exe`.

## Развитие проекта

Базовая версия рабочей станции 0.5.0 (16 ноября 2026 года) включала профилирование процессора, анализ исходников и DSP. Дальнейшая работа в main расширила C11-модули и тесты, добавила ядро памяти и метрик, нативный ABI, инструменты Embedded C, каталоги архитектур и языков, системную лабораторию и версионируемый контракт проекта. Реализацию подтверждают текущие исходники и тесты.

## Безопасность и ограничения

Обнаружение CPU работает только на чтение. Бенчмарки ограничены и зависят от оборудования. Анализатор не запускает проверяемый код. DSP обрабатывает только локальные буферы. Оценки и маршруты APSIA носят рекомендательный характер. Запись в BIOS/UEFI, прошивку, микрокод, напряжение, частоты, лимиты питания, драйверы и регистры устройств не выполняется. Поиск компиляторов проверяет только имена в `PATH`.

## Документация

[Архитектура](docs/ARCHITECTURE.md) · [Ядро памяти](docs/MEMORY_ARENA.md) · [Embedded C](docs/EMBEDDED_C.md) · [Атлас языков и архитектур](docs/SYSTEMS_LANGUAGE_ARCHITECTURE_ATLAS.md) · [Единый инженерный стандарт](docs/UNIFIED_ENGINEERING_STANDARD.md)

[Релизы Windows](https://github.com/Ivan-VHS/Pytyliki/releases) · [Сборка desktop](https://github.com/Ivan-VHS/Pytyliki/actions/workflows/desktop-build.yml)

## Лицензия

[MIT](LICENSE)