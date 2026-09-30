# PA25 implementation137 handoff

Stage base commit: fee6ad9076ff35c5272526e1c4c4df235fbf3bfe
Last reviewed commit: fee6ad9076ff35c5272526e1c4c4df235fbf3bfe

Target remains **PA25 full-stage**. Entry `11aee2da`: clean, **71/101**, 30
failures. The previous turn made progress (seven scalar failures removed).
This handoff completes related statement-expression behavior at **74/101**,
27 failures. It does not approve PA25 or authorize advancement.

## Design/spec alignment

The existing streaming frontend, canonical semantic graph, typed LowIR,
per-function MIR, native object and indexed linker remain the production path.
No host compiler, assembler, reference implementation or textual transport is
introduced. PA26/27 still own host-compatible object output.

| Completed owner | Data flow / bounded work | Evidence |
|---|---|---|
| Parser/semantic result | One parsed compound -> scoped statements, selected final conversion and class temporary; O(nodes), canonical IDs | Three original failures fixed; values, decay, scopes, class copies and diagnostics |
| Template/query/effect | Source binding -> canonical occurrence and complete-environment result query; cached declaration construction/destruction effects | Dependent locals/operators/decltype, fixed binding, noexcept, two body transitions in trace |
| Jump/lifetime | Scope entry and lifetime start -> region-aware transfers and sparse skipped-initialization guards | Enclosing returns, goto, break/continue, loop headers, class/reference cleanup |
| Lowering | Recorded facts -> typed LowIR; function-owned overlays map `(region,state)` once, share unchanged prefixes and preserve enclosing temporaries | Nested cleanup and result lifetime; separate/direct/both mixed links; validated LowIR and empty-PATH ELF trace |

Complexity is O(source nodes + required lifetime/control edges + emitted IR),
with average O(1) fact lookup. Function-local vectors and flat indexes are reset
at the next function; source facts remain TU-owned. O0 optional transform and
code-growth budgets are zero. Existing destination copy elision consumes the new
result form without adding a pass. No global cache, whole-program retry, fake
frontend node or grammar replay is added.

Details and extension policy: [ownership137](../student.tests/pa25/statement-expressions137.md).
The trace has 273 parsed nodes, two specializations/body transitions, four lowered
statement regions, two lifetime mappings and 22 hits, five native functions and
1581 text bytes. Earlier driver/scalar ownership evidence remains in
[validation136](../student.tests/pa25/validation136.json).

## Remaining implementation (requirements retained)

| Owner/group | Required failures | Missing data flow |
|---|---:|---|
| Native C++ runtime | 12 | Recorded RTTI/allocation/pure-virtual roles -> support definitions; dynamic cast consumes hierarchy data |
| Source EH/runtime | 14 | Catch/type/payload/lifetime and function-try facts -> matching, unwinding, cleanup and handler state |
| Floating evaluation policy | 1 | Reconcile exact runtime oracle with supported precision; five of one million outputs differ |

Cases and inherited diagnostics: [remaining137](../student.tests/pa25/remaining137.json).
The inherited unused-dependent-local reducer remains separate frontend work.
The floating [reducer/proof discussion](../student.tests/pa25/rounding136.md) still
applies; no reference, coverage or comparison rule was changed.

Boundary: the statement/result/control/lifetime group includes the related
class copies, reference lifetimes, skipped initialization, nested temporaries,
template queries and implicit effects exposed by its controls. There is no known
unresolved defect in that covered group. Further required work needs different
native class/EH/precision owners. In particular, executing source exceptions,
including ones initiated inside statement bodies, still needs the shared EH
runtime; extending statement analysis cannot provide that machinery.

## Performance and required validation

[Performance137](../student.tests/pa25/performance137.md) retains frozen hashes,
flags/inputs and every A/A + six ABBA observations. Equivalent executable pairs
are byte-identical. Template compiler B/A median **1.011**, paired range
**0.986–1.019**; peak RSS growth under **1.9%** across the fixed workloads.
Short memory/floating compile batches include startup and remain diagnostics.
No speedup is claimed; runtime spread and the floating compiler outlier remain
reported. The new statement baseline executes three million checked calls and
destructions: 64-compile batch **0.38425 s**, compiler peak **6832 KiB**, runtime
batch (three runs) **0.12588 s**, text **503 bytes**. Inherited 15% diagnostics add
no exit gate; mandated correctness, coverage and finite bounds remain. PA34 owns
self-hosting.

- `make test-pa25`: **74/101**; three original failures removed, none added.
- `make test-report-through-pa24`: **4152/4152**, all 24 stages pass.
- `make test-report-through-pa25`: **4226/4253**, no advancement claimed.
- File audit passes with four inherited header warnings; `git diff --check` clean.
- Explicit controls: **49 statement**, **61 driver**, **19 scalar**, including
  96 randomized full-width operand pairs. Empty-PATH template/cleanup trace runs;
  explicit AST and validated LowIR views pass.
- Intermediate build/audit/control failures are retained. Query dispatch was
  moved to its scoped owner to satisfy the function-size audit; final checks
  were rerun on the committed production source.

## Handoff ledger and independent review

| Increment | Disposition |
|---|---|
| `cf26b48b` | Driver handoff: 64/101; independent review pending |
| `11aee2da` | Scalar handoff: 71/101; independent review pending |
| `832d2d85` | Current ownership plan; both review markers preserved |
| `2705e57b` | Statement/result/query/control/lifetime implementation and controls |
| Evidence handoff | Final validation, frozen performance, trace and remaining-work ledger |

This is the next handoff after two accepted checkpoints. Independent review
remains pending for whole-stage source-to-ELF architecture, object/weak ownership,
scalar ABI/effects, new lifetime overlays/guards/query environments, runtime
boundaries, precision policy and performance acceptance. Review questions are
separate from unfinished implementation above; neither is waived.

[Validation137](../student.tests/pa25/validation137.json) records source/fixture
manifests, checks and evidence hashes. Artifacts:
`/home/vishvananda/work/private/v4codex/artifacts/pa25-137/`.
This committed implementation handoff returns control to Ralph for audit and
further work; it does not certify the assignment.
