# PA25 implementation137 in progress

Stage base commit: fee6ad9076ff35c5272526e1c4c4df235fbf3bfe
Last reviewed commit: fee6ad9076ff35c5272526e1c4c4df235fbf3bfe

Target remains **PA25 full-stage**. Entry `11aee2da`: clean, **71/101**,
30 failures. The previous turn made progress: seven original failures were
removed with committed scalar behavior and validation. Both review markers above
remain unchanged. This implementation turn starts with GNU statement expressions.

## Current implementation group

| Owner | Data flow / complexity | Validation planned |
|---|---|---|
| Parser and semantic expressions | One parsed compound region -> scoped statements plus recorded final value conversion and temporary; O(nodes), canonical occurrence facts | Original three failures, result/void/decay, nested scopes, templates and diagnostics |
| Lowering/control/lifetime | Recorded statement/value facts -> typed LowIR; enclosing return/break/continue/goto edges and nested full-expression cleanup; O(statements + emitted cleanup edges) | Runtime side effects, class copies/destruction, unreachable final value, direct/separate/mixed parity |

Frozen baseline: artifacts/pa25-137/compiler-A. O0 optional transform and growth
budgets remain zero. Measure equivalent compiler/executable workloads using the
existing A/A and ABBA protocol; new behavior gets a correct final/final baseline.
Required checks and handoff evidence are pending. The groups and independent
review questions retained below are not waived.

## Design/spec alignment

The inherited driver streams each source TU into the canonical semantic graph,
constructs typed LowIR directly, selects/releases per-function MIR and writes
native bytes. Internal PA25 objects retain native ABI facts and relocations;
the indexed linker imports/releases each object. No production host/compiler,
assembler, reference, textual IR transport or source-dependent object is added.
PA26/27 still own host-compatible object output.

| Completed owner | Data flow and bounded work | Validation |
|---|---|---|
| Syntax/type system | Wide fundamental vocabulary -> canonical TypeIds, ranks, signedness and conversions; fixed-width operations O(1). A memoized angle probe distinguishes a member comparison from inferred template syntax. | All six original wide failures pass; arithmetic/conversion and relational controls |
| Semantic scalar constants | TU-owned interned 128-bit payloads -> typed constants, templates, static data, bounds, enum values, switches and bit-fields. Zero has ID 0; complete payload equality resolves hash collisions. Common Constant size stays fixed. | 96 seeded full-width operand pairs, constexpr overflow rejection, high-half globals/templates/fields, separate and mixed linking |
| ABI/lowering | Recorded type/value identities -> full-width ABI literals and LowIR operands -> existing PA24 native pair operations/ABI. Names are rendered only at output. Normalized ABI fact adapters share the encoder. | Cross-TU template identity, ABI symbol/fact checks, wide stack arguments; all earlier PA9/24 checks pass |
| Floating builtins | Semantic NaN/infinity/classification facts -> constants or typed comparisons; lowering does not resolve runtime names. Payload strings are read once, bounded by source length; classification has constant work. | Required isnan/nanl case; three widths, templates, constexpr propagation, noexcept and single argument evaluation |

New payload pools have Analyzer/TU lifetimes; ABI nodes remain compact and
interned. No global mutable cache, optional optimization, fixed-point pass or
unbounded search is introduced. Constant-pool telemetry observes existing work.
The empty-PATH trace visits 20 unique wide constants, two specializations (one
primary body transition and one explicit specialization selection), five native
functions and 2538 text bytes. See [validation136](../student.tests/pa25/validation136.json).

## Remaining implementation (requirements retained)

| Owner/group | Required failures | Next data flow |
|---|---:|---|
| Native C++ runtime | 12 | Recorded RTTI/allocation/pure-virtual roles -> native support definitions; dynamic-cast must consume hierarchy data |
| Source EH/runtime | 14 | Catch/type/payload/lifetime and function-try facts -> matching, unwinding, cleanup and handler state |
| GNU statement expressions | 3 | Parsed statement result, enclosing control flow and lifetime facts -> semantic construction and lowering |
| Floating evaluation policy | 1 | Reconcile exact source-runtime oracle with supported floating precision; five of 1M outputs differ |

Case identities and original diagnostics: [remaining136](../student.tests/pa25/remaining136.json).
The inherited unused-dependent-local reducer is still rejected; it remains
frontend implementation work outside the required fixtures.

Boundary: the scalar group includes the payload consumers, template/ABI views,
bit-fields and builtin effects exposed by its controls, with no known unresolved
correctness defect in that group. Further required work now needs distinct
class-runtime, EH, or statement-expression owners. The calculator requires a
floating-evaluation policy across source/native contracts: its five reference
values are reproduced by extended intermediates permitted by C++11 [expr]/12.
[The reducer/proof discussion](../student.tests/pa25/rounding136.md) explains why
host agreement cannot justify a reference correction. The failing comparison
remains required; no oracle, coverage or comparison rule was changed.

## Performance and validation

[Performance136](../student.tests/pa25/performance136.md) records frozen binaries,
flags/input hashes, A/A plus six ABBA blocks, all samples and paired spreads.
Equivalent template/memory/floating compiler medians B/A: 1.010 / 0.955 / 0.930;
RSS growth under 1.1%; all executable pairs byte-identical. No speedup claim.
The new wide surface has a correct final/final baseline: 64 compiles in 0.40750 s,
6764 KiB compiler peak RSS; three checked executions in 0.29249 s, 977 text bytes.
O0 optional work/growth budgets are zero; new semantic work is linear in source
and unique facts. Inherited 15% diagnostics create no extra gate; mandated
correctness, coverage and finite bounds remain unchanged. Self-hosting is PA34.

- `make test-pa25`: **71/101**, 30 original failures remain; seven removed, none added.
- `make test-report-through-pa24`: **4152/4152**, 24 stages pass.
- `make test-report-through-pa25`: **4223/4253**; no advancement claimed.
- File audit passes with four inherited header warnings; `git diff --check` clean.
- Explicit driver controls: **61 pass**. Scalar controls: **19 pass**, including
  96 randomized full-width operand pairs. Empty-PATH source/template-to-ELF passes.
- A telemetry size-limit finding was fixed by moving constant reporting to its
  owner; authoritative checks were rerun. Intermediate evidence is retained.

## Handoff ledger and independent review

| Increment | Disposition |
|---|---|
| `cf26b48b` | Prior driver handoff: 64/101; review still pending |
| `da31a876` | Current ownership/scope plan; both review markers preserved |
| `1cc95b47` | Canonical wide source scalars and complete payload consumers |
| `bcdd0b72` | Floating builtins, full-width ABI adapters and controls |
| `ba608c4f` | Constant telemetry ownership and precision boundary documentation |
| Evidence handoff | Final required checks, performance and remaining-work ledger |

Independent review remains pending: whole-stage source-to-ELF architecture,
object/weak-definition ownership, runtime boundaries, scalar key/ABI/effect
correctness, floating precision policy and performance acceptance. These review
questions are distinct from unfinished implementation above; neither is waived.
Evidence: `/home/vishvananda/work/private/v4codex/artifacts/pa25-136/`.
This committed implementation handoff returns control to Ralph for further work
and audit; it does not certify assignment completion or authorize advancement.
