# Pytyliki MOS Switch-Level Model v1

**Schema:** `pytyliki.mos-switch-model.v1`  
**Implementation:** bounded native C11  
**Status:** first deterministic switch-level evaluator

## Purpose

The MOS Switch-Level Model gives Pytyliki a native representation below Boolean gates: N-channel and P-channel MOSFETs, gate-control levels, source/drain connectivity, supply rails and resolved electrical states.

It is a switch-topology model. It is not an analog semiconductor solver and it does not claim transistor-level SPICE accuracy.

## Model

A network has a fixed number of node identifiers, one VDD node, one VSS node and up to 256 MOS switch records. Every device stores three node indices and a channel kind.

- An N-channel device is definitely on when its gate level is HIGH.
- A P-channel device is definitely on when its gate level is LOW.
- An UNKNOWN gate level makes the device a possible connection, not a definite one.
- Source and drain are symmetric ideal-switch terminals in this model.

The evaluator computes definite and possible connectivity from both supply rails. Each node resolves to one of five states:

| State | Meaning |
|---|---|
| `high` | Definitely connected to VDD, with no possible VSS path |
| `low` | Definitely connected to VSS, with no possible VDD path |
| `contended` | Definitely connected to both rails |
| `floating` | Has no definite or possible path to either rail |
| `indeterminate` | An UNKNOWN-controlled device may connect the node to a rail |

An unknown gate therefore cannot silently turn into a zero or one. A possible short to the opposite rail prevents a false HIGH/LOW result.

## Example: CMOS inverter

Node assignment:

| Node | Role |
|---:|---|
| 0 | VDD |
| 1 | VSS |
| 2 | input |
| 3 | output |

Devices:

- P-MOS: gate 2, terminals 0 and 3
- N-MOS: gate 2, terminals 3 and 1

With input LOW, the output is HIGH. With input HIGH, the output is LOW. With an UNKNOWN input, the output is INDETERMINATE.

Run the native demonstration:

```sh
pytyliki-mos --inverter low
pytyliki-mos --inverter high
pytyliki-mos --inverter unknown
```

The output is a model result, never a measurement from a physical chip.

## Determinism and resource bounds

- C11 implementation; no dynamic allocation.
- Maximum 128 nodes and 256 MOS switch records per network.
- Caller-owned network, input-level and output-state storage.
- No hidden threads, network access, file access or global mutable state.
- Invalid topology and insufficient buffers are rejected before output states are changed.
- Evaluation is deterministic for a fixed topology and input-level vector.

The evaluator uses bounded graph reachability. Its work is bounded by the fixed node and device limits.

## Explicit limits

Version 1 does not calculate:

- gate threshold voltage or continuous terminal voltages;
- drain current, transconductance, channel resistance or short-channel effects;
- body effect, body-diode behavior, leakage or charge injection;
- capacitance, rise/fall time, propagation delay or clock-to-Q;
- temperature, process corners, noise, metastability or analog feedback;
- dynamic circuit fixed points or arbitrary sequential behavior.

Gate levels are supplied by the caller. The model resolves one switch network from those levels; it does not infer internal gate voltages by solving a complete circuit. The interchange envelope is described by `schemas/pytyliki.mos-switch-model.v1.schema.json`; no JSON importer or exporter is shipped in v1. Consumers must additionally ensure that `gate_levels` has exactly `node_count` entries, every node index is below `node_count`, and VDD and VSS are distinct.

It does not access MMIO, physical memory, CPU registers, DMA, firmware or connected devices.

## Evolution path

The next compatible layers should be separate versioned capabilities:

1. **Logic and sequential elements:** explicit gates, latches, flip-flops, setup/hold contracts and clock domains.
2. **Electrical parameter model:** units, voltage rails, threshold parameters, resistance and capacitance with documented compact-model assumptions.
3. **Timing engine:** event queues, propagation delays, clock constraints and deterministic traces.
4. **Netlist and hierarchy:** typed circuit nodes, instances, ports, subcircuits and source maps.
5. **Hardware IR:** a target-independent, versioned representation that can be checked before RTL or target-specific lowering.

A future analog solver must be a separately validated implementation. It must not be implied by this switch-level model.
