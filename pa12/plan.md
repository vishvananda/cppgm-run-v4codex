# PA12 implementation plan

Stage base commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Last reviewed commit: `91e5dbe0a850d79dc5bd911727ae0b89de3c2033`
Target: PA12 full-stage. Phase: implement.

## Design and remaining groups

Extend the shared source graph, canonical TypeId/EntityId facts and typed LowIR.
No source/IR replay, reference delegation, new optimizer or later native gate.

| Group / owner | Data flow and complexity | Validation |
| --- | --- | --- |
| Member declaration/selection | Canonical function ref-qualifier -> indexed declarations -> object conversion/ranking; O(parameters + required candidates) | ref-qualified, out-of-class and rejection fixtures; earlier PAs |
| Special members / semantic construction | Per-class demand states -> selected field/base copy/move actions -> typed lowering; once per demanded member, O(subobjects) | defaulted/deleted, implicit/user copy/move, bit-field units |
| Class ABI and value transfer / lowering | Selected constructors and explicit destinations -> class arguments/results and temporary identities; O(expressions + actions) | direct/indirect ABI, empty objects, return and conditional fixtures |
| Lifetime / semantic actions and cleanup | Full-expression/scope identities -> shared cleanup suffixes; O(actions + CFG edges) | condition, reference, array and cleanup controls |
| Conversions / overload engine | Indexed conversion candidates -> recorded standard/user sequences -> ordinary calls | scalar/pointer/class conversion and ref-ranking cases |
| Delegation, unions, allocation / object model | Constructor dependency edges, variant identity and allocation facts -> existing construction/destruction | constructor/union/new/delete fixtures |

## Performance evidence

PA12 is O0 source-to-LowIR; PA24 owns native generation. Measure frozen compiler
latency/RSS and supplied-backend executable runtime/text where supported. Compare
correct equivalent outputs with A/A calibration and ABBA observations. Necessary
semantic costs are documented, not extra gates; no unsupported numeric target is
inherited from PA11. Preserve bounded array expansion and all contract coverage.
No performance benefit is claimed before measurements.

## Handoff ledger

- Entry: clean HEAD above; provided baseline 61/257 passing, 196 failing.
  PA1–PA11 prior report 1327/1327; fresh checks required before handoff.
- Previous goal turn: no implementation progress evidenced in this session;
  inspected authoritative clean tree and terminal failure log; work resumed.
- Member/constructor increment: canonical ref qualifiers, implicit-object ranking,
  ordinary pointer-reference ranking, delegation edges/cycle checks, union variant
  selection/injected storage, and conservative empty-destructor emission.
  Owner/data flow follows the table; delegation and destructor summaries are
  bounded by the demanded constructor/subobject edges, with TU-local caches.
- Current `make test-pa12`: **85/257** (172 remaining); 26 original fixture
  failures removed, with two using-declaration ranking regressions subsequently
  found by set comparison (corrected in the next increment). PA1–PA11 **1327/1327** and
  file audit pass (existing Analyzer-header advisory). Five personal native/
  rejection reducers pass. Required root reports run serially: concurrent
  reports share a tally and cannot provide authoritative totals.
- Corrected increment: using-declaration ranking uses its introducing class;
  lowering keeps the selected base identity. Pointer-reference temporaries and
  explicit null globals retain the required typed LowIR. Deleted assignment and
  ref-qualified members are rejected when selected.
- Corrected `make test-pa12`: **92/257**, 165 failures. Set comparison proves
  **31 original failures removed, no new failures**. Earlier PAs **1327/1327**;
  file audit and nine personal native/rejection reducers pass.
- Both frozen member campaigns and new-feature absolute measurements completed;
  performance evidence is being consolidated without discarding observations.
- Next: finish pointer-reference materialization details, then introduce typed
  copy/move transfer actions and class-value ABI/lifetime facts. Combined
  ref-qualified/class-result tests and union copying depend on those owners.
- Full-stage implementation continues; no incomplete handoff yet.
