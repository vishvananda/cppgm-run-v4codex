# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement — dependent fact/definition owners**.
Entry **84/314**, 230 failures; current **264/314**, 50 failures.
**180 original failures resolved; no entry passes lost; coverage unchanged.**

## Design/spec alignment and remaining groups

Canonical template/argument identities now feed demanded declarations, bodies,
class completion and ordinary typed LowIR/ABI. Parsed source is retained once
with compact contextual occurrences; no grammar replay or syntax-tree copying.
[Implementation ownership and remaining requirements](implementation.md) records
the data flow, complexity, representative failures and concrete checkpoint boundary.

| Remaining owner | Next coherent work / validation |
| --- | --- |
| Dependent fact graph | Symbolic qualified types/values, trailing returns and bounds; definition-owned parameter environments; substitute dependent edges and reuse fixed facts. Validate dependent signatures, current specialization and alias chains. |
| Template definition registry | Indexed out-of-class member/nested-class/static-data definitions and declaration-owned defaults; narrow body/layout/storage demand, including unevaluated uses and late owning destructors. |
| Definition-time lookup/checking | Bind nondependent names, preserve base-specifier provenance, check unused bodies/scopes and parameter shadowing. Validate the rejection and dependent-base families. |
| Parser and inherited object facts | Qualified declarator/explicit-instantiation context; preserve value categories, empty-object transfers, local ABI roots and lifetime/storage actions through specialization. |

Finer occurrence demand, dependent-only semantic checking, typed demand edges,
distinct fact states and narrow failure memoization remain open **PA14 spec
requirements**, not later-stage exemptions. The completed call-demand increment
was extended through overloaded arguments and class-reference conversions.
Typed qualified member types and indexed definition owners are now implemented.
Dependent expression facts and finer semantic reuse remain open; related class
demand, nested-owner and explicit-instantiation work continues in this turn.

## Performance evidence

[Review](performance.md): **1,148** preserved timed processes across frozen A/B/C/D,
A/A calibration and ABBA blocks; checked compiler wall/RSS, native runtime and
size. Common outputs/executables are byte-identical. New templates are measured
only on working implementations. Repeated demand computes one body; 4× class
inputs produce 4× completion/occurrence work and 3.64× current median wall time.
Measured source-read overhead was corrected; text growth and remaining costs/
wall stalls are disclosed. Historical diagnostic misses add no unsupported
stage gate. O0, correctness, coverage, mandated limits and ownership remain intact.

## Handoff ledger

Continuation entry `4fafa38c`: clean tree, revalidated **222/314**. Previous
turn classified **progress** from committed implementation and verified fixture
improvement. First group: typed dependent qualifier identities -> canonical
substitution -> concrete member types; then declaration-owned indexed out-of-class
definitions -> narrow member/storage demand. Work tracks qualifier edges and
definitions for the requested owner. Validate dependent return/alias signatures,
renamed member heads, nested definitions, and evaluated/unevaluated static uses.

| Increment | Commit / result |
| --- | --- |
| Entry markers | `9bb7694a`: clean base above; prior PA13 audit was verified progress |
| Function demand, canonical declarations and ABI | `2cec3424`: 140/314 |
| Class registry, defaults, lazy completion and ADL | `73409fcc`: 202/314 |
| Call deduction/defaults/operators and base provenance | `45b15b80`, `c48def7d`: 219/314 |
| Static addresses and ordinary source-read fast path | `7e88952a`: 220/314 |
| Overloaded argument/name contexts and conversion completion | `e0eb788f`: 222/314 |
| Evidence consolidation | This plan, ownership/performance reviews, raw JSON and verification scripts |

Final checks: `make test-pa14` **222/314**; `make test-report-through-pa13`
**1621/1621**; through PA14 **1843/1935**, only PA14 fails. File audit **passes**
with three inherited header advisories. Six personal executables and **320**
release/ASan/UBSan status/output comparisons pass; rejection parity does not mean
320 course-correct programs. All **1,148** performance observations verify.
Fixtures/references and review markers are unchanged. Full logs and frozen
artifacts: `$RALPH_ARTIFACT_DIR/pa14-measurements/`. PA15 has not been started.

Current increment: **264/314**, through **1885/1935**, prior **1621/1621**,
file audit passes, eight personal executables pass. **42 continuation-entry
failures resolved with no regressions.** Qualified dependent types use canonical
owner/name/argument identities; out-of-class definition heads, bodies and static
initializers retain their parameter overlays. Indexed applications occur once
per concrete owner/definition. Definition-time shadowing and independent nullary
exception checks preserve the required rejection cases. Parser overlays retain
renamed heads. Current performance/sanitizer evidence remains to be collected.
