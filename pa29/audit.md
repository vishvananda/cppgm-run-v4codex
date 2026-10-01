# PA29 checkpoint audit162

Target: **PA29 full-stage**; checkpoint audit complete, stage implementation unfinished.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous review: `1ab3499d7046daf5c298d958a8770b413edb3615`.
Audit entry: `9662716b8a8aa0bef94f5a293d7900b28c42701c`.
Last reviewed commit: `cce8634c3c835cf6d5e8f4fa5fea0db213959718`.

The complete range covers nine entry commits, three accepted implementation
handoffs, and the cohesive audit fix. [Range evidence](../student.tests/pa29/evidence162/range.json)
records full hashes, every commit's paths/diff hashes and the combined source
diff. Audit158 remains in history at `044d9627`; its baseline is not reset to
the latest handoff. This records-only update follows the validated code tip.

## Findings and fixes

**Atomic compound assignments used increment semantics.** Every bool RMW wrote
true, including `false += 0` and `true &= false`. `atomic_update` now consumes
the selected arithmetic operation and computation type, then converts the
result to bool before CAS. Increment still works; operands evaluate once.
This preserves the arithmetic/conversion rules of N3485 [expr.ass]/7 and
[conv.bool]/1 while retaining atomic RMW. The old native code returned the
wrong result in the [bool reducer](../student.tests/pa29/controls162/atomic-bool.cpp).

**Atomic reference snapshots aliased the source.** Lowering removed atomic
identity before comparing the source object with a reference's referred-to
type, bypassing the temporary already selected by semantics. Consequently
`const int& r = atomic; atomic = 9;` changed the apparent value of `r`.
`values.cpp` now preserves atomic identity at the reference-storage boundary;
value conversion performs the atomic load into a separate temporary. Direct
atomic-reference binding still preserves object identity. Controls cover direct,
user-conversion, template and pointer-like invocation recipes; the lifetime
traits and runtime now agree on materialization. The relevant ordinary
reference/temporary rules are N3485 [dcl.init.ref]/5 and [class.temporary],
applied to the selected hosted atomic value conversion.

**Canonical signatures erased reduced storage alignment.** An aligned(1) alias
of a 16-byte type arrived at atomic lowering through a canonical pointer type.
The old code regained natural alignment and emitted `cmpxchg16b`; three new
controls crashed. Lowering now consumes the first operand's existing decorated
storage TypeId, retained through members, aliases, function returns and template
occurrences. Inline selection requires power-of-two width ≤16 and alignment at
least that width. Generic layouts use the existing libatomic ABI; scalar/sync
load/store/exchange/CAS and RMW now use that same conservative fallback when
native alignment is unproved. No name lookup or new semantic reconstruction
repairs the fact in lowering. [Declared-field reducers](../student.tests/pa29/controls162/atomic-generic-template.cpp)
avoid relying on a manufactured object lifetime in a character buffer.

The [GNU atomic contract](https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html)
documents generic runtime fallback, memory-order strengthening and strong
implementation of weak CAS. LowIR atomics retain their [PA8 contract](../pa8/lowir.md).
The fix preserves atomicity and failure-only expected-value updates, including
unaligned scalar arithmetic implemented with a bounded emitted CAS recipe.

**Atomic identity was treated as const/volatile.** The packed type flag's bit 4
incorrectly participated in object cv checks. Valid `void*` conversions,
reinterpret casts and temporary binding to `const _Atomic(int)&` failed.
The shared conversion/list/query owners now compare cv bits independently of
atomic type identity. Qualification conversions and template identity still
distinguish atomic from ordinary types; atomic payload inheritance does not
create an implicit atomic-to-base pointer conversion. Negative controls preserve
const protection. N3485 [conv.ptr]/2, [expr.reinterpret.cast]/2,7,11 and
[dcl.init.ref]/5 support the ordinary pointer/reference rules; `_Atomic` remains
the hosted extension required by the handout, not a replacement for volatile.

[Entry regressions](../student.tests/pa29/evidence162/entry-regressions.json)
show seven failed reducer checks on the frozen entry compiler.
[Final controls](../student.tests/pa29/evidence162/controls.json) pass **49/49**.
No required fixture, reference, comparison or discovery rule was changed.
No reference correction was needed or claimed. Clause citations above refer to
the checked-in [N3485 draft](../doc/n3485.txt).

## Every commit reviewed

