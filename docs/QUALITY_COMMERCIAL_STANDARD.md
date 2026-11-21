# Quality, execution and commercial readiness

## Scope

Pytyliki is a read-only processor discovery, bounded measurement, source diagnostics, memory and DSP workstation. The matrix turns the supplied 20 term triplets into traceable product controls.

## 20 × 3 control matrix

| # | Quality | Execution | Commercial | Acceptance intent |
|---:|---|---|---|---|
| 1 | Качественность | Качество | Коммерческий | Приёмка каждой возможности опирается на критерии и проверяемые результаты. |
| 2 | Добротность | Добротность | Торговый | Долговечность обеспечивается ограничениями, обработкой ошибок и сохранностью данных. |
| 3 | Безупречность | Безупречность | Предпринимательский | Опасные и неоднозначные действия исключаются или явно ограничиваются. |
| 4 | Совершенство | Совершенство | Рыночный | Изменения воспроизводимы, регрессионно проверяемы и пригодны к выпуску. |
| 5 | Цельность | Мастерство | Деловой | Модули, интерфейсы и назначение образуют цельный продукт. |
| 6 | Целостность | Профессионализм | Финансовый | Доверие обеспечивают происхождение данных, контроль доступа и границы ответственности. |
| 7 | Монолитность | Надёжность | Прибыльный | Ключевые операции предсказуемы в поддерживаемых нагрузках. |
| 8 | Согласованность | Основательность | Доходный | Выводы обоснованы измерениями и прослеживаемыми источниками. |
| 9 | Последовательность | Гармоничность | Экономический | Повторяемые операции имеют единый порядок и контролируемые затраты. |
| 10 | Системность | Согласованность | Хозяйственный | Политики, конфигурация и системные зависимости согласованы. |
| 11 | Структурированность | Цельность | Бизнесовый | Структура продукта и контрактов понятна для интеграции. |
| 12 | Упорядоченность | Целостность | Маркетинговый | Пользовательское представление объясняет ценность, ограничения и сценарии применения. |
| 13 | Непротиворечивость | Монолитность | Клиентский | Документация, API, тесты и поведение не противоречат друг другу. |
| 14 | Однородность | Нераздельность | Сбытовой | Путь пользователя от задачи до результата остаётся связным. |
| 15 | Унификация | Единство | Промышленный | Форматы и интеграции используют стабильные повторяемые правила. |
| 16 | Стандартизация | Слаженность | Корпоративный | Релизы и изменения проходят повторяемые контрольные процедуры. |
| 17 | Полнота | Ритмичность | Инвестиционный | Доказательств достаточно для независимой технической и финансовой проверки. |
| 18 | Точность | Размеренность | Монетизируемый | Монетизация требует отдельного подтверждения ценности, прав и затрат. |
| 19 | Выверенность | Синхронность | Рентабельный | Рентабельность опирается на измеренные затраты, а не предположения. |
| 20 | Профессионализм | Упорядоченность | Капитализированный | Активы продукта описаны, лицензированы и пригодны к сопровождению. |

## Evaluation

The root profile **quality-commercial.v1.json** records all 60 controls. Run **python tools/quality_commercial.py** for a text report or add **--format json** for a machine-readable report. The workflow runs the evaluator and its regression tests on relevant changes.

Maturity is explicitly declared per control: **implemented** = linked implementation evidence; **planned** = planned and receives 25% credit; **not_assessed** = no evidence and receives no credit; **not_applicable** = excluded from the denominator. The evaluator checks profile structure and repository-relative file presence only. It does not run the linked tests or certify the product.

## Commercial gates

- Market validation: not assessed; require observed user workflows and customer feedback.
- Pricing: not set; do not invent price points or willingness-to-pay.
- Unit economics: not assessed; measure distribution and support costs before claims.
- Licensing: manual review required for dependencies and distribution.
- Support: planned; define maintenance ownership, support channel and lifecycle.
- Distribution: GitHub release/build artifacts are present; this does not prove sales.

## Product boundaries

Processor discovery and source review remain read-only; benchmark results are bounded local comparisons, not hardware certification. DSP runs on local buffers. Hardware-rule scores and routes remain advisory. Export, sharing and telemetry retain consent-aware controls. No firmware, microcode, voltage, clock, driver or device-register write path is added by this framework.

A control may move to **implemented** only when linked artifacts exist and a regression test covers the behavior. Commercial states advance only when independently reviewable customer, financial, licensing or support evidence is available.
