# PA32 implementation plan

Stage base commit: e82bf4152fe8d6d68b9cd966655db0d8142cf81b
Last reviewed commit: e82bf4152fe8d6d68b9cd966655db0d8142cf81b

## Design and ownership

The turn starts at 0/425 required cases (425 failures). The previous goal turn
has no live process; inspection provides new evidence: the optimizer is a
scaffold and all three driver routes lack the shared optimization boundary.
Keep the existing arena/pool typed LowIR and direct native/ELF pipeline.

1. LowIR entry/transport: lowiropt owns CLI, external parsing/validation and
   writing; the driver owns source/LowIR dispatch and level/debug policy.
   Source -> typed Program -> shared optimizer -> writer or native backend.
   Preserve section, ABI, debug and parameter object-extent metadata on replay.
2. Local scalar/CFG simplification: function-owned dense value facts, operand
   users and dirty worklists; fold safe integer operations, propagate copies,
   simplify branches and remove dead pure definitions. Linear/near-linear work,
   no growth; conservative fallback for unknown arithmetic/effects.
3. Remaining implementation: slot promotion, bounded dataflow/CSE, loops,
   inlining/specialization and their source/debug lanes. Group by semantic owner
   using failing predicates after the shared path works.

## Acceptance and evidence

Required: unchanged course fixtures/envelopes, earlier through report, PA32
course/debug lanes, file audit. Personal tests are explicit under student.tests.
Freeze binaries/flags/inputs for A/A and ABBA compiler wall/RSS and executable
runtime/text measurements. O0 does no optimization work; O1 local transforms
have no code growth, bounded work and conservative legality. Later backend
allocation quality remains PA33-owned; no unsupported diagnostic threshold is
an extra gate. No optimization performance claims before measurement.

## Handoff ledger

First implementation checkpoint: shared CLI/level/hosted LowIR routes, deterministic
ELF symbol order and function schedule, bounded scalar/CFG simplification and
block-local slot forwarding/CSE are implemented. Primary report: 106/219
(113 failures), versus 0/219 at entry; Ralph's entry census was 0/425 and is
kept as a separate accounting basis. All 25 no-debug object replays pass.
Explicit personal integer/phi/mutable-value test: 501 cases at four levels pass.
An inherited native mutable-temporary alias defect was fixed with stable homes.
Prior through report before that fix: 5178/5178; final revalidation pending.
Do not run root reports concurrently: they share report artifacts.
Debug direct O1: 3/3; cross-block slot promotion/debug and source locations remain
implementation work. Performance measurement and final checks are pending. Whole-stage audit
is pending independently: trace metadata transport, pass legality/budgets,
source-to-ELF ownership and performance evidence. Review markers above remain
unchanged until independent review. No requirements waived.
