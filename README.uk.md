# Pytyliki — робоча станція процесорних систем

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

**Модульна робоча станція на C11 для локального аналізу процесора, обмежених тестів продуктивності, діагностики коду та PCM16-аудіо/DSP.** Проєкт містить нативний Win32-інтерфейс і CLI.

## Реалізовані можливості

- **Інвентаризація процесора:** модель і виробник CPU, логічні процесори, пам’ять та доступні ISA-можливості з перевіркою стану векторних регістрів ОС.
- **Тести продуктивності:** обмежені процесорні, memory- та matrix-навантаження для локального порівняння. Результати залежать від обладнання, компілятора й ОС.
- **Діагностика коду:** аналіз підтримуваних файлів без виконання чи переписування; експорт JSON-звітів.
- **Аудіо/DSP:** імпорт та експорт PCM16 WAV, генерація тону, low-pass, компресія, сатурація, gain і вимірювання рівня.
- **Каталоги планування:** Syntax-pie, матриця покращень, HyperFusion та APSIA. Це рекомендаційні моделі, а не автономні оптимізатори чи компілятори.
- **Десктоп:** нативний Windows GUI для сканування процесора, аналізу коду й аудіокерування.

## Скріншоти

У репозиторії наразі немає перевіреного знімка запущеного застосунку. Попередній ілюстративний макет інтерфейсу навмисно не видається за справжній скриншот. Додайте сюди захоплення зібраного Win32-застосунку після фактичного запуску.

## Збирання та тести

Потрібні CMake 3.16+, компілятор C11, Git, Windows або Linux.

```bash
git clone https://github.com/Ivan-VHS/Pytyliki.git
cd Pytyliki
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

У Windows також створюється `pytyliki-gui.exe`. Приклади CLI:

```bash
./build/pytyliki --profile
./build/pytyliki --suite
./build/pytyliki --improvement-plan
./build/pytyliki --tone-out test-tone.wav --frequency 440 --seconds 3
./build/pytyliki --json report.json
```

Для типового Windows multi-config build використовуйте `build/Release/pytyliki.exe`.

## Архітектура та безпека

Реалізація — ISO C11, система збирання — CMake/CTest. GitHub Actions перевіряє Windows і Linux; окреме Linux-завдання запускає AddressSanitizer та UndefinedBehaviorSanitizer.

Виявлення процесора працює лише на читання. Застосунок не змінює firmware, BIOS/UEFI, microcode, частоти, напруги, power limits, параметри ядра, драйвери чи регістри пристроїв. Аналіз коду не виконує файли. Бенчмарки не є сертифікацією або гарантією прискорення. Рекомендаційні каталоги — це засоби планування, а не доказ оптимізації.

## Документація

- [Архітектура](docs/ARCHITECTURE.md)
- [Маршрутизація APSIA](docs/APSIA_2.0.md)
- [Механізм апаратних правил](docs/HARDWARE_RULE_ENGINE.md)
- [Таксономія процесорів](docs/PROCESSOR_TAXONOMY.md)
- [Політика перевірки платформ](docs/PLATFORM_VALIDATION_POLICY.md)

## Ліцензія

[MIT](LICENSE).

> 千里之行，始于足下。 — Подорож у тисячу миль починається з першого кроку.
