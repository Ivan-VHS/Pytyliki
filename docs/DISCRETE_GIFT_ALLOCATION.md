# Discrete Gift Allocation Core

Pytyliki exposes a C11 API for exact integer allocation, small combinatorial calculations, bounded candy-basket planning and deterministic greeting routing.

## API

Include `gift_planner.h` and link `pytyliki_core`.

- `pl_gift_split_evenly`: divides a whole-unit budget; indivisible remainder goes to the lowest indexes first.
- `pl_gift_split_weighted`: largest-remainder proportional allocation; tie-breaking is stable by input order and all units are conserved.
- `pl_gift_percent`: integer percentage with floor semantics.
- `pl_gift_count_combinations`: exact (n)-choose-(k), rejecting values outside the 64-bit result range.
- `pl_gift_build_candy_basket`: deterministic greedy selection using preference/price ratio, stock and a hard budget ceiling. This is intentionally documented as a heuristic, not an optimal knapsack solver.
- `pl_gift_build_greeting_graph`: complete directed sender/recipient bipartite links with deterministic template rotation.

All currency is supplied as integer units. Pass hryvnias for whole-UAH planning or kopiykas for penny-accurate accounting. The API never uses floating-point values for money.

## Example

A budget of 500 split between five recipients yields five shares of 100. A 2:1:1 weighting yields 250, 125 and 125. Twenty-five percent of 500 is 125. Choosing two items from a set of five has ten possible combinations.

## Safety and limits

The API has fixed upper bounds for recipient, inventory and greeting-edge counts. The basket heuristic never spends more than the supplied budget and respects each stock ceiling. It does not query live shop prices, make purchases or claim that an assortment is globally optimal.

## Validation

The `discrete-gift-planner` CTest target covers budget conservation, proportional rounding, percentages, combinations, inventory limits and greeting-graph determinism.