| Commit | Reviewed content and interactions |
|---|---|
| `044d9627` | Prior audit records and full review boundary; inherited storage facts and performance classification. |
| `46fa99d2` | Legacy trait registry and structural/member versus expression-usability distinctions; shared reference-binding phases and callers. |
| `8bc08195` | Structural member cache ownership, complete class prerequisites and selected subobject cv/mutable behavior; superseded uncached traversal also reviewed. |
| `5d2b1657` | Source-prvalue destruction checked separately from result temporary destruction. |
| `8bdc6bbf` | Trait controls, preliminary/final performance observations, complete failure/remaining-work handoff. |
| `2fd6963e` | Invocation receiver recipes, query context/operator family, lane environments, fixed/dependent reuse, constant evaluation, cleanup and exceptions. |
| `7f0a2c4b` | Invocation validation/scaling evidence and residual contract questions. |
| `802f28fd` | Atomic registry/types/signatures, substitution/deduction, ABI, constant access, storage, class padding, native/runtime lowering and exception facts. |
| `9662716b` | Atomic validation/performance and 338/403 checkpoint; all retained failures and inherited obligations reviewed. |
| `cce8634c` | Four audit fixes above, independent reducers, inspection/benchmark harnesses and reproducible gate recorder. |

## Architecture and ownership audit

| Spec surface | Evidence and conclusion |
|---|---|
| §1 parsing/source | `lowering/driver.cpp` creates immutable source/preprocessor buffers, streaming PostTokenCursor/syntax Cursor and integrated Parser/Analyzer per TU. `_Atomic(type-id)` and retained template bodies are parsed once. The new owners add no token replay, duplicate syntax graph or production text phase. |
| §§2–3 identity/selection | Type/entity/query IDs and flat Index keys remain primary. Atomic signatures use operation/form/result bits plus canonical pointee identity. Storage decoration remains a distinct existing expression fact. Reference phases keep typed ambiguity/failure and selected conversions; lowering consumes those recipes. No library spelling or pretty-printed semantic key was introduced. |
| §§4–5 demand/caches | Legacy class properties key operation/type after completion; member triviality keys declaration ID. In-progress/success/failure states prevent duplicate structural work and unavailable prerequisites restore not-started state. Bodies cannot alter completed structural facts. Invocation keys include access context, operands and the captured ordinary operator family; lookup merges create immutable overload identities. Expanded operands enter their lane frame. Existing reverse query/completion edges invalidate only dependent consumers. |
| Fixed/dependent invocation | `RangeOperation` stores selected unary receiver, conversions, result, adjustment and virtual slot by compact IDs. Immutable query/source recipes do not own a runtime temporary. `project_object_use` maps retained source operands into an occurrence; evaluated use creates its own conversion/materialization records and body demands. Constant, exception and lifetime consumers share the same selection. |
| §§6–7 lowering/native | Procedural builds typed Program directly. Atomic operation/signature, reference materialization, decorated alignment, base adjustment and nonthrowing facts survive into normal LowIR operations/calls. `compile_object` → `native::compile_image` → per-function Selector/encoder → HostElf has no assembly/text transport or host compiler substitution. External LowIR validation/roundtrip is an explicit audit adapter. |
| §8 allocation/lifetime | TU arenas/slabs, interned pools, flat indexes and amortized vectors own semantic/query/receiver facts. New stored recipes have compact fields, not owning per-node pointers. Candidate/argument scratch releases at query/call exit; lowering slots/blocks have function lifetime. Native Function/MIR temporaries die after each encoding. Linkage owns at most four generic atomic runtime symbol identities per program. No mutable process-global cache is added. |
| §§9–10 evidence | Telemetry reads existing work; all inspected objects are byte-identical with stats on/off. Cache/scaling counters below observe real demand. Required output is generated by this compiler; host linking and libatomic runtime calls are ordinary authorized ABI boundaries. No test recognition or reference delegation exists in the range. |

The nontrivial declaration trace is `Holder::value` in
[atomic-generic-template.cpp](../student.tests/pa29/controls162/atomic-generic-template.cpp).
`Packed` retains the aligned(1) operand while canonical language/ABI identity is
`Pair`. Layout places the field at offset 1. `address()` preserves the decorated
result type. The selected generic builtin signature remains canonical; lowering
uses the separate expression storage fact to choose a serialized runtime call.
The final ELF contains the field-address relocation and `__atomic_*` relocations,
with no misaligned `cmpxchg16b`.

The demanded template trace is `operate<int>`/`operate<char>` in that source,
plus `invoke<Pointer>` and `snapshot<int>` in
[atomic-reference-recipes.cpp](../student.tests/pa29/controls162/atomic-reference-recipes.cpp).
Parsed bodies and fixed recipes are retained. Dependent uses substitute through
canonical frame/query identities; each demanded function body transitions once.
Pointer-like invocation selects `Pointer::operator*`, then the member receiver
and argument conversion. The selected scalar temporary survives the member's
write to the original atomic object. LowIR/native execution and telemetry agree.
[Inspection evidence](../student.tests/pa29/evidence162/inspection.json) records
the complete trace through serialization, object encoding and execution.

The repeated legacy queries still compute five query facts, four class-member
properties and three structural member facts at both 100 and 10,000 repetitions;
invocation repeated-query work and body transitions also remain constant.
[Trait inspection](../student.tests/pa29/evidence162/inspection159.json) and
[invocation inspection](../student.tests/pa29/evidence162/inspection160.json)
retain the actual counters. Distinct demanded specializations scale linearly in
the new combined workload rather than recomputing unrelated facts.

