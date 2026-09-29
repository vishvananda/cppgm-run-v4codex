# PA19 compact plan — final audit 93

Stage base: `e5f4c3ed78972c8d161671d145bf525cb99033f4`.
Last reviewed commit: `228d7fc7` (complete implementation and both handoffs).
Target: **PA19 full-stage**. Phase: **audit complete**.
**Spec Alignment: aligned for PA19/O0.** [Independent audit](audit.md).

## Final design

| Owner | Data flow / invariant | Review evidence |
|---|---|---|
| Source/parser | Immutable buffers → streaming PP/post/syntax cursor → one source graph with attached semantic facts. Templates keep parsed bodies; compact occurrences project demanded regions | Actual parser/semantic entry, source regions and lifetime boundaries reviewed; trace has 612 parsed nodes / 497 occurrences |
| Types, aliases, arguments | Canonical declaration/type/argument IDs retain alias-template identity, complete defaulted packs, function/ellipsis forms and dependent owners | Handoff 91 source review; 42 composition controls; native alias/pack/ADL/reference controls |
| Deduction and ordering | Relevant indexed candidates → immediate-context substitution → recorded selected declaration/conversions; defaults only after selection | Fixed-prefix/empty-tail ordering and declaration-order/ambiguity cases; independent selected/unselected default controls |
| Variable declaration/value/storage | Source initializer QueryId + immutable outer/inner frames → distinct declaration and initializer states → checked persistent literal constant → separate storage demand | Handoff 92 source review; 29 variable controls; two initializer transitions and one object per demanded key in the combined trace |
| Demand and invalidation | Canonical fact keys, active/success/failure states, precise owner queues and reverse query dependencies; nondependent facts shared | Source inspection of frame composition, body/default/constant owners and completion scheduling; stress counters scale with actual keys/edges |
| LowIR and ABI | Recorded layout/calls/conversions/cleanup → direct typed LowIR and PA9 ABI graph; serialize once as required output | `Pair<short>` is 16x8 with fields at 0/8; emitted calls, storage, signed widening and pack ABI inspected; native trace returns zero |
| O0 work and lifetime | Bounded named-constant forwarding, required constant-array images, at most four source conversion variants; conservative fallback and no new optimization pass | Receiver effects, volatile/effectful fallbacks, array identity and negative NTTP controls; TU pools/function scratch release reviewed |

Production remains self-contained; supplied native backend calls occur only in
validation. No new source unit or source-set registration is needed. Native
MIR/allocation/object encoding, optimized modes, native debug and self-hosting
remain with PA24/PA32–34, outside PA19's explicit output/acceptance boundary.

## Findings and changes

- Independently reviewed every stage implementation commit and its full owning
  paths. No additional compiler defect or unfinished PA19 behavior group found.
- Added 16 cross-handoff controls, a combined source-to-LowIR/native trace,
  reproducible verification and a frozen full-stage performance comparison.
  The compiler remains byte-identical to the validated handoff 92 binary.
- Reconstructed all 15 reference revisions from original oracles and cited
  standard/contract rules: [91](reference-correction91.md),
  [92](reference-correction92.md). No further reference edit. All 423 source
  inputs/status sidecars, coverage and comparisons remain unchanged.
- The audit verifier's initial mixed-manifest schema assumption was corrected;
  final verification passes. No compiler change was needed for that tool issue.

## Performance acceptance

[Final report](final-audit-performance.md): nine fixed workloads, 324 observations
and 34 warmups; frozen stage-entry/final hashes, separate compiler/executable
measurements, A/A and four ABBA blocks, checked results and all spreads.
Seven shared workloads have identical LowIR/native bytes; two are final-only
new behavior. Compiler text +5,952 bytes (+0.298%). Largest namespace-variable
paired latency +2.8%, peak RSS +2,328 KiB, with one required conversion check per
initializer. Runtime loop paired medians 1.003 / 1.000 / 1.004; no speedup claim.

[91](performance91.md) and [92](performance92.md) retain their broader corpus,
raw measurements and incomplete attempts. No historical observation is removed.
Spec §9 stage-scoped acceptance passes: inherited +15%, +16 MiB and 5.5× values
are diagnostic targets, not mandated PA19 gates. Required semantic costs and
later native ownership are documented; no correctness, coverage, work bound or
mandated limit is waived. No avoidable regression or new unprofitable optional
transform was found.

## Validation and ledger

[Current validation](../student.tests/pa19/audit93-validation.json),
[controls/trace/coverage](../student.tests/pa19/audit93-evidence.json):

- `make test-pa19`: **423/423**, exit 0.
- `make test-report-through-pa19`: **3452/3452**, **19/19 stages**, exit 0,
  plus the harnesses' **22 focused properties**.
- Required file audit: **pass**, exit 0; three inherited header advisories,
  no errors. Source ownership is reviewed in the audit.
- Personal controls: **100/100** (79 native, 21 required rejections), plus
  the native combined trace and defaulted-pack reducer; all 15 oracle
  reconstructions and unchanged source/status/comparison checks pass.
- Instrumentation parity and optional syscall self-containment inspection pass.

Handoff 91 (`1747113b` → `065d6783`) covers type/declaration identity,
alias/default composition, deduction/ordering and the proved pack oracle.
Handoff 92 (`42251d95` → `228d7fc7`) covers lazy variable facts/storage,
O0 dependent layout provenance, fourteen proved oracle revisions and frozen
validation/performance evidence. Both are fully reviewed in [audit 93](audit.md).
This audit commit consolidates their record and adds independent evidence.

**Open findings / unaudited handoffs: none through `228d7fc7`.**
Commit the cohesive audit artifacts and verify `git status --short` is empty.
PA20 work has not started.
