# PA10 implementation

Stage base commit: c2a4786ebe60f52a7384c8c6e1ce12bd7e0b173a
Last reviewed commit: c2a4786ebe60f52a7384c8c6e1ce12bd7e0b173a

## Design and ownership

The parser/semantic analyzer owns the single source graph, canonical types,
declaration identities, selected calls and conversions. PA10 consumes these
facts directly into PA8's typed Program; PA9's typed encoder owns external
names. No textual phase transport or repeated overload resolution.
Translation-unit vectors indexed by EntityId/NodeId own lowering mappings;
function-local builders own value/slot/block IDs and release transient state
after each body. Work is proportional to semantic nodes and produced IR.

## Remaining work

Course behavior is complete: 121/121 including 5/5 focused controls. The two
constant-initialization corrections are proved in `reference-corrections.md`.
Finish the frozen performance campaign and sanitizer audit, then rerun exit
checks and record final evidence. No later object-model/template lowering is
part of this procedural stage.

## Performance evidence

O0 adds required lowering only, with no optional optimization passes. Measure
latency/RSS and IR work on fixed growing procedural workloads; execute eligible
outputs through the allowed reference backend and report runtime/text size.
The stage base produces no LowIR: it is not a semantically equivalent A/B
baseline. No optimization benefit is claimed without the spec's frozen ABBA,
A/A and equivalent-output protocol. Later template/backend gates are deferred
to their owning stages; all mandated current-stage constraints remain.

## Handoff ledger

- Entry: clean base above; prior turn supplied evidence of the unimplemented
  driver (progress evidence, not a live wait). Baseline 0/121, 121 failures.
  Earlier stages and file audit reported passing; rerun at exit.
- Procedural implementation checkpoint: 119/121; all five controls pass.
  PA1–PA9 904/904 and file audit (125 files) pass. Scalar/CFG/call/array/
  reference groups now work together. Two remaining reference facts defer
  mandatory constant initialization; reduced reference executions both exit 1
  where C++11 requires 0. Proof review and corrected sidecars are next.
- Still active: complete reference proof, personal native/API verification,
  architectural cleanup and frozen performance evidence. No handoff yet.

- Audit increment: full through report 1025/1025; seven independent native
  programs and seven semantic rejections pass. Added explicit in-memory IR
  validation, constant-fact memoization, shared indirect signatures, and
  function-owned operand scratch. Fixed discarded volatile reads, bool
  increment normalization, aggregate padding and ABI/IR global-name collisions.
  The successful course corpus also passes `--validate-lowir`.
