# PA15 final performance audit

Final implementation: `dff6a92b`; final-audit entry: `d97445d4` (same compiler as `5452ad74`); stage base: `8000f3c8`. All final compiler binaries use `g++ -std=gnu++11 -Wall -O3` with the test runner enabled. Target flags are `--emit-lowir -O0`; the pinned supplied backend uses `-O0`. CPU affinity is 31.

The complete frozen binaries, source/LowIR/native hashes, commands, phase/work telemetry, user/system time, scheduling counts and observations are retained in the three final campaigns:

- [stage-performance.json](/home/vishvananda/work/private/v4codex/artifacts/pa15-final-audit/stage-performance.json)
- [owner-performance.json](/home/vishvananda/work/private/v4codex/artifacts/pa15-final-audit/owner-performance.json)
- [owner-repeat-performance.json](/home/vishvananda/work/private/v4codex/artifacts/pa15-final-audit/owner-repeat-performance.json)

The [cumulative harness](../student.tests/pa15/audit_benchmark.py) and [owner harness](../student.tests/pa15/final_benchmark.py) run a warmup per binary, four A/A observations, then two ABBA blocks. Output/behavior parity is checked before timing. Telemetry and full LowIR validation run separately. The owner timer excludes usage parsing and hash checks; the inherited cumulative timer includes reading a tiny usage record. Compiler samples around 6 ms are startup dominated and support no compiler speed claim. Native loops use volatile runtime bounds and verify their results. The frozen original owner harness is retained alongside its first campaign; later filtering only selects the documented repeated inputs.

No observations were discarded. [The verifier](../student.tests/pa15/verify_final.py) checks **24 campaigns / 4774 invocations**, including **644** new final observations and warmups (154 cumulative, 420 owner, 70 repeat). Historical [checkpoint](audit-performance.md), [matching](matching-performance.md), [initialization](initialization-performance.md) and [execution](execution-performance.md) results remain evidence for their actual binaries. Exact historical harness hashes are resolved to retained git/file contents, avoiding stale live-path assumptions.

## Compiler latency and peak memory

Wall times below are medians in milliseconds; peak RSS is maximum observed process KiB. A/A is the four-observation wall range, not a confidence interval. Both paired B/A ratios are reported. Full sample spreads are in JSON.

Accumulated PA15, stage base versus final, on semantically equivalent common inputs:

| Workload | A/B wall ms | A/B peak RSS KiB | Paired B/A | A/A ms |
|---|---:|---:|---|---|
| types-1000 | 71.831 / 72.133 | 17304 / 17348 | 1.0116, 1.0033 | 70.6–72.0 |
| wide-signatures-1000 | 218.651 / 216.050 | 51264 / 48680 | 0.9925, 0.8989 | 216.3–226.4 |
| types-4000 | 279.054 / 285.347 | 53596 / 53988 | 1.0240, 1.0221 | 278.2–283.0 |
| wide-signatures-4000 | 906.229 / 893.248 | 188100 / 178560 | 0.9825, 1.0209 | 902.8–977.9 |
| constants | 229.977 / 229.553 | 34696 / 34880 | 1.0060, 0.9843 | 226.3–230.4 |

The 4000-type workload is about 2.3% above the stage base; both paired ratios agree (1.0240/1.0221). The accumulated stage adds required argument normalization, matching and checked semantics. Earlier scalar owner cleanup and the current source audit found no optional transform behind this cost. Wide signatures use less peak memory; their latency ratios disagree, so no precise speedup is claimed. Constants remain near baseline.

Final audit entry versus final repair; all these inputs are correct in both compilers and have identical LowIR:

| Workload | A/B wall ms | A/B peak RSS KiB | Paired B/A | A/A ms |
|---|---:|---:|---|---|
| calls-1000 | 35.177 / 35.684 | 9428 / 9384 | 1.0190, 1.0033 | 35.0–35.2 |
| calls-4000 | 125.434 / 125.909 | 20748 / 20752 | 1.0029, 0.9884 | 124.4–126.2 |
| receivers-4000 | 306.300 / 306.621 | 48628 / 48536 | 0.9987, 0.9964 | 303.2–307.0 |
| dormant-4000 | 171.836 / 171.378 | 31736 / 31664 | 0.9974, 0.9875 | 170.9–171.8 |
| storage-4000 | 292.824 / 292.471 | 46016 / 45824 | 0.9888, 1.0056 | 290.2–295.7 |
| nested-packs-1000 | 284.941 / 285.020 | 46068 / 46076 | 1.0117, 1.0072 | 284.6–286.0 |
| nested-packs-4000 | 1229.477 / 1296.569 | 166548 / 166496 | 1.0032, 1.0816 | 1214.5–1223.7 |
| pack-targets-4000 | 564.966 / 565.568 | 90284 / 90220 | 0.9682, 1.0019 | 562.0–567.4 |
| pack-selections-4000 | 231.128 / 230.382 | 35068 / 34908 | 1.1034, 0.9983 | 229.1–233.7 |
| defaults-4000 | 216.931 / 217.250 | 34272 / 34304 | 1.0053, 0.9912 | 214.5–217.9 |
| values-4000 | 221.751 / 223.002 | 38452 / 38412 | 1.0055, 0.9991 | 220.7–221.9 |
| class-patterns-4000 | 446.570 / 449.508 | 63600 / 63524 | 1.0135, 1.0216 | 445.6–465.2 |
| ordered-patterns-4000 | 202.449 / 202.949 | 32236 / 32108 | 1.0033, 0.9936 | 201.5–203.5 |
| alias-overloads-4000 | 488.964 / 487.536 | 79052 / 78996 | 1.0160, 0.9753 | 488.6–524.2 |
| arrays-4000 | 532.366 / 533.958 | 83992 / 83952 | 1.0039, 0.9989 | 531.5–535.0 |
| derived-4000 | 298.402 / 296.882 | 55180 / 55060 | 1.0001, 0.9897 | 296.4–298.6 |
| updates-4000 | 458.388 / 460.243 | 71056 / 71156 | 0.9907, 1.0005 | 453.0–463.6 |

The first large nested-pack campaign includes B observations at 1347.1/1375.7 ms with only about 1230/1220 ms user+system time, producing a 1.0816 second-block ratio. These observations prompted a frozen repeat, together with smaller pack/call controls and class matching; they were not removed:

| Repeated workload | A/B wall ms | A/B peak RSS KiB | Paired B/A | A/A ms |
|---|---:|---:|---|---|
| calls-1000 | 35.366 / 35.369 | 9196 / 9264 | 0.9897, 1.0205 | 35.1–35.9 |
| calls-4000 | 124.803 / 125.918 | 20668 / 20648 | 1.0197, 1.0081 | 123.5–170.0 |
| nested-packs-1000 | 281.531 / 280.179 | 45972 / 45804 | 0.9952, 0.9192 | 281.1–285.2 |
| nested-packs-4000 | 1218.845 / 1205.405 | 166312 / 166312 | 0.9982, 0.9920 | 1217.4–1234.7 |
| class-patterns-4000 | 443.109 / 442.195 | 62896 / 62892 | 1.0045, 1.0012 | 437.7–493.7 |

The repeat does not reproduce a large nested-pack or class-selection regression. Some new observations also stall (calls-4000 A/A reaches 170.0 ms; class-4000 reaches 493.7 ms), and all remain in the record. Calls-4000 adds about 1.1 ms median in the repeat; its 1.0197/1.0081 ratios and the first campaign’s 1.0029/0.9884 do not establish a precise speed claim. The volatile legality checks cost constant work at existing value/initializer visits and add no new state or optional transform. No unsupported percentage gate is inferred from these noisy results.

Compiler `.text` is **1,424,966 → 1,528,326 bytes** across PA15 (+103,360, 7.25%). The final volatile repair is **1,528,070 → 1,528,326 bytes** (+256, 0.017%). RSS stays near audit entry on the measured owner corpora. Historical required ordinary-body validation, initializer classification and storage costs remain disclosed in their original campaigns.

## Executable runtime, text and data

