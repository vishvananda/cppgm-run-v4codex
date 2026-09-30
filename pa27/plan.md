# PA27 implementation

Stage base commit: `f833cf1ff361529147361cada33eca62e55330cf`
Last reviewed commit: `f833cf1ff361529147361cada33eca62e55330cf`
Target: **PA27 full-stage**. Phase: **implement**.
Entry: **116/158**, 42 failures; PA1–PA26 previously pass. Review markers
remain unchanged until independent audit.

## Design and remaining groups

| Group / owner | Data flow and work bound | Validation |
|---|---|---|
| Host ELF coalescing, imported addresses, named sections / native image and ELF writer | Consume typed symbol binding, placement and fixup ownership; partition bytes once, remap offsets and symbols once. O(bytes + relocations + symbols), with sorted extent lookup at most O(log symbols). No text transport or host code generation. | COMDAT/body+relocation inspections, GOT/PCREL, section controls, host and own links. |
| Emission demand and linkage / semantic facts, lowering | Internal reachability, extern-template declarations, C linkage and TLS identities must survive by entity identity. Deduplicated demand edges. | pruning, internal template argument, extern-template, C and TLS fixtures. |
| ABI spelling / typed PA9 mangler | Dependent result/NTTP types, substitutions and anonymous typedef identity. Reuse canonical semantic facts, no reconstructed textual semantics. | spelling and host-helper fixtures. |
| Object lifetime / semantic construction and lowering | Anonymous storage initialization, move bodies, virtual-base construction tables. Consume recorded layout/lifetime facts. | runtime and system-include fixtures. |

Implement the connected object-emission group first and extend related fixes as
evidence supports them. Other groups remain implementation work, not audit-only
questions. No fixture/reference/comparison changes are planned.

## Performance evidence

Freeze entry/final binaries and fixed personal inputs. Record A/A calibration
and ABBA compiler latency/RSS, checked executable runtime and text together.
PA27 semantic object-format costs are necessary; no optional optimizer or new
growth/search policy is introduced. Inherited blanket percentage/zero-growth
diagnostics are not stage gates (spec section 9). Preserve observations and
investigate avoidable regressions; all mandated limits and coverage remain.

## Handoff ledger

- Entry145: inspected actual clean HEAD, contract, spec, baseline errors and
  symbol/relocation writer. Previous turn supplied stage evidence; classified
  as progress. No prior running compiler process is assumed live.
- Implementation: pending. Required stage, prior-through and file audit must be
  rerun before handoff; independent architectural/performance review remains.
