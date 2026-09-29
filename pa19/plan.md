# PA19 implementation plan

Stage base commit: `e5f4c3ed78972c8d161671d145bf525cb99033f4`.
Last reviewed commit: `e5f4c3ed78972c8d161671d145bf525cb99033f4`.
Target: PA19 full-stage. Phase: implement; validated full-stage implementation handoff 92.
Loop entry: `065d6783`, **407/423**; current: **423/423**, zero failures.
All 423 input sources and exit-status sidecars remain byte-identical to entry;
comparison and coverage rules are unchanged. Independent stage audit is pending.

## Design/spec alignment

Preserve the prior type/alias/deduction/ordering work and streaming parser,
canonical identity, immutable substitution frames and direct typed LowIR.

| Owner | Data flow and completed behavior | Complexity / validation |
|---|---|---|
| Variable declaration | Source initializer QueryId is retained through member declaration; source head + enclosing frame compose only on value demand. Declaration types substitute through complete specialization keys; expected signature failure stays compact | One source query, dependent edges per demanded frame; no grammar replay, initializer projection, global scan or retry. Defaults, packs, dormant outer/inner initializers, SFINAE controls |
| Variable value | Selected partial/primary + composed frame → typed query → checked initialization conversion → persistent literal value. Declaration and initializer facts have separate monotonic states | One initializer per specialization; existing conversion and constant caches. Class values, braced/implicit conversions, invalid/deleted/explicit construction, recursion, leaf-SFINAE |
| Variable storage | Evaluated use → distinct emission demand → initializer and relocation facts → existing constant-object LowIR | Canonical EntityId identity, once-only emission demand; native scalar/class value and repeated-address controls |
| Parser lookahead | Skip a template variable's initializer during class declaration classification; a constructor expression cannot hide the class name | Bounded delimiter lookahead; no second grammar parse; class-valued literal controls |
| O0 conversion provenance | Retained sizeof type parameter remains a substituted layout fact after instantiation; the neighboring widening keeps its explicit O0 conversion | O(1) identity lookup; no new optimization, graph, cache or search. NTTP call fixture, negative-value reducer, all earlier stages |

Class-valued and scalar initializers use ordinary semantic conversions and typed
constant storage; lowering does not reconstruct template semantics. No new source
unit, optional transform, process-global cache, host compilation or reference
output is used in implementation. Supplied native backend execution is validation
only, as required before PA24. Native optimization/debug/self-hosting acceptance
remains in those later stages.

## Reference contract corrections

[Proof and reducers](reference-correction92.md) and the
[independent reconstruction](../student.tests/pa19/reference92.py) cover fourteen
oracles: five PA16 automatic-array copies; five static-definition demand cases;
discarded-reference consumption; constant reference initialization; constant class
initialization; and explicit-instantiation metadata after specialization. The pinned
bundle is unchanged. Original inputs/statuses and all comparisons remain. The
sizeof/NTTP conversion case was repaired in implementation, without an oracle edit.
Prior [defaulted-pack correction 91](reference-correction91.md) is preserved.

## Validation and performance

- `make test-pa19`: **423/423**, exit 0. All sixteen loop-entry failures resolved.
- `make test-report-through-pa19`: **3452/3452**, exit 0, plus 22 focused properties.
- Required prior report: **3029/3029**, exit 0; file audit passes with three
  inherited header advisories and no errors.
- Personal controls: variable facts **29/29** (22 native, seven rejected), versus
  **13/29** on frozen entry; inherited composition **42/42**; reference reducers
  **13/13** native plus structural checks. Independent oracle reconstruction passes.
- [Frozen performance evidence](performance92.md): 16 completed workloads, 464
  observations plus 52 warmups; A/A, ABBA, latency/RSS, checked runtime/payload
  size, all spreads and the malformed-generator attempt retained. Nine shared
  outputs are byte-identical. Seven inputs measure new final-only behavior.
  Compiler text +1,600 bytes (0.080%). Largest namespace-variable paired latency
  +5.5%, with one required extra initialization-conversion check per key; largest
  new workloads scale with demanded facts. No speedup or optional transform.
  PA19/O0 stage-scoped acceptance passes. Inherited +15%, +16 MiB, 5.5× targets
  remain diagnostics; no mandated limit, correctness or coverage is waived.

## Handoff ledger and boundary

- Handoff 91: `1747113b`, `dbe5e97c`, `57e27df2`, `4dd6e737`, `065d6783`:
  type/declaration identity, alias/default composition, deduction/ordering and
  proved defaulted-pack oracle; 397 → 407/423. Evidence preserved in
  [handoff91.json](../student.tests/pa19/handoff91.json) and [performance91.md](performance91.md).
- `42251d95`: loop 92 ownership and evidence plan, preserving both review markers.
- `ea5c1d82`: lazy variable declaration/value/storage composition and 29 controls;
  both required compilation failures compile; earlier template stages pass.
- `5763cf6c`: O0 dependent layout provenance and thirteen defined reducers.
- `946c651b`: fourteen independently proved contract-oracle revisions; all stage
  fixtures pass. No input, exit status, comparison rule or required behavior removed.
- Final evidence commit: performance, check/coverage hashes and this compact plan.

**Unfinished implementation:** no known PA19 behavior group remains. Required checks
and frozen performance evidence are complete. This is a full-stage
implementation handoff, not advancement to PA20 or certification of the assignment.
**Independent review:** whole-stage correctness, architecture and performance,
including the reference proofs, remain audit obligations. Neither review marker
moves during implementation; a green suite does not waive whole-stage review.
