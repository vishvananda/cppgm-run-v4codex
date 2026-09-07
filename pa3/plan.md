# PA3 consolidated plan and final audit ledger

Stage base: `dddda5eec41799b1e45b39e896c3a6f3da1c5a3b`.
Independent audit entry: `c0bb94df8` (clean); last implementation: `f0b92212e`.
Target: **PA3 full-stage**, phase: **final audit**. All work groups complete.
The stage began at 0/20; final PA3 is 20/20 and through-PA3 is 100/100.

## Final Spec Alignment

Immutable source → PA1 streaming cursor and TU-interned identifiers → shared
PA2 scalar conversion → incremental typed expression evaluation → decimal view.
Each token is consumed once. Compact value/operator scratch replaces token
vectors and syntax trees; no serialized representation transports phase data.
Signedness and deferred arithmetic errors survive until branch selection.

| Owner / completed group | Final design, work and release boundary |
| --- | --- |
| Source and identity | Immutable TU source, bounded character lookahead, borrowed token spellings, flat stable identifier IDs; source/name arenas release at TU end. |
| Numeric/literal conversion | Shared PA2 decoders; PA3 requests only integral facts. Reject floating/UD numbers without floating extraction or suffix interning. No per-token ownership. |
| Grammar and evaluation | Iterative precedence/parenthesis/conditional stacks, O(tokens) reductions and O(pending depth) geometric scratch. Signed/unsigned 64-bit bits and error facts; no host UB in discarded arms. |
| Selection and recovery | Check all syntax/types; only selected arithmetic errors propagate. `finish` clears line state; scratch releases with the evaluator. `defined` uses a live typed callback with no cache to invalidate. |
| CLI and evidence | Logical lines, phase errors, checked output/EOF, optional existing-work counters; direct own implementation. Fixed benchmarks and independent oracle/API/sanitizer checks supplement course fixtures. |

[The independent audit](audit.md) records end-to-end traces and applicability of
all spec sections. Declarations/templates, LowIR/MIR, ELF, optimization levels
and generated executable benchmarks have no PA3 surface; they remain later PA
requirements, with no claim of implementation or generated-code improvement.

## Findings and changes

`f0b92212e` fixes the ownership defect found at the PA3 → PA2 decoder boundary:
PA3 used to construct floating values and intern rejected numeric suffix names.
The shared decoder now has an explicit numeric domain; PA2 keeps its complete
behavior. The new allocation/name-retention regression failed on the original
implementation and passes after the fix. Added decimal/exponent/UD rejection
cases also cover discarded branches and recovery. No course fixture, reference,
harness, timeout or coverage was changed; no grammar/arithmetic defect remained.

## Performance and validation

[Final evidence](../student.tests/pa3/final-audit-performance.md) retains **324**
observations from three serial frozen A/B campaigns, each with nine fixed inputs,
two A/A pairs and two ABBA blocks. A is the audited completion binary; B is
`f0b92212e`, both at identical ordinary flags. All outputs and hashes agree.
Floating-rejection paired latency improves 37.3–40.4%; suffix rejection improves
18.4–21.2%. The first suffix campaign is inconclusive against its 31.11% A/A
excursion; the other two confirm gains beyond noise. Every sample is retained.
Suffix name storage falls from 12,935,068 to 64 bytes; peak RSS from about
20,852–20,856 to 8,024–8,032 KiB. Host-tool text grows 312 bytes (0.36%), within 1%.

Unchanged-workload timing variation is disclosed, including one +7.64% nesting
block; its campaign paired mean is +3.39%. All campaign paired means stay within
the 5% plus noise regression budget. Final ordinary 4/16 MiB medians are
0.440050/1.743335 s, peak RSS 8,028/20,320 KiB, ratio 3.961673x (limit 6x).
Repeated scratch stays 140 bytes; flat chains use 35 bytes. All work/memory
budgets pass. Generated runtime/text size: **N/A**. The earlier 168 observations
remain archived as evidence for the original implementation, not the final B.

Fresh checks: `make test-pa3` 20/20; `make test-report-through-pa3` 100/100,
3/3 stages; required file audit 39 implementation files (40 including entry).
Personal PA3 ordinary and ASan/UBSan: 72 invocations, 15,045 results, plus API
promotion/identity/recovery/allocation checks. Full PA3 ASan/UBSan: 20/20 with
an isolated binary and 60-second auxiliary timeout. PA2 ASan/UBSan personal
checks: 316 cases including 7,062 independent integers. Required optimized
checks keep their original defaults. Diff/provenance checks pass.

## Handoff ledger

- `bc1a3f303`: recorded stage baseline and streaming plan before implementation.
- `e8cf198c7`: implemented all PA3 language groups; 20 → 0 failures, prior 80/80.
- `4314a6eb3`: independent semantic, deep-stack, identity and allocation checks.
- `c0bb94df8`: prior audit, report renderer and 168 frozen performance samples.
  This handoff had not had an independent audit; all its sources, reports,
  manifests, observations and hashes were reviewed and recomputed at entry.
- `f0b92212e`: independent architecture finding fixed across both decoder and
  consumer; regression and inherited behavior checked before committing.
- Final consolidation: extended fixed benchmarks, verified A/B report, noise
  disclosure, spec ownership/profitability review and fresh exit checks. Prior
  goal turn classified as progress from authoritative committed work. No stale
  checkpoint conclusion or reused green status substituted for source review.

No unaudited PA3 handoff or deferred related work remains. The intended audit
changes are committed; the final clean-worktree and exit checks close PA3 only.
