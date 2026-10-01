# PA30 compact implementation plan — handoff197

Stage base commit: `27029f978e65b78331233123922d342033d5d1f7`.
Last reviewed commit: `27029f978e65b78331233123922d342033d5d1f7`.
Target: **PA30 full-stage**. Phase: **implementation handoff, incomplete stage**.
Entry197: `407fcdc0fceea44331958d1f0dd312432773fd35`, clean. Previous goal turn:
progress (validated handoff196); no live work needed resumption. Review markers
retain the PA29 boundary; implementation checkpoints are not independent audits.

## Completed group and spec alignment

Earlier parser/current-instantiation groups and their evidence remain in
[performance195](performance195.md), [performance196](performance196.md) and
`student.tests/pa30/evidence195/`, `evidence196/`.
Implementation197 completes **alias identity in lookup and pattern matching**:

| Owner / commit | Data flow | Complexity / lifetime |
|---|---|---|
| `semantic/class_pattern_selection` / `6bf7812b` | Candidate arguments → deduction/substitution → memoized semantic signature shape → unique partial specialization. Retained alias obligations no longer create false type inequality. | Exact-ID fast paths; shape work once per distinct immutable argument graph, O(1) average reuse; existing TU Analyzer cache. Substitution/access checks remain candidate-local. |
| `semantic/lookup` / `94faedf1` | Indexed using edges → declaration pair → canonical type/namespace identity → merged result. Same-type namespace typedefs and namespace aliases converge. | O(1) pair comparison, no new storage, scans, retries or invalidations; existing visited edge traversal and TU entities. Class-base contract unchanged. |

Extended beyond the initial namespace/ordering reducers through qualified,
transitive/cyclic lookup, namespace aliases, nested alias patterns, packs, cv,
non-type values, ambiguous patterns, private access and failed substitution.
Shared typed semantic graph → typed LowIR → native ELF path remains in use;
no new source registration, backend, optimization or production text adapter.

## Validation and performance

[Required checks](../student.tests/pa30/evidence197/validation.json): prior PAs
**4941/4941 pass**, file audit passes (four inherited header warnings), PA30
**129/153 → 132/153**, **24 → 21 failures**, zero regressions. The through-PA30
report is **5073/5094**; the stage remains incomplete and cannot advance.
Ralph's entry summary count is 129/154; the raw entry log and every isolated
course run contain 153 fixtures. The [delta](../student.tests/pa30/evidence197/stage-delta.json)
binds all existing cases and unchanged PA30 inputs/oracles/harnesses. Initial
concurrent reports shared counters; only later isolated reports are acceptance
evidence. No reduced coverage or newly added passing fixture supplies progress.

[Controls](../student.tests/pa30/evidence197/controls.json): **23 commands pass**,
including direct object execution, explicit LowIR/native adapter execution,
failed substitution/access and required ambiguity rejections. Personal controls
were run explicitly. One PA6 reference correction has a reduced reproducer,
C++11/CWG14 proof and bundle binding in [reference-correction197](reference-correction197.md).
No PA30 reference changed; comparison rules and fixture discovery are intact.

[Performance197](performance197.md) retains frozen A/B flags, inputs and binaries,
404 observations + 16 launchers, A/A + six ABBA blocks, corrected-owner scaling,
compiler latency/RSS and checked
runtime/text measurements. No optional transform or speedup claim. Mandatory
45-second compile limit is preserved (largest repaired fixture: 2.715 s /
88,636 KiB). Lookup work = `102N+49`; ordering shape work = `2N+1`. Inherited blanket percentage/no-growth
targets remain diagnostic under spec §9, with historical measurements preserved.

## Remaining implementation groups

| Failures | Owner / required data flow |
|---:|---|
| 6 | `semantic/callable`, template demand: callable/shared-pointer uses → available prerequisite facts and demanded bodies. Shared-pointer now passes partial ordering and reaches this owner. |
| 6 | `semantic/type_builder`, constant evaluation: hosted array expressions → checked integral bounds. |
| 2 | `semantic/access`: random-library nested member use → retained access context. |
| 2 | `semantic/construction`: piecewise tuple arguments → viable constructor and conversions. |
| 1 | `semantic/template_call`: bind member callable → viable overload. |
| 1 | `semantic/exception_specification`: replaceable new redeclaration → compatible exception fact. |
| 1 | Builtin registry / vector lowering: target vector intrinsic → typed operation. |
| 2 | Body checks / lowering reachability: cross-function local reference and reachable missing return → required rejection. |

## Handoff ledger and review boundary

- Fixed existing chrono, same-type namespace and inline-callee closure fixtures.
  Related shared-pointer ordering also resolves, exposing a prerequisite failure.
  No remaining failing fixture diagnoses ambiguous namespace lookup or partial
  specialization. Type identity is established before the remaining decisions.
- Coherent boundary: further fixes require separate prerequisite scheduling,
  constant evaluation, access, conversion, intrinsic or lowering facts. The
  completed identity owners cannot supply those facts; further related identity
  work lacks a failing reducer. The remaining groups above are implementation,
  not questions deferred to review, and remain required.
- Independent review pending: earlier split-cache/class-friend and injected-type
  scope changes; signature-shape comparison versus substitution/access obligations;
  namespace merge identity and canonical representatives; whole-stage demanded
  template-to-ELF architecture and performance. Also reconcile the retained base
  alias rejection contract with [class.member.lookup]/3,6–7 wording, separately
  from the namespace proof. No base-reference correction is asserted or waived.
- Evidence binding: **725 consistency checks pass** via `student.tests/pa30/verify197.py`.
- This ends implementation197 only. Ralph audits the accumulated checkpoints
  and schedules further implementation before advancement. Evidence/docs do not
  change the tested implementation; finish with committed, clean status.
