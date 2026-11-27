# OCTA-8 — four-project peer federation

**Status:** shared, validated topology for Reaper, Pytyliki, OPlague and VentuSample. The apex is protocol metadata only, not a central runtime bus.

An octagonal base, eight triangular facets and a non-executable coordination apex define the topology. Four independent products form a complete directed mesh: **12 directed links** and **four self-loops**.

| Direction | Contract | Status |
|---|---|---|
| OPlague → Reaper | `oplage.device-snapshot.v1` | implemented |
| Reaper → OPlague | `reaper.oplage-comparison.v1` | implemented |
| Reaper ↔ Pytyliki | `pytyliki.c-abi.v1` | implemented |
| VentuSample → Reaper | `ventusample.pack.v1` | partial; consumer absent |
| Reaper → VentuSample | `ventusample.render-request.v1` | planned |
| VentuSample ↔ Pytyliki | `pytyliki.c-abi.v1` | planned |
| VentuSample ↔ OPlague | none | not applicable |

Eight facets cover identity, contracts, transport, provenance, validation, privacy, safety and audit. Pytyliki remains the portable C11 numerical provider; OPlague remains read-only; no integration grants driver mutation.

```bash
python tools/validate_octa_pyramid.py
python -m unittest tests.test_octa_pyramid -v
```

[Reaper](https://github.com/Ivan-VHS/Reaper) · [Pytyliki](https://github.com/Ivan-VHS/Pytyliki) · [OPlague](https://github.com/Ivan-VHS/OPlague) · [VentuSample](https://github.com/Ivan-VHS/VentuSample)
