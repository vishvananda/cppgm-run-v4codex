# PA12 implementation plan

Stage base commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Last reviewed commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Target: PA12 full-stage. Phase: implement; incomplete.

## Design/spec alignment

Extend the shared source graph, canonical TypeId/EntityId facts and typed LowIR.
No source/IR replay, reference delegation, new optimizer or later native gate.
The completed group owns canonical ref qualifiers, using-object ranking, deleted
member selection, pointer-reference materialization, delegation edges and union
variant/storage identities. Existing constructor actions and ABI adapters consume
those facts; synthesized copy/move transfers will extend the same model.

| Group / owner | Data flow and complexity | Validation / status |
| --- | --- | --- |
| Member declaration/selection | Canonical qualifier -> indexed declaration -> object/reference conversion; O(parameters + required candidate/base edges) | Focused ref-qualification, using, deletion and pointer-reference fixtures pass |
| Delegation/unions / construction | Selected constructor edges and variant/storage IDs -> existing actions; once per demanded edge/subobject | Delegation/cycles, variant initialization, injection and destructor-boundary fixtures pass |
| Special members / semantics | Per-class demand/deletion state -> field/base copy/move actions -> typed lowering; O(subobjects) | Remaining: implicit/defaulted/user transfers, empty objects, bit-field units |
| Class ABI / lowering | Selected transfer + explicit destination -> value arguments/results; O(expressions + actions) | Remaining: direct/indirect ABI, named return slots, conditional values |
| Lifetime / actions and cleanup | Full-expression/scope identities -> shared cleanup suffixes; O(actions + CFG edges) | Remaining: local/static references, condition declarations, array/temporary controls |
| Conversions / overload engine | Indexed conversion functions -> recorded user/standard sequences -> calls | Remaining: scalar/pointer/class conversions and their combined ref-qualifier cases |
| Allocation / object model | Allocation choice and storage -> construction/destruction actions | Remaining: scalar/array new/delete and nonthrowing-null paths |

## Performance evidence

[Evidence](performance.md) retains both frozen A/B campaigns, A/A noise, ABBA
pairs, spread, latency/RSS, runtime/text, hashes and absolute new-feature costs.
Corrected common compiler medians increase 1.12%/0.21%; native LowIR/text is
identical. The empty-destructor loop runs about 52% faster, text 928 -> 241 bytes.
New 4x member/delegation inputs use 4.04x time and 3.62x RSS; demanded work scales
linearly. No unsupported numeric gate is inherited. O0 work is bounded by required
candidate/subobject edges; empty-call removal has zero generated-code growth.
Preserve the eight-element array expansion limit and all course coverage.

## Handoff ledger

- Entry: clean stage base, terminal failure log; no live prior work handle.
  Previous goal-turn progress was unverified; authoritative inspection resumed work.
  Baseline **61/257**, 196 failures; earlier PAs **1327/1327**.
- `4e209677`: member qualification, delegation, unions and destructor boundaries.
  Checkpoint 85/257; failure-set audit found 26 old failures removed and two
  using-overload regressions. Initial measurements are preserved as historical.
- `55d15ba8`: correct using-owner ranking, pointer-reference output and deleted
  member/assignment selection. Final **92/257**, 165 failures: **31 original
  failures removed, no new failures**; comparison coverage is unchanged.
- Required `make test-pa12`: fails at the remaining groups above. Fresh serial
  `make test-report-through-pa11`: **1327/1327**. File audit passes with the
  existing Analyzer-header advisory. Nine explicit personal checks pass (four
  native successes, five required rejections). `git diff --check` passes.
- Serial root reports are required for trustworthy counts: concurrent reports
  share a tally. The invalid concurrent totals were replaced by serial runs.
- Handoff boundary: declaration/object-selection and delegation/union-initialization
  work is closed. The remaining combined cases require class transfer records,
  helper demand/deletion rules and direct/indirect class ABI destinations together;
  current same-class values still fall into aggregate/scalar lowering. Continuing
  with isolated call-site patches would lack constructor and lifetime ownership.
  Next implement that shared transfer model, then attach temporary/scope cleanup.
  This is an incomplete behavior-group handoff; the full-stage goal stays active.
