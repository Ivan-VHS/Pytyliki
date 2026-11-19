# Pytyliki — рабочая станция процессорных систем

<p align="center">
  <a href="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml"><img alt="CI Windows и Linux" src="https://github.com/Ivan-VHS/Pytyliki/actions/workflows/ci.yml/badge.svg"></a>
  <img alt="ISO C11" src="https://img.shields.io/badge/C-ISO%20C11-2563EB?style=flat-square">
  <img alt="Windows и Linux" src="https://img.shields.io/badge/платформы-Windows%20%7C%20Linux-DC2626?style=flat-square">
  <img alt="Лицензия MIT" src="https://img.shields.io/badge/лицензия-MIT-111827?style=flat-square">
</p>

<p align="center"><strong>ОБНАРУЖЕНИЕ · АНАЛИЗ · ОБРАБОТКА · ПРОВЕРКА</strong></p>

<p align="center"><img src="assets/pytyliki-hero.svg" alt="Рабочая станция процессорных систем Pytyliki" width="100%"></p>

Pytyliki — модульная рабочая станция на C11 для локальной диагностики процессора, ограниченных бенчмарков, статического анализа исходников и обработки PCM16-аудио. В проект входят нативный настольный интерфейс Win32, CLI и проверяемый инженерный каталог.

**Языки:** [English](README.md) · Русский · [Українська](README.uk.md) · [简体中文](README.zh-CN.md)

## Визуальный обзор

<p align="center"><img src="assets/pytyliki-workstation-preview.svg" alt="Превью Win32-интерфейса Pytyliki по исходному коду" width="100%"></p>
<p align="center"><img src="assets/pytyliki-audio-meters-preview.svg" alt="Превью независимых стереоизмерителей аудио" width="100%"></p>
<p align="center"><img src="assets/pytyliki-architecture.svg" alt="Обзор архитектуры Pytyliki" width="100%"></p>

<sub>В репозитории находятся SVG-превью, построенные по исходному коду, и схема архитектуры. Это не фотографии и не скриншоты реально запущенного приложения Windows. Для настоящего снимка Win32-интерфейса нужна Windows-сессия рабочего стола.</sub>

## Возможности рабочей станции

| Область | Реализованные возможности |
|---|---|
| Инвентаризация процессора | Идентификация CPU, архитектура, логические процессоры, память и доступные ISA-возможности |
| Бенчмарки | Ограниченные нагрузки CPU, памяти и матриц для локального сравнения |
| Анализ исходников | Статическая диагностика поддерживаемых файлов без их выполнения или переписывания |
| Аудио / DSP | Импорт/экспорт PCM16 WAV, генерация тона, фильтрация, компрессия, сатурация и gain |
| Измерение каналов | Независимые peak, RMS, DC offset, crest factor и число клиппированных отсчётов для каждого канала |
| Windows GUI | Нативная Win32-рабочая станция с единой бело-красно-синей темой |
| Инженерный каталог | Syntax-pie, CPU Suite Pro, Forge66, TriMatrix, HyperFusion, APSIA и рекомендательные аппаратные правила |
| Проверка | CMake/CTest, CI для Windows и Linux, Linux ASan/UBSan и read-only HMGA-аудит |

## Быстрый старт

### Требования

- Windows или Linux
- CMake 3.16+
- Компилятор с поддержкой C11
- Git

### Сборка и тесты

    git clone https://github.com/Ivan-VHS/Pytyliki.git
    cd Pytyliki
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
    cmake --build build --config Release --parallel
    ctest --test-dir build -C Release --output-on-failure

В Windows-сборке также создаётся нативная цель pytyliki-gui.exe.

### Полезные команды

    ./build/pytyliki --profile
    ./build/pytyliki --suite
    ./build/pytyliki --improvement-plan
    ./build/pytyliki --systems
    ./build/pytyliki --tone-out test-tone.wav --frequency 440 --seconds 3
    ./build/pytyliki --json report.json

В Windows используйте путь к исполняемому файлу вашей конфигурации сборки, например build/Release/pytyliki.exe.

## Объединённые инженерные обновления

Ветка интеграции объединяет совместимые изменения из существующей разработки:

- **Измерение аудио:** независимые peak/RMS/DC/crest/clipping для каждого канала и вывод в CLI/Win32.
- **Безопасность WAV:** проверка границ RIFF, заголовков чанков, данных и байтов выравнивания до обработки.
- **Единый таймер:** процессорные измерения и бенчмарки используют общие монотонные миллисекунды; добавлен отдельный контрактный тест.
- **HMGA v2:** read-only аудит репозитория, тесты и ограниченная инженерная политика.
- **HMGA v3:** детерминированный каталог на 20 аксиом и 400 дескрипторов C11 с пределом в 64 раунда планирования.
- **Win32-тема:** централизованные белые, красные и синие цветовые токены и owner-drawn элементы управления.
- **CPU Suite:** упрощён и сделан понятнее код поиска модулей по ID и имени; поведение поиска не изменено.

Исторические ветки проверялись, а не копировались целиком. Уже существующие в main функции сохранены без дублирования, пересекающиеся прототипы не создают параллельные API. Подробная сверка приведена в [аудите интеграции](docs/INTEGRATION_AUDIT_2026-11-19.md).

## CI, безопасность и границы проекта

GitHub Actions собирает и тестирует проект на Ubuntu и Windows. В Linux выполняются тесты с AddressSanitizer и UndefinedBehaviorSanitizer. HMGA-аудит проверяет инженерные артефакты и правила репозитория, но не переписывает исходники.

- Диагностика процессора работает только на чтение. Записи в firmware, BIOS/UEFI, частоты, напряжение, лимиты мощности, ядро ОС, драйверы и регистры устройств не выполняются.
- Анализ исходников не запускает и не изменяет проверяемые файлы.
- Результаты бенчмарков зависят от оборудования, компилятора, ОС и нагрузки; это не сертификация и не гарантия ускорения.
- HMGA v3 перечисляет ограниченные плановые дескрипторы, но не генерирует и не выполняет исходный код.
- Настольный интерфейс реализован на Win32 для Windows. Поддержка macOS и гарантированная производительность ARM64/x86_64 не заявляются.

## Документация

- [Архитектура](docs/ARCHITECTURE.md)
- [Инженерный стандарт](docs/ENGINEERING_STANDARD.md)
- [Контракт HMGA v2.0](docs/HMGA_V2.0.md)
- [Матрица операторов HMGA v3.0](docs/HMGA_V3.md)
- [Аудит интеграции](docs/INTEGRATION_AUDIT_2026-11-19.md)
- [Маршрутизация APSIA 2.0](docs/APSIA_2.0.md)
- [Движок аппаратных правил](docs/HARDWARE_RULE_ENGINE.md)
- [Таксономия процессоров](docs/PROCESSOR_TAXONOMY.md)
- [Политика проверки платформ](docs/PLATFORM_VALIDATION_POLICY.md)


- [Инженерный профиль AETHER-ANAS](docs/AETHER_ANAS_INTEGRATION.md)
- [Контракт Architecture 66](docs/ARCHITECTURE_66.md)
- [Профиль соответствия 666 правилам](docs/CONSTITUTION_666_PROFILE.md)
- [Инженерная сверка 100 китайских пословиц](docs/CHINESE_PROVERBS_100_ENGINEERING_CROSSWALK.md)
- [Профиль цели MCST Elbrus E2K](docs/ELBRUS_E2K.md)

## Лицензия

Проект распространяется по [лицензии MIT](LICENSE).
