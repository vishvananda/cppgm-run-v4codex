# PA24 final audit134

Stage base: `bde9eb3e128e24923a1de40bb63b8e348a13553b`.
Entry: `63dc6c11`, clean. Final production repair: `a8168480`.
Last reviewed commit: a81684807f1f2c6da2cd83d99fa3d08dfaaf770a
Target: **PA24 full-stage**, native LowIR execution and MIR at O0.
Disposition: **accepted for advancement**; no outstanding in-scope finding.

The preceding goal turn was progress: committed implementation and its primary
test log established the completed course surface. No earlier process was live
at entry. This audit read `spec.md`, the PA24 handout/encoding lesson, LowIR
contract, testing/layout instructions, stage history, plan and evidence. It
independently reconstructed native owners and consumers, the 42-file
production/build delta, shared LowIR changes and representative frontend demand
and lowering paths. [Checkpoint130](audit130.md) is preserved as history, not a
substitute for this review. `review-inventory.json` and `stage-commits.txt`
record all 20 entry-stage commits and source hashes.

## Findings and changes

One new correctness defect crossed placement, CFG flow and phi emission. With
five or more parameters surviving a call, native placement can reserve both an
incoming carrier and an immutable frame home. `value()` chooses the carrier only
while the current path's clobber mask permits it. `Selector::run()` emitted
deferred branch/switch edge blocks after all source blocks, but retained the last
source block's mask. A last block on a no-call path could make a post-call phi
read an overwritten argument register.

`a8168480` publishes the final mask for each predecessor while selecting its
body, then restores that fact when emitting its deferred transfers. Direct jumps
already execute with the right state. The repair applies to every delayed edge
and every parameter position/type; it does not disable promotion or force global
spilling. Added storage is four bytes per LowIR block plus vector overhead;
work is one write per block and one read per delayed edge. The fact lives in the
unit's dense workspace, shares stable block identity, and expires with the unit.

[audit134.py](../student.tests/pa24/audit134.py) retains reduced inputs and numeric
checks. Frozen entry fails **36/61**, all conditional/switch variants across six
argument registers and `i8/u16/i64`; corrected code passes **61/61**. Direct jumps
are positive controls. Other cases exercise TLS exception payload widths and
unwind across a dynamically allocated callee while retaining the destination
frame's allocation. The proof is the [LowIR phi contract](../pa8/lowir.md):
transfers read predecessor values in parallel; a call cannot change a parameter's
logical value. No reference change or compiler-agreement argument is involved.
Initial malformed generator variants and rejected diagnostics remain in evidence.

## Final Spec Alignment and ownership

| Spec | Current architecture and audited invariant |
|---|---|
| §1 source/parse | `lowering/driver.cpp` connects immutable preprocessor buffers and interned identifiers through post/syntax cursors to `Parser::translation_unit` and `Analyzer::consume`. Source nodes and typed facts share identity. `syntax/occurrence.cpp` retains immutable source regions and projects compact context occurrences; grammar reinterpretation after publication is rejected. No template token replay or second complete syntax tree. |
| §2–3 identity/lookup | Entity/Type/Scope/Node IDs and flat `IdIndex` tables key semantic facts. Selected calls/conversions, field layouts and ABI entries feed lowering by identity. Interned names/manglings are boundary spellings. Native values, slots, symbols and blocks use dense compact IDs, never rendered MIR or mangling lookup. |
| §4 templates | `substitution_frame` keys specialization, parameter range/count, parent and selected arguments. Parent-linked frames and dependent-only type substitution preserve fixed facts. `reuse_fixed_expression` inherits immutable expression/conversion recipes. Definition, body, layout and member demand are distinct monotonic states; class completion does not demand an unused member. |
| §5 scheduling/cache validity | `Analyzer::finish` advances deduplicated demand cursors; completion/failure belong to the requested fact. Substitution caches key pattern and complete environment. Native parameter flow uses monotonic six-bit facts and queues only newly contributed bits. Delayed edges now consume their predecessor's exit fact. No global retry, generation-wide invalidation or stale cross-function cache. |
| §6 typed lowering | `Procedural`, signature plans and `FunctionBuilder` consume recorded declarations, conversions, layouts and lifetimes into typed LowIR. External LowIR validation is the PA24 adapter, performed once. Native phases never serialize/reparse LowIR, MIR or assembly. Missing location/signature facts remain invariant failures. |
| §6–7 selection/ABI | Per-function `ValueState`, use intervals, aliases, deferred addresses, fixed effects and shared `AbiCursor` determine locations. Caller/callee share scalar/XMM/object classification, register rollback, stack alignment and hidden results. Register/address dependency masks order argument transfers. Unknown clobbers, joins and pressure preserve homes. |
| §7 MIR/encoding | Flat instruction arrays carry typed operands, debug locations, conditions, call masks and frame policy. Dump and encoder read the same facts. Reserved scalar/vector scratch is outside ordinary allocation. Integer/SSE/x87/multiword/atomic instructions and labels encode directly; typed fixups patch after layout. |
| §7 runtime/layout | Generic EH uses an image-owned handler top/payload and 80-byte dynamic records. Unwind pops before transfer and restores stack/base/preserved registers; frame floors retain function-lifetime allocations. Overaligned storage uses a reserved base. TLS uses storage/wrapper IDs, initial-thread FS setup, `TlsAddr` MIR and thread-offset fixups. Demanded accessors use the common function encoder. |
| §8 ownership | TU arenas/pools and flat indexes own frontend/source facts. Typed LowIR is retained for the explicit input/output boundary. Native workspace retains dense unit indexes; selector vectors, assignments and MIR die after each function. Image code/data/fixups and reusable label capacity last until ELF emission. No per-node owning pointer graph, cloned bodies or process-global mutable cache. |
| §9–10 bounds/evidence | Selection/encoding/layout are linear except sorted CFG/phi edges, O(E log E). Fixed pools, bounded carry windows and monotonic flow cap work. Telemetry observes existing work. Source review, file audit, own ELF execution and empty-PATH output identity establish self-containment. |

