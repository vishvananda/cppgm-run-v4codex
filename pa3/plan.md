# PA3 implementation plan

Stage base commit: `dddda5eec41799b1e45b39e896c3a6f3da1c5a3b`
Last reviewed commit: `dddda5eec41799b1e45b39e896c3a6f3da1c5a3b`
Last implementation change: `e8cf198c7`
Target: **PA3 full-stage**. Baseline: 0/20, 20 failures. Final: 20/20, 0 failures.

## Design/spec alignment and completed groups

Immutable source → PA1 streaming cursor/interned names → PA2 scalar decoders
→ incremental controlling-expression evaluator → explicit decimal output.
One parse, no token vectors, syntax tree or text transport. The evaluator owns
reusable value/operator scratch; typed values retain signedness and deferred
arithmetic errors. Both arms remain syntax/type checked; selection determines
which errors affect the result. The injected `defined` query supports PA4 reuse.

| Group / owner | Data flow and complexity | Validation |
| --- | --- | --- |
| Primary conversion / PA2 decoders + expression adapter | PP identity/scalar → signed or unsigned 64-bit value; O(spelling bytes) | primary, Unicode, every integral promotion, defined and malformed operands |
| Grammar/arithmetic / expression evaluator | tokens → iterative compact stacks; O(tokens), O(pending nesting) scratch | precedence, chains, alternatives, 492,075 triples, signedness, shifts and overflow |
| Selection/recovery / evaluator + CLI | deferred errors → selected result; newline resets syntax | lazy branches/types, unconditional invalid-token rejection, malformed expressions, phase errors and logical lines |
| Evidence / personal harness | independent typed-tree oracle, counters, frozen latency/RSS runs | personal/API/sanitizer checks, course/through suites, file audit |

All groups are complete. Spec §§1–2/5/8–10 apply to compact identities, direct
streaming, explicit ownership and bounded work. Typed declaration graphs,
templates, IR/backend and executable optimization remain later-stage work.
The [completion audit](audit.md) traces data flow, selection, lifetimes and each
contract requirement. Existing course fixtures/references/harnesses and PA1/PA2
implementation sources are unchanged; no coverage has been reduced.

## Performance and validation

[Evidence](../student.tests/pa3/performance.md): 168 retained observations from
two frozen seven-workload campaigns, with A/A calibration and ABBA telemetry
comparisons. The final ordinary 4/16 MiB workload measures 0.437491/1.735391 s,
8032/20320 KiB peak RSS and a 3.966689x latency ratio (budget ≤ 6x).
Expression scratch remains 140 bytes; a two-million-operation flat chain uses
35 bytes. Deep nesting, 200k names and recovery pass all declared work/memory
budgets. Timing noise and instrumentation cost are disclosed; no speedup or
optimization-profitability claim is made. Host-tool text: 86,656 bytes.
Generated executable runtime/text size: N/A (PA3 emits expression values).

Validation: `make test-pa3` 20/20; through-PA2 80/80; through-PA3 100/100;
required file audit 39 files (40 including the stage entry point); diff check
passes. Both ordinary and ASan/UBSan personal checks pass 67 invocations / 15,015
expression results plus API identity/promotion/allocation tests. Full PA3 under
ASan/UBSan passes 20/20 with a 60-second auxiliary timeout. The first sanitizer
run exceeded the default 10-second triple-fixture limit; the optimized required
checks pass their unchanged defaults. Frozen build/input hashes match current
implementation; intended changes are committed and the final worktree is clean.

## Handoff ledger and reason

- `bc1a3f303`: instructions/spec/handout and clean HEAD inspected; stage markers
  recorded before implementation. Entry checkpoint: no PA3 progress (stub, 20
  failures); revalidation identified implementation as the next safe action.
- `e8cf198c7`: all language groups implemented together; existing failures drop
  20 → 0, prior 80/80 and file audit pass. Work continued beyond the first green
  suite to independent semantic, ownership and resource validation.
- `4314a6eb3`: personal AST/boundary/stress/API checks and reproducible performance
  harness; 200k nesting and zero allocations after scratch warmup verified.
- Final completion record: full sanitizer course pass, two frozen performance
  campaigns and requirement-by-requirement audit. Original review markers remain
  unchanged. Handoff reason: PA3 full-stage is complete; no related PA3 behavior
  or evidence group is deferred, and no later milestone was started.