All common executables have identical bytes across the compared compilers. Their native runtime samples report 256 KiB peak RSS. Native text is the sectionless ELF payload from entry to the data tail, including alignment padding; array data tails are accounted for separately. The owner campaign independently checks the full executable hash against the previously measured final ELF before reusing its verified text/data split. This does not count readonly data as machine instructions.

| Comparison / workload | A/B runtime ms | Text bytes A/B | Data bytes A/B | Paired B/A |
|---|---:|---:|---:|---|
| Stage: runtime-calls | 301.304 / 300.939 | 206 / 206 | 0 / 0 | 0.9959, 1.0003 |
| Stage: runtime-memory | 176.405 / 176.114 | 434 / 434 | 0 / 0 | 1.0000, 0.9990 |
| Stage: runtime-floating | 208.244 / 209.090 | 230 / 230 | 0 / 0 | 1.0056, 0.9904 |
| Audit: runtime-constant | 416.944 / 415.301 | 242 / 242 | 0 / 0 | 0.9962, 0.9889 |
| Audit: runtime-class-pattern | 311.702 / 313.851 | 217 / 217 | 0 / 0 | 1.0052, 1.0313 |
| Audit: arrays-4000 | 1466.076 / 1464.831 | 376128 / 376128 | 64000 / 64000 | 1.0006, 0.9956 |
| Audit: runtime-array-4 | 149.708 / 149.828 | 224 / 224 | 16 / 16 | 1.0016, 1.0014 |
| Audit: runtime-array-8 | 142.707 / 142.403 | 232 / 232 | 32 / 32 | 0.9958, 1.0012 |
| Audit: runtime-array-16 | 204.794 / 204.891 | 310 / 310 | 0 / 0 | 1.0001, 0.9959 |
| Audit: runtime-array-64 | 570.882 / 572.272 | 750 / 750 | 0 / 0 | 1.0016, 0.9999 |

No runtime optimization is claimed for the volatile repair. Timing variations on identical bytes, including the selected-class second block at 1.0313, do not establish a code-quality change. The repaired volatile initializer paths require dynamic loads; the final native/LowIR reducers check those effects rather than comparing speed against an incorrect omitted read.

## Bounds, profitability and stage acceptance

Current counters establish the same owner scaling as the reviewed implementation: nested packs have nine retained source regions, 18 discovery visits, 7N expansion lanes and 3N body transitions at N=1000/4000. Scalar calls have one checked body, N activations, 5N expression visits and N memo hits. Four thousand ordered matches compute only two immutable pair facts (7998 hits). Storage performs zero unrelated body checks. Full telemetry for values, defaults, targets, selected packs, aliases, initializers and updates is preserved in the owner campaign.

At O0, constant execution retains limits of 512 active calls and 1,000,000 expression visits per root; exhaustion cannot publish a false reusable activation failure. Required work follows parsed/dependent nodes, candidates, demanded facts and emitted IR. There is no new fixed-point optimizer, search or unbounded code-growth transform.

The inherited backing policy permits at most **32 bytes per eligible local constexpr array**. It checks static addresses, copy legality and volatile subobjects and preserves distinct destination identity. It adds at most 32 bytes times the number of such emitted objects across the pipeline. Larger/noncopyable arrays use their ordinary typed initializer plans and zero loops. Preserved initialization campaigns rejected unbounded backing after an approximately 8% runtime regression on the 64-element case, despite fewer instructions. The 8-element case shows paired runtime benefit (0.9334/0.9268); the required 4-element contract form has a disclosed small cost (about 1.6% median). The final 16/64-element programs match the direct-initialization baseline bytes.

PA15 mandates O0 LowIR correctness and its comparisons, with no numerical latency/RSS/text ceiling. Spec §9 classifies inherited diagnostic targets, such as the PA14 local 64-KiB compiler-text review budget, as evidence rather than an accumulated PA15 gate. Historical misses do not permanently fail the corrected policy; mandated limits, coverage and all measurements remain preserved. No avoidable optional regression is retained under a correctness excuse.

Native allocator/spill/encoding quality, optimized LowIR levels and self-hosting belong to later assignments. The supplied backend establishes current executable behavior and size; it cannot certify a student backend that PA15 does not implement. No future-stage measurement is invented or made an extra PA15 exit gate.