The trace crosses two explicit adapters: PA23 requests LowIR output; PA24 accepts
LowIR input. Combined source driver, relocatable objects, hosted EH/TLS linking
and self-hosting belong to PA25–31/34. Unavailable surfaces are not replaced with
host/reference output or made additional PA24 exit gates. O1–O3 add no policy
here; PA24 requires O0 and later milestones own optimizer behavior.

## Representative data and useful-fact traces

[trace134.cpp](../student.tests/pa24/trace134.cpp) defines aligned ordinary `Pair`,
`read(const Pair&)`, TLS `total`, and `Accumulator<7>::step(double)`. Source layout
records Pair's 32-byte size/alignment and field offsets 0/8. `read`'s reference
becomes `ptr [pass=by_address, object_bytes=32]`. The selected template call
records receiver/double arguments; its member consumes a field projection,
explicit FP-to-integer conversion, constant seven and the same TLS identity.
The invalid dependent `dormant` remains undemanded.

Telemetry records one class completion, one demanded member region, four fixed
expression reuses and three checked/emitted functions. Typed LowIR carries the
local `obj<32x32>` and selected calls. MIR uses r15 as aligned local base, required
parameter/FP homes, rbx/r12 to retain TLS address/value across the call, and
explicit `fptosi`. `read` loads offset zero and folds the offset-eight load into
the add. Encoding emits **389 text bytes**; both argc inputs return zero. Saved
LowIR, MIR, phase counters, raw disassembly and `readelf -h -l` show the complete
path and separate RX/RW segments. Dump/plain output bytes are identical.

The repaired useful fact distinguishes an unchanged parameter value from its
call-clobbered incoming register. Placement creates the home, CFG flow records
the kill, selection records the predecessor exit, and delayed phi emission
selects that home. MIR/disassembly show a typed frame load before transfer.
This is correctness cost, not an optimization gain.

For profitable selection, a scalar's complete interval across one adjacent
single-successor/single-predecessor edge permits a surviving register. Alias and
address consumers extend the interval before placement; joins, backedges, calls
without preserved carriers and fixed effects retain homes. Encoding removes only
proven redundant traffic. Historical ABBA evidence measured forward-edge profit
(176→161 text bytes) and XMM reuse profit (2169→1522 bytes). Final images retain
these bytes; fewer IR nodes alone are not the evidence.

## Transformations and composed budgets

