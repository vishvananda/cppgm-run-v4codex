# PA25 checkpoint audit138

Stage base commit: fee6ad9076ff35c5272526e1c4c4df235fbf3bfe
Last reviewed commit: 4530fe939e95a45ff6a46d5e53c76807b5bdd352

Target: **PA25 full-stage**. Phase: **checkpointAudit**. The first accumulated
review covers the stage base through the code tip above: all 12 original commits
and the cohesive audit fix. Three accepted implementation checkpoints are
reviewed together. This checkpoint preserves **74/101**, the same **27 failures**
as entry `34fe77a3`; PA25 remains incomplete and advancement is not approved.

## Reviewed ownership and fixes

The production path remains streaming source -> canonical syntax/semantic facts
-> typed LowIR -> per-function MIR -> native object -> indexed ELF linker.
[Audit138](audit.md) records each commit, cross-handoff findings, proofs and one
ledger row. Its fixes are committed before these records.

| Owner | Completed audit repair / bounded behavior |
|---|---|
| Semantic constants | Exact enum range/type selection and normalized payloads; checked implicit increments, fixed enum ranges, non-finite casts and integer-to-floating narrowing; typed constant views |
| ABI literals | Preserve enum type, full 128-bit value and sign through canonical facts, explicit adapters and mangling; common nodes stay 32 bytes |
| Native definitions | Producers record fixup owners; aliases share definition identity; version-2 objects persist facts; foreign extents decoded once; lazy GOT demand uses O(symbols + relocations) worklist |
| Builtin role | Semantic builtin fact reaches native emission through typed metadata; only the external LowIR adapter decodes legacy spelling; metadata stays 36 bytes |
| Statement/template/lifetime | Combined source-to-ELF trace confirms canonical occurrences, demanded body work, scoped query reuse and function-owned lifetime overlays |

Object version 1 files must be rebuilt. No assignment fixture, reference output,
comparison rule or required behavior changed. The floating oracle is retained:
C++11 permits excess precision, so the reducer does not prove it erroneous.

## Evidence and acceptance

- `make test-pa25`: **74/101**, exit 2, exactly the same 27 failing cases.
- Required through-PA24 command: **4152/4152**, all 24 stages pass.
- File audit: pass, four inherited header warnings; `git diff --check`: clean.
- Explicit controls: **122 audit**, **61 driver**, **19 scalar** plus **96**
  randomized full-width operand pairs, **49 statement**, **4 ABI/API**, and
  **16** final-binary source/AST/validated-LowIR/MIR/ELF trace checks.
- [Validation138](../student.tests/pa25/validation138.json) pins commits, source
  inventory, binaries, logs, exact failure sets, traces and evidence hashes.
- [Performance138](../student.tests/pa25/performance138.md) preserves 1624
  observations and verified input/binary manifests. Exact final template compile
  median **0.32798 s**, B/A **1.061 [0.794, 1.304]**, peak **14900 KiB**;
  equivalent executable pairs remain byte-identical. No speedup is claimed.

Stage-scoped acceptance supersedes inherited self-imposed timing gates: the 15%
latency/RSS and zero optional text-growth targets are diagnostics. Required
correctness, coverage, complexity and native bounds remain. Necessary semantic
and definition work is bounded; no optional optimization pass is added. Existing
copy elision consumes selected semantic facts with conservative fallback.
PA34 owns self-hosting, and later stages own higher optimization levels.

## Remaining work, grouped by ownership

| Group | Required coverage / retained work |
|---|---|
| Native C++ runtime and source EH integration | 12 class/RTTI/allocation/pure-virtual failures and 14 exception/unwind/handler failures; carry recorded hierarchy, payload, lifetime and function-try facts through shared support definitions and native execution |
| Source semantic completion | 1 exact floating-oracle failure (five of one million lines); resolve source evaluation policy without changing generic LowIR arithmetic or comparison coverage. Retain the inherited unused-dependent-local reducer as separate frontend completion work |

[Remaining137](../student.tests/pa25/remaining137.json) names the unchanged cases;
[rounding136](../student.tests/pa25/rounding136.md) preserves the precision proof
and bundle identity. These are unfinished implementation requirements, not waived
audit gates. Complete a broad owner group with its interactions and evidence in
one handoff. Avoid a handoff per runtime symbol, test case, plan or telemetry
adjustment; the earlier separate planning/evidence commits added avoidable review
fragmentation around otherwise substantial driver, scalar and statement groups.

Full-stage completion still requires `make test-pa25` and the root through-PA25
report to pass. Raw audit artifacts are in
`/home/vishvananda/work/private/v4codex/artifacts/pa25-138/`.
