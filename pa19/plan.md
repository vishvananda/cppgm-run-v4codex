# PA19 implementation plan

Stage base commit: `e5f4c3ed78972c8d161671d145bf525cb99033f4`.
Last reviewed commit: `e5f4c3ed78972c8d161671d145bf525cb99033f4`.
Target: PA19 full-stage. Phase: implement; independent PA19 audit pending.
Entry: 397/423 required cases pass (26 failures); earlier PA1–PA18 pass.
The previous interrupted turn has no evidenced implementation or live job;
this turn revalidated the clean checkout and failure log.

## Design and work groups

Retain the streaming parser, canonical typed graph, specialization fact owners,
lazy demands and direct typed LowIR path. Group failures by their shared owner
before editing; record concrete data flow and complexity with each repair.

- Type composition and deduction: function/reference/array shells, qualified
  aliases, defaulted class packs, constructor ordering.
- Dependent lookup/substitution: member aliases/variables, qualified results,
  explicit-template ADL, default-argument queries and no-eager SFINAE.
- LowIR mismatches: inspect selected semantic entities, demand and ABI facts;
  distinguish implementation faults from independently proved oracle defects.

## Performance and validation

PA19/O0 has no mandated numerical latency/RSS ceiling. Preserve inherited
measurements; their self-selected thresholds are diagnostics under spec §9.
Freeze entry/final binaries and inputs; measure compiler wall time/RSS with
A/A calibration and ABBA, verify equivalent output before comparison, and
measure checked native runtime/payload size through the supplied backend.
Newly correct behavior receives final-only costs, not spurious speedup claims.
No optional optimization is planned. Record scaling and existing work bounds.

Required: `make test-pa19`, `make test-report-through-pa18`, file audit for
`dev/src`; finish with the through-PA19 report when the stage passes. Personal
controls live in `student.tests/pa19/` and run explicitly. Preserve all 423
course cases and their comparison rules.

## Handoff ledger

Implementation unfinished: all 26 entry failures pending diagnosis.
Independent review: whole-stage correctness/architecture/performance audit
remains pending; review markers above must not move during implementation.
No completed behavior group or handoff boundary yet.
