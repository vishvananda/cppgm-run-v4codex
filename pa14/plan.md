# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement**; architecture remains open.
Original entry **84/314**; continuation entry/current **314/314**. All **230
original failures** are resolved. Coverage, references and comparisons remain
unchanged. PA15 has not started.

## Design/spec alignment

Active continuation from `5a795af4`; previous turn: **verified progress**.
Split immutable expression facts from per-use incoming conversion/evaluation
state. Source-owned facts publish once; substituted object/call facts publish
new identities. Fixed-expression reuse aliases the source fact until a concrete
result differs. Storage/work follow source facts, changed facts and use edges;
no content hash or syntax replay. Extend the same ownership split to retained
call-input slices and source/context views, so concrete argument use IDs do not
force copies of fixed callee/conversion facts. Validate all prior identity, default, scalar,
materialization and cleanup controls, then measure the full frozen corpus and
source/repetition scaling before accepting the representation cost.


The initial expression-owner campaign is frozen and verified: 560 new / 9,954
total observations. All 32 LowIR and eight native outputs match exactly. RSS
falls on the larger workloads, but value-offset-1000-128 rises 2.418760→2.551136 s
with paired ratios 1.0299/1.0630. This regression is under investigation before
acceptance. A retained CPU profile identifies source/context index lookup and
repeated full AST views as the dominant cost; testing bounded local view reuse
is the next related increment. No timing observation or historical gate is removed.


Continuation from `c78e8d3b`: **verified progress**. This group closes typed
layout-query values and extends that owner into dependent array bounds,
qualified constants, conditional/logical queries, scalar casts and ABI output.
sizeof/alignment retain fixed result types while values depend on the template.
Fixed operand obligations are checked at definition time, including operands
whose values will be short-circuited. Source conversion decisions survive value
substitution; shared O0 policy variants preserve entry LowIR/native bytes.

| Owner / data flow | Complexity and validation |
| --- | --- |
| Declaration types, source regions and defaults (inherited) | Immutable source topology → demanded regions and declaration-owned frames → concrete facts. Source indexes once per region; defaults once per complete key, with declaring-head identity and separate body/initializer demand. Prior evidence retained. |
| Typed values and bounds (implemented) | Source expression/type → canonical query → immutable frame substitution → cached constant → ordinary array type and direct ABI graph. Query Active/Success/Failure states and complete typed keys; source scalar decisions shared. Work follows unique queries and demanded values; conversion policy has at most four variants per source operation. Validate casts, short circuits, access, deduction, renamed heads, parameter adjustment and native output. |
| Dependent body identities and lifetimes (remaining) | Used regions still project all source occurrences and allocate fact slots. Expressions, local class/enum/object identities, receivers, materialization, storage, cleanup and declaration/return conversions need explicit source/context overlays consumed by semantics and lowering. Validate identity/lifetime separation and source/instance scaling. |
| Demand/failure dependencies (remaining) | Extend the separate declaration/definition/layout/default/exception/body/vtable/emission states with typed reasons, reverse edges and structured expected failures. Compute once per complete key; validate recursive demand, negative keys and unrelated-declaration scaling. |

**Concrete boundary:** `demand_region`/`instantiate_function` still resize facts
and expressions for every projected NodeId. Local declarations, object uses,
conversion inputs and cleanup consumers depend on those concrete IDs. The new
query owner can substitute types/values but cannot identify a local object or its
lifetime. Removing the projection requires a joint declaration/object/lifetime
overlay and corresponding lowering changes; extending constant evaluation would
either duplicate those decisions or alias distinct objects. This handoff closes
the coherent value/bound owner. The remaining representation change is a separate
current-stage group, with no external blocker or later-stage exemption.

## Performance evidence and budgets

All **9,394 observations** verify: 8,932 inherited plus 462 from the frozen
28-input/seven-executable value campaign. [performance.md](performance.md) retains
all samples, A/A calibration, ABBA pairs, spreads, hashes and costs. Repeated
layout offsets at N=1,000 W=128 improve 2.574320→2.446097 s, paired ratios
.9453/.9402; peak RSS 395,534→372,962 KiB. W=8 also improves in both blocks.
The used-body and N=4,000 offset results retain outliers and do not establish
separate repeatable timing gains. Unused-body medians rise about 1%, calls 0.3%;
the small used-body RSS rises 1,270 KiB. No runtime optimization is claimed.

Used-body expression/conversion work is `4N+2W+3` / `3N+2W+2`; offset work is
`3N+4W+1` / `3N+4W`. Both evaluate N canonical layout queries with NW uses and
W conversion variants. Bound query work is `3N+1`. Storage is source/key/occurrence
proportional; whole-region occurrences remain. Compiler text is 1,294,278 bytes,
+11,520 (+0.898%). Entity/Expression/ObjectUse remain 112/36/36 bytes, frames 20,
occurrences eight, Ast 504, query/type facts 48/48 and queried values eight.
All historical layout snapshots remain; the new probe verifies live headers.

Explicit budgets remain at most four conversion variants per source operation,
source/key-proportional query storage and **zero common-correct generated-code
growth**. O0 has no mandated numeric compiler latency/RSS/text ceiling. Required
semantics and repeatable offset savings justify the recorded costs. No optional
optimizer was added; unsupported historical diagnostic gates remain observations.
The inherited isolated source-cache +6,714 KiB and calls-4 roughly +15 MiB RSS
deltas remain unexplained; this campaign does not establish their cause.

## Handoff ledger

| Coherent increment | Commit / evidence |
| --- | --- |
| Field/receiver/prototype ownership, declaration types and immutable frames | `78bdbc3f` through `ca42e706`; prior proofs and measurements retained |
| Region/default ownership, declaring heads and source topology cache | `feac8cd6` through `c78e8d3b`; six new default rejections, prior 8,932 observations and source work equations retained |
| Typed layout values and fixed source scalar decisions | `8bb47eb3`; eight value rejections, six newly rejected unused bodies |
| Canonical bounds, qualified values, short-circuit/cast queries and O0 conversion variants | `66fe52c1`; dependent-bound native control and exact conversion reducer parity |
| Frozen workload, standard proofs and current transitive-header probe | `025cb21b`; 22 rejection proofs, three native proofs, immutable entry/final binaries |

Validation of `66fe52c1`: PA14 **314/314**, prior **1621/1621**, through
**1935/1935**, **23** native programs, **337** release/ASan/UBSan parity inputs,
**124** rejection controls on both compilers, **six** ABI controls and **seven**
reducer parity/native checks. File audit passes with three inherited header
advisories. Raw evidence/verifiers are committed in `student.tests/pa14/`;
command/status manifests, frozen binaries, outputs and initial failed probes
remain under `$RALPH_ARTIFACT_DIR/pa14-value-facts/`. Full-stage graph work remains
at the concrete ownership boundary above.
