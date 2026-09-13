# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement**; architecture remains open.
Original entry **84/314**; continuation entry/current **314/314**. All **230
original failures** are resolved. Coverage, references and comparisons are
unchanged. PA15 has not started. Last complete evidence baseline: `5ae726e0`.

## Design/spec alignment

Active continuation from `167f5f43`; previous turn: **verified progress**. Trace
typed declaration/member demand before changing scheduling. Existing body/storage
queues already deduplicate. First close repeated out-of-class definition-bucket
walks: source list head + concrete specialization owns a completed traversal;
new definitions extend an immutable tail and reuse completed prior tails.
Validate repeated overload demand, re-entrant definitions, late publication,
nested/static members and rejection behavior. Work should follow unique requested
heads and newly visited definition edges, with no global invalidation or output
change. Extend into related application-state/dependency ownership after this
base owner is established; measure the full frozen corpus and source/key scaling.

| Owner / data flow | Complexity and validation |
| --- | --- |
| Declaration types, regions, defaults and typed values (inherited) | Immutable source topology → declaration-owned frames → canonical type/value queries → demanded concrete facts and ABI output. Source indexes once per region; defaults/queries once per complete key. Earlier proofs, work equations and measurements remain. |
| Expression properties and uses (implemented) | Source properties → shared fact ID → sparse concrete entity/receiver/incoming/evaluation use. Publish only changed properties; conversion variants use source/sequence IDs. Snapshot reads survive recursive growth. Validate identity separation, conversion sharing, defaults, moves and conditional/full-expression lifetimes. |
| Call-input and local expression views (implemented) | Source argument slice → existing occurrence context → concrete argument use; materialized arguments keep concrete slices. Semantics, lowering, conversions, unwind and cleanup use one typed accessor. One stack AST view per affected visit removes repeated projections without a persistent cache or new semantic decisions. |
| Declaration/scope/object/lifetime graph (remaining) | Whole demanded regions still allocate occurrence IDs and dense Fact slots. Joint ownership must distinguish newly required context identities from fixed facts and eliminate remaining fixed rechecks. Validate local classes/enums, storage, return/declaration conversions and cleanup alongside source/instance scaling. |
| Demand/failure dependencies (remaining) | Complete typed reasons, reverse edges, separate declaration/definition/layout/default/exception/body/vtable/emission states and structured expected failures. Compute once per complete key; validate recursive demand, negative keys and unrelated-declaration scaling. |

**Concrete boundary:** `demand_region` and `instantiate_function` still establish
NodeIds consumed by parameter/local declarations, block scopes, object/storage
identities and lifetime actions. `Fact::target/type/entity/scope/value` crosses
those owners. Expression properties and argument views cannot replace these
identities: changing only their store would alias distinct local objects or omit
required declaration/lifetime work. Further removal of fixed rechecks requires
that joint producer/consumer change, with broader demand/failure validation.
This closes the expression ownership and access group, including its measured
regression. The remaining current-stage work has no external blocker.

## Performance evidence and budgets

All **10,514 observations** verify: 9,394 inherited plus two frozen campaigns of
560 observations each (32 compiler inputs/eight executables). See
[performance.md](performance.md) for medians, paired results, spreads, hashes,
all costs and the retained initial slowdown. Final entry/current large-body
medians are 2.198947→1.992706 s, layout offsets 2.428378→2.147875 s, and repeated
call inputs 2.018831→1.799594 s. Each improves in both ABBA blocks. Peak RSS falls
35,086 / 35,352 / 46,076 KiB respectively. Tiny compiler inputs retain startup
noise, small median increases and an unfiltered .049191 s B outlier. All 32 LowIR
and eight executable hashes match; generated-code growth is **zero**. No runtime
optimization is claimed. Initial and final measurements both remain verified.

Properties/use records are 24/20 bytes with a four-byte occurrence-to-use index.
For N instances and W source operations, used-body properties are `4N+4W+4`,
offset properties `3N+6W+1`, and call-input properties `3N+5W+8`; concrete uses
track emitted work. Retained call-input edges are `N+2W`. Expression/conversion
and query work is unchanged from the equivalent entry compiler. Public
Entity/Expression/ObjectUse remain 112/36/36 bytes; Analyzer grows 5,576→5,680
bytes per TU. Compiler text is 1,303,046 bytes, **+8,768 (0.677%)**. Current and
historical transitive-header layout probes verify.

Explicit budgets: at most four conversion variants per source operation,
source/key/concrete-use-proportional storage, one local view per affected visit,
and zero generated growth. O0 has no mandated numeric latency/RSS/text ceiling.
Measured compilation and memory savings justify the recorded owner/text costs.
Historical live-header equality is snapshot evidence; the newest probe checks
live headers. Compact occurrences share one parsed graph; zero occurrence count
is not an extra numerical gate. Fixed semantic rechecks and incomplete demand
ownership remain spec work. No mandated limit or measurement was removed. The
inherited source-cache +6,714 KiB and calls-4 roughly +15 MiB RSS deltas remain
unexplained; these campaigns do not establish their cause.

## Handoff ledger

| Coherent increment | Commit / evidence |
| --- | --- |
| Field/receiver/prototype, declaration types and immutable frames | `78bdbc3f` through `ca42e706`; earlier proofs retained |
| Region/default ownership and typed layout values/bounds | `feac8cd6` through `5a795af4`; prior 9,394 observations and rejection/native proofs |
| Immutable expression properties and concrete use state | `4aedd84c`; snapshot and conversion-variant control |
| Contextual call inputs and sparse concrete identities | `158e7b0a`, `c4e4e6f4`; lifetime/native controls and source/instance equations |
| Frozen workload/layout and initial ownership evidence | `1faaabd9`, `5780dfbe`; 560 observations, unchanged outputs, offset regression retained |
| Local semantic/lowering views and final validation | `5ae726e0`; separate 560-observation campaign resolves the offset regression |

Validation: PA14 **314/314**, prior **1621/1621**, through **1935/1935**, **24**
native programs, **338** release/ASan/UBSan parity inputs, **124** rejection
controls on both compilers, **six** ABI controls, **seven** reducer/native checks,
plus storage/snapshot and entry/release/sanitizer lifetime controls. File audit
passes with three inherited header advisories. Raw evidence and verifiers live
in `student.tests/pa14/`; frozen binaries, outputs, profile, command/status logs
and initial failed probes remain under `$RALPH_ARTIFACT_DIR/pa14-expression-owners/`.
