# PA3 implementation plan

Stage base commit: `dddda5eec41799b1e45b39e896c3a6f3da1c5a3b`
Last reviewed commit: `dddda5eec41799b1e45b39e896c3a6f3da1c5a3b`
Target: **PA3 full-stage**. Entry baseline: 0/20 passing, 20 failures. Current: 20/20 passing.

## Design and remaining groups

Immutable source → PA1 streaming cursor/interned names → PA2 scalar decoders
→ incremental controlling-expression evaluator → explicit decimal output.
One parse, no token vectors, syntax tree or text transport. Evaluator scratch
belongs to the consumer and is reused between logical lines. A typed value
retains signedness and deferred evaluation errors, so short-circuit selection
suppresses arithmetic errors while both branches remain syntax/type checked.
The `defined` query is injected for PA4 reuse; PA3 owns its mock policy.

| Group / owner | Data flow and complexity | Validation |
| --- | --- | --- |
| Primary conversion / PA2 decoders + expression adapter | PP identity/scalar → signed or unsigned 64-bit value; O(spelling bytes) | primary, Unicode, defined and malformed operands |
| Grammar and arithmetic / expression evaluator | tokens → compact iterative operator/value stacks; O(tokens), O(nesting) scratch | precedence, chains, alternatives, triples, comparisons, shifts and overflow |
| Selection and recovery / evaluator + CLI | deferred value errors → selected result; newline resets state | lazy branches, conditional types, malformed expressions, phase errors, logical lines |
| Evidence / personal harness | fixed input/output checks, counters, latency and RSS | explicit personal checks, sanitizers, course/through suites, file audit |

Spec §§1–2/5/8–10 apply now: compact identity, streaming, bounded storage/work,
explicit ownership, telemetry and self-contained conversion. Later typed graph,
template, IR/backend and executable optimization requirements remain later-stage
work. PA3 emits values, so generated executable runtime/text size is N/A.

## Performance evidence

Pending fixed workloads and explicit work/memory/scaling budgets. Language
groups pass the full 20-case course suite, including 492,075 triple expressions;
through-PA2 passes 80/80 and the source file audit passes 39 files. No speedup
claim against the nonfunctional entry stub. Any comparative claim requires
frozen binaries/flags/inputs, equivalent outputs, A/A calibration, ABBA wall
timings and retained observations per spec §9.

## Handoff ledger

- Entry: clean HEAD and authoritative logs inspected; previous checkpoint is
  no progress (stub, 20 failures). Revalidation identifies implementation as
  the next safe action. All four groups remain; no handoff boundary reached.

- Implementation: primary conversion, complete iterative grammar/arithmetic,
  lazy error selection, conditional type propagation and line recovery pass
  all 20 existing fixtures (20 → 0 failures). No fixture/coverage changes.
  Shared PA1/PA2 source remains unchanged. Personal stress/API/sanitizer checks
  and frozen performance evidence remain; passing tests is not a handoff.
