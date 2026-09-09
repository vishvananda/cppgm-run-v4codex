# PA10 implementation

Stage base commit: c2a4786ebe60f52a7384c8c6e1ce12bd7e0b173a
Last reviewed commit: c2a4786ebe60f52a7384c8c6e1ce12bd7e0b173a

## Design and ownership

The parser/semantic analyzer owns the single source graph, canonical types,
declaration identities, selected calls and conversions. PA10 consumes these
facts directly into PA8's typed Program; PA9's typed encoder owns external
names. No textual phase transport or repeated overload resolution.
Translation-unit vectors indexed by EntityId/NodeId own lowering mappings;
function-local builders own value/slot/block IDs and release transient state
after each body. Work is proportional to semantic nodes and produced IR.

## Remaining groups

- Driver, ABI adapter, typed value/slot construction; scalar expressions and
  conversions. Validate arithmetic, boolean, casts and enum fixtures together.
- Statement CFG, short circuit, loops, switch and condition declarations.
- Resolved direct/indirect calls, defaults and reference storage boundaries.
- Constant globals, array layout/initialization, scaled pointers, volatile
  accesses and memory builtin metadata. Preserve facts for later object lowering.

## Performance evidence

O0 adds required lowering only, with no optional optimization passes. Measure
latency/RSS and IR work on fixed growing procedural workloads; execute eligible
outputs through the allowed reference backend and report runtime/text size.
The stage base produces no LowIR: it is not a semantically equivalent A/B
baseline. No optimization benefit is claimed without the spec's frozen ABBA,
A/A and equivalent-output protocol. Later template/backend gates are deferred
to their owning stages; all mandated current-stage constraints remain.

## Handoff ledger

- Entry: clean base above; prior turn supplied evidence of the unimplemented
  driver (progress evidence, not a live wait). Baseline 0/121, 121 failures.
  Earlier stages and file audit reported passing; rerun at exit.
- Active implementation; no handoff boundary reached. No reference revisions.
