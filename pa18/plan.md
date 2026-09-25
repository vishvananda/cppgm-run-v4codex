# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `f6eaf8ab213c90eefffd48f5c47b1ac9a75bce8d`.
Target: **PA18 full-stage**, unfinished. Phase: **checkpoint audit 86 complete**.
Entry `890f810b` and reviewed code: **417/420**, the same **three failures**.
The preceding goal turn supplied committed progress; entry had no live job to resume.

## Accumulated review and ownership

[Audit 86](audit.md) covers every commit from previous review `ecc308bc` through
handoffs 83–85 and the validated code tip above. [Audit 82](audit82.md) is preserved
verbatim. Array-bound completion, constant images, constructor entries, empty
transfers, discarded-value demand and static storage were reviewed together.

The audit repaired three connected gaps: exception queries now consume retained
discarded-copy/default-argument effects; the shared constant evaluator checks
canonical literal-type facts before accepting temporary lifetimes; cast prediction
recognizes parenthesized braced constructions. Query recipes stay sparse and
TU-owned, completion invalidation stays local, and evaluated uses own their
materializations. No grammar replay, initializer syntax scan, eager body demand,
textual semantic key or global retry was introduced. Eight source-to-native
traces and inherited ownership/scaling inspections pass.

## Evidence and acceptance

The [loop86 manifest](../student.tests/pa18/loop86-evidence.json) binds the complete
18-commit range, source/binary hashes, all checks and historical evidence.
`make test-pa18`: **417/420**, exit 2, identical three entry failure paths.
Earlier PAs: **2609/2609**. File audit passes. All **420** stage inputs and **1686**
fixture paths remain; comparison rules are unchanged. All **23** accumulated
oracle corrections reproduce from [proof 83](reference-correction83.md),
[proof 84](reference-correction84.md) and [proof 85](reference-correction85.md).
The audit changes no reference. Personal controls: **1355 inherited + 74 audit =
1429**, all pass; the exact new corpus improves **23/74 → 74/74**. Another 34 PA16
initialization controls pass.

[Performance 86](performance86.md) preserves **869 observations / 36 workloads**
and verifies **1990** historical observations. Frozen cumulative A/A/ABBA runs
measure compiler latency/RSS and checked native runtime/payload. Empty-helper
omission has a repeatable benefit with O(1) proof and zero growth; required array
image/copy costs are disclosed. Acceptance is **PA18/O0 LowIR**, spec §9.
Historical **+15%, +16 MiB and 5.5×** targets remain diagnostics: PA18 mandates no
numeric compiler latency/RSS ceiling. Correctness, coverage, mandated limits,
bounded work and optional-transform profitability remain requirements. Existing
initialization, summary and constant-evaluation budgets remain. Later native,
debug and self-hosting obligations do not add PA18 exit gates.

## Remaining implementation

**Class-result ABI:** resolve friend-function-template alias results,
conversion-function-template object results and dependent defaulted non-type
declaring-scope results together across definitions, calls and indirect signatures.
These are the three course failures. Reduced bundle observations have not yet
established a uniform canonical rule for the differences.

**Class ellipsis:** finish the inherited representation across LowIR's scalar
variadic boundary. Passing other controls does not waive this obligation.

The separate array, constructor and discard handoffs left shared exception and
constant-lifetime consumers unchecked together. This fragmentation was avoidable.
Future groups should validate each fact through semantic/query/constant consumers,
lowering and emission before handing it off. The accumulated review is complete;
the remaining implementation above is not a waiver or advancement to PA19.

## Handoff ledger

Stage entry **266/420**. Older ledgers remain in [audit82](audit82.md) and its links.

| Checkpoint | Range / disposition |
|---|---|
| 82 | `82fca940` → entry `85ea42c0` → code `ecc308bc`; accumulated audit, **396/420**; earlier **2609/2609**, file/coverage and **1164** controls pass. |
| 83–85 | Entry `48c864ab` → `890f810b`; [arrays](handoff83.md), [constructors](handoff84.md), [discard/storage](handoff85.md); **411 → 414 → 417/420**, 23 proved oracle corrections; reviewed and repaired in 86. |
| 86 | `ecc308bc` → entry `890f810b` → code `f6eaf8ab`; accumulated audit and exception/constant-lifetime/brace fixes. **417/420**, identical three failures; earlier **2609/2609**, file/coverage and **1429** controls pass; cumulative stage-scoped performance accepted. The code tip is the next review baseline. |

Required commands: `make test-pa18`, `make test-report-through-pa17`,
`perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src`.
Root reports run sequentially because they share `.test_counts`.
**Do not advance to PA19 until `make test-report-through-pa18` passes and the
whole-stage independent audit is resolved.**
