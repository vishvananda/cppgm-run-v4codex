# PA15 implementation handoff

Stage base commit: `8000f3c8ef4647d57f2c0775192585f14cab33d8`
Last reviewed commit: `8000f3c8ef4647d57f2c0775192585f14cab33d8`

Target: **PA15 full-stage**, O0 typed LowIR. Implementation remains incomplete;
this handoff completes the canonical explicit-selection and constant-variable
query/storage group. Independent whole-stage review remains pending.

Turn entry `d3475a79`: **114/177**. Handoff: **128/177**, **49 failures**.
Fourteen original failures resolved, no previously passing fixture regressed,
coverage unchanged at 177. The preceding goal turn made verified implementation
progress; its scalar-value evidence is preserved.

## Design and spec alignment

| Owner | Data flow, identity and work |
|---|---|
| Explicit selection | Primary EntityId + canonical argument slice selects the existing declaration before body/layout demand. Early aliases and forward references retain identity. Completed primary definitions cannot be replaced; no global invalidation or retry. Expected O(arguments), plus the required function overload candidates. |
| Explicit source definitions | An explicit class gets its own lexical environment, independent of primary parameter names. A function specialization uses its own parsed signature/body and parameter names; primary defaults and ABI identity remain attached to the selected entity. Source parsing and primary body instantiation stay distinct. |
| Members and consumers | Undemanded primary inline member bodies yield to explicit definitions before checking. Static declarations/definitions, destructor exception boundaries, virtual members and transitive pointer conversions use ordinary typed facts. Linkage distinguishes explicit definitions from implicit instantiations and inline declarations. |
| Constant variable extension | Fixture-required integral constant variable templates retain initializer queries; canonical primary/argument identity owns one computed value and object. Simple reference-pattern partials use the primary's candidate list. Work follows parameters, relevant partial candidates and new query nodes. Address/reference observation records storage demand; lowering consumes constant/object facts and typed ABI arguments. |
| Lifetimes and counters | New indexes use the existing flat TU-owned storage; temporary substitution bindings are local. No grammar replay, name-string keys or textual phase transport. Selection, initializer transition/reuse and candidate counters observe existing work. |

The prior scalar argument, query, default and signature group remains documented
in [its handoff](../student.tests/pa15/handoff.json) and
[performance evidence](../student.tests/pa15/performance.md).

## Remaining implementation and boundary

| Group | Missing owner/facts |
|---|---|
| Packs, partitions and literals | 36 cluster-200 failures plus the many-partition case. Need pack boundaries in keys, retained list-expansion contexts, lockstep/nested expansion, sizeof... and literal-call construction. Scalar argument selection cannot produce these declaration/call/base/initializer lists. |
| Broader substitution and constant queries | Class partial matching and dependent alias overload cases; constant object conversions/calls, string-element evaluation and a nested static-member query case. Need selected-pattern substitutions and constant execution/binding facts beyond the completed scalar query owner. |
| Ordinary source obligations | Unused ordinary member assertions remain unchecked, including the inherited obligation for ordinary explicit-class member bodies. Need validation independent of body emission demand; eagerly lowering every inline body would violate the demand boundary. This is unfinished implementation, not an audit waiver. |
| Initializer and storage contracts | Aggregate functional/constexpr initialization, an undemanded static constant definition, and the stale-function fixture's initialization/comparison shapes still differ. The stale-function failure is now an initializer/LowIR mismatch, not function selection. |

The exact 49 failures are in [the current ledger](../student.tests/pa15/specialization-handoff.json).
Related selection work was extended through member bodies, arrays, destructor and
virtual consumers, defaults, canonical value queries, storage and redeclarations.
The next work needs list-expansion, execution or source-obligation owners; changing
selection tables further cannot establish those missing facts. Those groups remain
required for the full stage. No reference, fixture, bundle or comparison rule changed.

Independent review questions remain open: whole-stage correctness and spec
conformance, complete specialization keys, source sharing, demand/invalidation
boundaries and performance acceptance. Neither review marker is advanced.

## Validation, performance and ledger

Required checks: `make test-pa15` **128/177**, exit 2, with strict failure
reduction; `make test-report-through-pa14` **1935/1935 pass**;
`perl scripts/cppgm_file_audit.pl --stage pa15 --paths dev/src` **pass**, the same
three inherited header warnings. Personal controls explicitly run: **24 native
outcomes + 13 rejections**, plus **26 value** and **10 constant** groups.

[Current performance](../student.tests/pa15/specialization-performance.md) retains
frozen A/A and ABBA observations, compiler latency/RSS/text, native runtime/text,
output parity and work scaling. PA15/O0 introduces no optional optimization or
numerical exit gate; later native optimization/self-hosting limits keep their
owning stages. Raw checks and binaries: `$RALPH_ARTIFACT_DIR/pa15-specialization/`.

| Increment | Commit | PA15 |
|---|---|---:|
| Prior constants and value arguments | `cdbfeb8a`, `01b543ed` | 114/177 |
| Selection/member/variable-query implementation | `767e0797` | 128/177 |
| Selection and query telemetry/benchmark | `93dd5e29` | 128/177 |
| Specialized declaration/definition distinction | `113b6907` | 128/177 |

[Verification script](../student.tests/pa15/verify_specializations.py) checks frozen
hashes, original failures, fixture trees, counters and preserved review markers.
This implementation handoff does not certify the assignment or close its audit.