| Policy | Legality/profitability and conservative fallback | Work/growth/invalidation |
|---|---|---|
| Parameter/Boolean/copy aliases | Single unescaped nonvolatile initialization or same representation; conversions retain proven 0/1; otherwise explicit operations | Linear uses/definitions, complete intervals before allocation; no later semantic mutation |
| Folded loads/indexes | Safe adjacent consumer or valid complete address-carrier interval; observed pointers/clobbers force materialization | Linear reverse pass, no memory propagation across effects or body duplication |
| GPR/XMM reuse | Last use and new interval survive calls/fixed effects; otherwise another carrier or home | Nine GPR/fourteen XMM choices per value; local facts; no search/fixed point |
| Private reload carrying | Same-block one-store 64-bit home, no unsafe implicit/explicit effect/alias; unknowns retain traffic | Three probes of at most 64 MIR instructions per store, bounded rewrite, no code growth |
| ABI/phi moves | Parallel value/address dependencies, object rollback, scratch/target protection; staging breaks cycles | At most fourteen ABI carriers and bounded cycle rounds, linear stack arguments, sorted phi edges, one transfer block per distinct edge |
| Integer/bulk encoding | Width-correct wrapping, exact copied/zeroed bytes, reserved scratch/flags; ordinary forms remain | Constant local choices; fixed 32/64-byte copy thresholds, ≤32-byte zero-form comparison; linear bytes/fixups |
| Wide/FP/atomic/EH/TLS | Full payloads, rounding/control-word restore, atomic semantics, ABI/dynamic lifetimes | Constant code per operation; 128 target division iterations, four conversion digits, 80-byte handler records, alignment-minus-one padding, fixed TLS address sequence |

Bounds compose to O(N + E log E) compiler work and linear IR/code/storage growth.
There is no new optional transform. Required canonical mixed-ABI homes and
void-output frame capacity remain actual encoder facts. `e5b5dada` removed the
earlier overbroad leaf-frame reservation; its old measurements remain preserved.

## Performance acceptance and validation

[Performance134](../student.tests/pa24/performance134.md) consolidates frozen A/B
latency/RSS, checked executable runtime/text, A/A calibration, ABBA ratios and
spread. All **37** inherited observation/identity manifests rehash correctly.
Final native compiler paired medians are **0.983–1.026**, with effectively
unchanged RSS. All ten measured runtime images are byte-identical to entry.
The unchanged frozen frontend also passes the fixed 9600-specialization workload
through our native backend; both images have **806082 text bytes**. No new
compiler or runtime speedup is claimed from timing variation.
Necessary canonical ABI cost and initial misses remain disclosed. Inherited 15%
latency/RSS and zero optional text-growth targets are diagnostics under spec §9;
neither overrides current-stage acceptance. Correctness, canonical comparisons,
MIR envelopes and bounded work remain mandatory. No reference correction was
needed: all **2403** fixture files, comparison scripts and bundle are unchanged.

- `make test-pa24`: **296/296**, plus **17/17** focused properties.
- `make test-report-through-pa24`: **4152/4152**, all **24/24** stages, plus
  **39/39** focused properties. The supplied 4315 is not the primary log or fresh
  report count; no coverage was removed.
- Required file audit: exit zero, four inherited substantial-header warnings,
  zero fatal findings.
- Explicit personal runs: **1820 scalar**, **1259 floating**, **1495 wide**,
  **160 object**, **75 EH/frame**, **236 TLS**, **205 placement**, **21 audit130**,
  **61 audit134**; ABI/debug/ELF and TLS integration pass.
- Trace passes both inputs, MIR view identity and empty-PATH compilation.
  [Validation134](../student.tests/pa24/validation134.json) binds exact commands,
  source/binary hashes and logs.

## Handoffs since checkpoint130

| Handoff | Independently reviewed ownership paths | Disposition |
|---|---|---|
| 131 `45774fa2` / `c7587f72` | Handler top/payload, nested cleanup/resume, early epilogues, allocation floor, saved frame, over-alignment/call-stack restoration | Accepted; original and mixed TLS/unwind cases pass |
| 132 `db965a86` / `03eeff9e` | TLS layout, wrapper map/demand, address consumers, startup before initializers, typed fixups and common function MIR | Accepted; 236 controls and debug/multifile/view checks pass |
| 133 `1d73b01e` / `e5b5dada` | Mixed classification, argument scheduling, stable homes, Boolean/debug facts, frame policy and canonical costs | Accepted after delayed-edge repair; 205 original and 54 edge variants pass |
| 134 whole-stage `a8168480` | Current architecture, source/demand trace, performance protocol and exit checks | Accepted; all findings repaired, evidence consolidated and required checks pass |

No implementation handoff remains unaudited. Evidence directory:
`/home/vishvananda/work/private/v4codex/artifacts/pa24-134/`.
