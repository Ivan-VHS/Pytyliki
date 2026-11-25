# Pytyliki — UI/UX Profile — Stage 0.5

**Stage convention:** [Product Stage Naming Standard](https://github.com/Ivan-VHS/DuckOS/blob/main/docs/STAGE_NAMING_STANDARD.md).  
**Shared standard:** [Duck UI/UX Foundation — Stage 0.1](https://github.com/Ivan-VHS/DuckOS/blob/main/docs/UI_UX_STAGE_0_1.md).

This file keeps only the Win32 toolkit notes and implementation evidence. Common interaction rules, accessibility budgets, theme/component contracts and the definition of done live in the shared specification.

### Pytyliki — локальная реализация

- Win32-контролы сохраняют tab-stop, явные подписи, крупные точки попадания для частых действий и доступный focus rectangle.
- Контраст рассчитывается для нативной светлой/тёмной темы; при недостаточном отношении выбирается проверенный текстовый цвет, а не более яркая декоративная заливка.
- Точные числовые поля, пределы DSP, единицы измерения и отчёт остаются видимыми и не округляются ради экономии места.
- UI не входит в C11 core, DSP, FFI, benchmark, PCM и worker paths; никакой визуальный эффект не влияет на детерминизм обработки.
- BPM/transport, телеметрия и прогресс отображаются только при реальном источнике данных; отсутствие источника должно быть явно подписано.

## Implementation evidence — Pytyliki

The C11 UI policy test checks relative luminance, WCAG ordinary/large-text thresholds and the preferred 44 px target token. The helper is header-only and used only by the Win32 presentation layer; it is not linked into the DSP, FFI or processor core. Windows/Linux build checks do not replace manual keyboard-only or 200% scaling review of the native workstation.
