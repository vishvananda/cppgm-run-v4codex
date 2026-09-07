# PA4 completed plan and handoff ledger

Independent final audit in progress (entry `60ef1b9df`). Source review found
per-prescan task and child-buffer allocation/destruction in the explicit task
stack. Replace it with stable slabs and reusable depth-local buffers; preserve
the existing indexed slices and expansion semantics. Freeze entry binary A
before edits. Compare all eight fixed workloads with the existing A/A, B/B and
two-block ABBA protocol. Acceptance budgets remain <=10% plus calibrated noise
for paired latency, <=15% plus 1 MiB RSS, <=15% host text growth, and <6x time /
<5x RSS for 4x input. Require nested-work benefit beyond noise and counters
proving task/buffer growth follows maximum simultaneous depth, not invocation
count. Runtime/generated text remain N/A. Pool capacity releases with its
expander; no translation-unit/global cache is introduced.

Stage base commit: `a682ffe75533c8aed941f46f6131c9e8af22f93d`
Last reviewed commit: `a682ffe75533c8aed941f46f6131c9e8af22f93d`

Target: **PA4 full-stage**, phase: **complete**. Entry 0/105 → final 105/105
(71 macro, 34 directive fixtures); prior PA1–PA3 100/100; through-PA4 205/205.
No fixture, reference, harness, timeout or coverage was changed. Prior goal turn
classified as progress from committed, verified PA3 work. Review markers remain
at entry for the independent Ralph review.

## Design/spec alignment

| Owner | Data flow, complexity, lifetime and validation |
| --- | --- |
| Source/directives | Immutable TU buffers → streaming PA1 cursor; one pending directive, per-file conditionals → shared PA3 evaluator. Indexed names and flat device/inode once table; includes/locations/primary reset pass all 34 directive fixtures. |
| Macros | Dense definitions with prebound parameters → raw indexed argument slices → explicit prescan tasks → iterative rescan. Expand each ordinarily used argument once. Token-local ancestry/permanent paint; fixed-depth radix membership, shared intersections; all 71 macro fixtures pass. |
| Storage/API | No owning token strings or full output vectors. Deferred arguments release after substitution; context/generated-spelling scratch rewinds when expansion drains; TU sources/definitions release at TU end. Shared post-token cursor exposes physical and presumed locations, including split suffixes, to later consumers. |
| Observation | `preproc -o` writes PA2 records from structured tokens; invalid phase-7 tokens reject. Phase-3 lexing of generated replacement spellings implements paste/stringize/predefined tokens, never phase-to-phase text transport. Own implementation throughout. |

[Architecture audit](audit.md) traces ownership and semantics. Semantic template
instantiation, LowIR/MIR, ELF and optimization levels remain later-stage surfaces;
PA4 preserves their direct structured input and introduces no substitute graphs.

## Performance evidence and budgets

[Final evidence](../student.tests/pa4/performance.md): frozen functional A
`28279a9d0` vs final B `1af70fc0d`, identical ordinary flags, eight fixed inputs,
A/A and B/B calibration, two ABBA blocks, equivalent outputs, 120 observations
plus eight startup probes. Two preceding campaigns retain another 210 samples.
The verifier recomputes every budget and checks the actual final binary hash.

Nested arguments: 5.860740→0.109026 s median, 137092→4564 KiB RSS; both paired
blocks improve 98.14%. Long chains improve 4.78–5.80%. Disclosed regressions:
ordinary groups generally +3.54–9.76%; one flat block is +12.08% with 5.64% noise.
Host-tool text grows 6.78%; 4x source growth takes 3.9643x time and 3.0431x RSS.
Generated executable runtime/text: **N/A**. No generated-code benefit claimed.

Budgets fixed before campaigns: latency <=10% plus measured noise; RSS <=15%
plus 1 MiB; host text growth <=15%; 4x input <6x time/<5x RSS. All pass. API checks
prove 3n captured tokens for 20,000 nested calls, no host-stack recursion, and
<=128 KiB generated spelling for 100,000 counters. Fastest workload is 25.7x
measured startup. Benefits justify the recorded compiler work and growth.

## Validation and ledger

- `babb8e9d2`: recorded baseline/design before stage edits.
- `28279a9d0`: complete course behavior; 105 failures → zero, prior tests pass.
- `c90cf1e62`: eliminated quadratic nested-argument copies; explicit task stack,
  indexed slices, bounded spelling storage, personal/API checks and telemetry.
- `957b47c37`: eliminated unnecessary indexing for parameterless helper macros;
  kept both performance campaigns, including the initial regression.
- `1af70fc0d`: fixed presumed suffix locations; the new API regression failed
  before the fix and passes afterward. Final binary is the measured candidate.
- Final consolidation: final ordinary PA4 105/105, required prior report 100/100,
  through-PA4 205/205, file audit 43 files, and diff/fixture checks all pass.
  Final ASan/UBSan: 105 course cases, 163 personal invocations and API checks.
  Earlier personal suites also pass: PA1 64 cases/API, PA2 316 cases/API and
  7,062 independent integers, PA3 72 invocations/15,045 results/API.

Handoff reason: full stage complete; no remaining implementation or related
behavior group is deferred. Final plan/evidence are committed with a clean tree;
the preserved review marker remains available for Ralph's independent audit.