## Optimization legality, profitability and budgets

No optional optimization was added in the accumulated range or audit fix. The
new work is required semantic selection/storage and bounded target lowering.
The new optional work/code-growth budget is **zero**. Correctness fixes are not
presented as faster programs; an incorrect implementation is not a timing oracle.

Alignment is the useful fact traced to encoding: source decoration → expression
storage identity → width/alignment legality test → native atomic or ordinary
runtime call. Facts are immutable after completion; no new analysis invalidation
is needed. Native lowering is selected only for supported, sufficiently aligned
representations. Missing alignment proof selects the conservative ABI path.
Each primitive introduces constant compiler work and slots; payload copies scale
with actual width. CAS-based RMWs emit one fixed loop; native add/exchange retain
their bounded recipes. Runtime contention is not compiler search, and no
unrolling, specialization or growth fixed point is added.

Bool updates retain the selected arithmetic and bool conversion before CAS;
snapshot binding retains an atomic load and a distinct temporary. Operands are
evaluated once outside retries. Existing seq_cst strengthening, weak-to-strong
CAS, exception boundaries, ABI and source/debug identities are preserved.
External LowIR roundtrips reproduce both fallback behavior and expected updates.
Existing constexpr million-step/depth-512, native frame/data `0x70000000`,
global alignment 4096 and representable object/branch limits remain unchanged.

Actual costs are inspected. The reference reducer has a 32-byte frame: the
atomic resides at `rbp-8`, the snapshot at `rbp-16`, and its reference home at
`rbp-24`. The bool reducer retains arithmetic, `setne` and `lock cmpxchg`, with
an expected-value stack slot. Each generic template has a 96-byte frame and
runtime-call argument homes. These O0 costs are disclosed rather than inferred
from a smaller IR. No new allocator/loop optimizer is claimed.

[Performance162](performance162.md) reports latency, compiler RSS, checked runtime
and text size together, including noise calibration, paired spreads, preliminary
observations and scaling. Final cumulative common compiler ratios are
0.9299–1.0133; audit-only ratios are 0.9835–1.0016. All common A/B objects and
executables are byte-identical. New ownership costs grow with actual demand;
no repeatable avoidable regression is established. Historical blanket 15% and
zero-growth goals remain diagnostics under spec §9, not exit gates. Older
measurements and mandated limits remain intact. PA30–34 retain broad hosted
runtime, optimization/allocation and self-hosting ownership.

## Validation, residual work and handoffs

[Validation](../student.tests/pa29/evidence162/validation.json) proves required
checks on code `cce8634c`: PA29 **338/403**, exit 2; PA1–28 **4538/4538**, exit 0;
through PA29 **4876/4941**, exit 2, only PA29 fails. File audit passes with the
same four inherited header-body warnings, no new waiver. New controls pass
49/49, prior storage controls 34/34, and handoff controls 47/47, 46/46, 45/45.
The file/coverage recorder verifies all 403 inputs and all checked sidecars are
byte-identical to the previous review, with no harness/discovery/comparison edit.
The exact [65-failure set](../student.tests/pa29/evidence162/stage-delta.json)
is unchanged; extra personal passes do not compensate for a course regression.

The [compact plan](plan.md) retains five broad groups: assembly 6; extended
syntax/types/layout 37; template demand/hosted ABI 19; structured intrinsic
contexts 2; and one legacy trait contract question. Code alignment, dependent
offsetof ABI signatures and class-convertible indices remain with their owners;
extended float suffix parsing still lacks extended precision. The undefined
std-trait and reserved-name false-primary invocable fixtures remain unresolved
contract questions. Ordinary-name reducers or compiler agreement alone do not
prove their reference outputs wrong; references and failures remain intact.

Handoff fragmentation was partly avoidable. Traits needed separate structural
cache and source-destruction followups, and repeated records-only handoffs added
review overhead. The atomic handoff changed shared identity/storage paths without
closing reference materialization and reduced-alignment interactions. Future
handoffs should complete one broad owner group through semantic selection,
lowering, independent cross-owner controls and evidence, rather than passing each
small symptom separately. No residual required behavior is waived, and full
through-PA29 success is still required before advancing.

## Audit ledger

| Checkpoint | Reviewed range | Findings / disposition | Evidence / result |
|---|---|---|---|
| 158 | `2734e5c6..1ab3499d` (three handoffs) | Alias/expression/template storage fixed; deleted-copy reference corrected with clause proof; historical performance targets classified under spec §9. | PA29 317/403, no new failures; PA1–28 4538/4538; file audit/controls pass; four performance dimensions retained. |
| 162 | `1ab3499d..cce8634c` (entry `9662716b`, three handoffs) | Fixed atomic bool RMW, reference snapshots, alignment/native fallback and cv/identity conversions; reviewed complete traits/invocation/atomic ownership range; no reference changes. | PA29 338/403, identical 65 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 221/221 explicit controls; native/LowIR/cache checks and 1,088 performance observations retained. |
