# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement**; architecture remains open.
Original entry **84/314**; continuation entry/current **314/314**. All **230
original failures** are resolved. Coverage, references and comparisons are
unchanged. PA15 has not started. Current implementation/evidence: `0fc50a95`.

## Design/spec alignment

Continuation from `167f5f43` closes ordinary member definition matching, selection
and application together. Source prototype IDs and canonical signatures select
definitions; concrete members retain that identity. Application state belongs to
specialization/definition; traversal success/absence belongs to member/source-head.
Source checking publishes selection only after completion. Renamed nested aliases
retain their declaring head. Matched bodies register against the selected entity
and substitute raw source parameter facts, preserving cv/array/function forms.
Defaults, exceptions and virtual checks retain their existing owners.

Active continuation from `60cf761c`; previous turn: **verified progress**. Extend
the retained source-signature owner to special members, replacing nullary syntax
matching with complete typed signatures. Injected-class/current nested types now retain declaring-head parameter slices;
matched special definitions use direct member application. Preserve transfer, exception and base/complete-entry
owners. Validate constructor/copy/move/conversion overloads, unused rejection,
renamed heads, deferred destruction and source/key/demand scaling; freeze full
correctness/layout/performance evidence before acceptance.

| Owner / data flow | Complexity and validation |
| --- | --- |
| Ordinary definition matching/application (completed group) | Source prototypes → canonical signature index → per-prototype definition list → concrete member/body facts. For N specializations, K overloads and Q requests: source work K, applications/edges N, requests N(Q+1), hits NQ; required candidate work NKQ is unchanged. Validate late/re-entrant publication, overload isolation, renamed/nested heads, parameters and twelve unused-definition rejections. |
| Expression, region/default and typed-value owners (inherited) | Immutable source topology → parent-linked frames → canonical queries → concrete uses/lifetimes and ABI facts. Earlier source/instance equations, negative keys, conversion sharing and measurements remain verified. |
| Special-member source signatures (remaining) | Establish typed injected-class/current-instantiation signatures for constructors, destructors and conversions. These still use unresolved source types and the broad declaration path. Couple signature identities to transfer/default/exception and base/complete-entry consumers. |
| Declaration/scope/object/lifetime graph (remaining) | Demanded regions still establish occurrence IDs and dense Fact slots. Distinguish required local object/scope identities from fixed facts and eliminate remaining fixed rechecks jointly with cleanup and lowering consumers. |
| Demand/failure dependencies (remaining) | Finish typed reasons, reverse edges and separate declaration/definition/layout/default/exception/body/vtable/emission states. Memoize structured expected failures per complete key; enqueue only affected consumers. |

**Concrete boundary:** the completed ordinary path has a checked source signature
and known concrete EntityId. Special members do not consistently have that source
type: injected-class lookup under a renamed head, conversion targets and transfer
classification still cross declaration, class completion and lifetime ownership.
Routing those cases through the new path would assume missing facts. Likewise,
removing region occurrences independently would alias local objects or lose cleanup
and parameter facts. Further work requires those joint producer/consumer changes,
not another local signature/traversal shortcut. There is no external blocker.

## Performance evidence and budgets

**11,158 observations** are retained: 10,514 inherited plus 644 new observations
on 37 compiler inputs/nine executables. The new N/K/Q scaling cases improve
36.63%, 56.38%, 37.64% and 12.21%, each in both ABBA blocks; peak RSS falls by
25,574 / 414,794 / 99,770 / 34,922 KiB. All 37 LowIR and nine executable hashes
match; generated growth is **zero**, with no runtime optimization claim. Full
medians, calibration/spreads, every regression and outlier are in
[performance.md](performance.md) and the raw JSON.

Compiler text grows 7,552 bytes (0.580%). Analyzer grows 5,680→5,920 bytes per TU;
source prototype/definition records grow 16→24 / 24→32; MemberFacts grows 120→124.
Entity/Expression/ObjectUse stay 112/36/36, expression properties/uses 24/20.
The inherited call-input-128 RSS rises 15,206 KiB. Separate Massif profiles show
only 24 bytes more peak live heap and identical dominant allocations/counters,
supporting an allocator/page-retention explanation without identifying its exact
native cause. The RSS increase and both profiles remain; no tuning hides them.

Explicit budgets: source/key/concrete-use-proportional storage, at most four
conversion variants per source operation, one local view per visit, and zero
generated growth. O0 has no mandated numeric latency/RSS/text ceiling. Repeatable
affected-workload savings justify the source/member owner costs; small inherited
latency/RSS increases remain disclosed. The latest transitive probe checks live
headers; preceding live-header equality is reclassified as frozen snapshot
integrity. Zero occurrences is not a numerical gate. No mandated limit, coverage
or measurement was removed. Historical source-cache +6,714 KiB and calls-4 roughly
+15 MiB RSS deltas remain unexplained by this campaign.

## Handoff ledger

| Coherent increment | Commit / evidence |
| --- | --- |
| Earlier declaration/frame/default/value owners | `78bdbc3f` through `5a795af4`; preceding proofs and measurements retained |
| Immutable expression uses, contextual inputs and local views | `4aedd84c` through `5ae726e0`; prior 10,514 observations and lifetime controls |
| Completed definition traversal/application states | `c65dbfe6`, `00495eb1`; initial combined owner later replaced by distinct correct keys |
| Typed source signatures and precise prototype demand | `39f385af`, `367d64a8`; twelve rejection proofs and unused-overload isolation |
| Declaring-head aliases and direct parameter/entity application | `5743a3cd`, `0fc50a95`; four exact LowIR/native controls |
| Frozen proofs, corpus, layouts and full acceptance | `6dc728f8` onward; 644 observations, sanitizer/native evidence and heap diagnostics |

Validation: stage **314/314**, prior **1621/1621**, through **1935/1935**, **28**
native programs, **342** release/ASan/UBSan parity inputs, **136** rejection
controls, **six** ABI controls, **seven** native reducers, expression-store checks
and entry/release/sanitizer lifetime controls. File audit passes with three
inherited header advisories. Evidence/verifiers live in `student.tests/pa14/`;
frozen binaries, outputs, profiles, command/status logs and initial failed probes
remain under `$RALPH_ARTIFACT_DIR/pa14-definition-demands/`.
