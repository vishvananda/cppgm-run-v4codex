# PA20 accumulated checkpoint audit — loop 97

Stage base commit: `a9b24ab68f1a75288df10161cb171fa239e1409a`.
Last reviewed commit: `882cf5236a8ddb756403105cd50135440920e2a4`.

**Checkpoint audit complete for PA20/O0; full stage remains incomplete.**
Entry was clean at `7eca0738b6e949ea32da7c8959b8d41c28fba7c2`, with
121/144 course tests passing and 23 distinct failures. No build/test process
remained live. The previous accepted handoff made verified progress (110 to
121 passing fixtures); this audit independently inspected current files and
re-executed checks rather than accepting that handoff as review evidence.

## Complete range and contract

The initial review marker equals the recovered PA19 stage boundary. Review
covers all 14 accumulated commits through entry, their combined changes to
42 implementation/registration files, and the cohesive audit fix. No latest-
handoff-only boundary was used. `spec.md`, PA20's README, PA8's LowIR contract,
`TESTING_AND_REFERENCES.md`, the project map and inherited plans/evidence
define the applicable requirements. The target is source-to-LowIR at O0;
course-provided native encoding is an external validation boundary at PA20.

| Commit(s) | Reviewed changes and interactions |
|---|---|
| `e36f3662`, `ef897177`, `e59e46fa`, `7eca0738` | Stage boundary and all three handoff records, raw performance/validation harnesses and reference correction proof; no review was inferred from these records alone |
| `dbd1a8f6` | Canonical placeholder result types; ordinary/lambda return deduction owned by function body states; demand from calls, operators and address selection; constexpr result obligation completes after deduction |
| `df239d8e` | Complete array expression/query types and bounds; aggregate construction fallback; selected reference conversions for builtin increment/decrement; conversion-type-id lookup in the using scope; parser type/expression prediction |
| `0dd795a1` | Direct canonical binding for an unstructured by-value placeholder avoids scratch substitution maps; cv/reference/declarator structure retains general deduction |
| `2cbd6b8d` | Typed range endpoint/operator/conversion records, once-evaluated storage, direct LowIR loops, lexical cleanup and jump barriers |
| `14fff139` | Shared nondependent range recipes versus per-occurrence storage/demand; definition-time rejection and conversion-created reference lifetime extension |
| `58c89851`, `fe0c1722` | Selected promoted arithmetic types for implicit increment; conversion-function class prvalues construct directly in the destination, preserving self-pointers and destruction |
| `beb83901` | Scalar array member transport, omitted scalar parameter shapes and ordered interleaved class transfers; exposed the broader delayed-store defect fixed below |
| `6fbfd99f` | Lexical lambda defaults, destination construction, selected receiver-free ABI entries, one checked body shared by demanded entries, shared statics and function-owned label/range storage |
| `5d2e6a65` | TU-owned canonical array-type eligibility facts, including nested tails and negative results, with work/hit telemetry |
| `882cf523` | Semantic helper safety proof and ordered fallback; full earlier-stage compatibility, cross-handoff controls, frozen performance and architecture evidence |

PA20 fixtures also explicitly exercise parenthesized aggregate construction,
lambda defaults and broader return deduction. Their required course behavior
is retained; these are not asserted to be unrestricted ISO C++11 features.
The 23 unimplemented course cases remain visible and mandatory in the plan.

## Finding and ownership repair

**Correctness defect: evaluating helper arguments delayed earlier destination
stores past later initializer clauses.** For example:

```cpp
struct S { int a[2]; int b; };
int main() {
  S s[] = {{{3, 4}, s[0].a[0]}};
  return s[0].b != 3;
}
```

The entry compiler populated a separate array argument, loaded the still
uninitialized destination member, then called the helper. The same defect
affected scalar members, nested array elements and template occurrences.
[The 18 audit controls](../student.tests/pa20/audit97_controls.py) include seven
entry failures; [entry results](../student.tests/pa20/audit97-entry-controls.json)
and final validation preserve that comparison. N3485 §8.5.4 [dcl.init.list]/4
sequences every value computation and side effect of a clause before the next
clause; §8.5.1 [dcl.init.aggr]/2 assigns the clauses to successive subobjects.
See [the local standard](../doc/n3485.txt), particularly line 11637. This proof
is about the initialization itself, not compiler agreement.

`semantic/initializer_effects.cpp` owns a conservative summary on already
checked expression occurrences and completed constructor identities. It accepts
independent scalar operands and checked empty constructors whose scalar member
initializers/arguments are independent. Opaque calls, aliases, member/indirect
reads, volatile effects and unavailable facts keep ordered initialization.
The TU's flat index uses disjoint NodeId and tagged EntityId keys; constructor
bodies/actions must already be complete. It triggers no new body demand,
lookup, overload selection, grammar pass or constant evaluation. Facts are
stable across later declaration insertion because they describe selected,
checked occurrences, not open lookup sets. Runtime value changes do not change
the structural independence proof.

Initializer/list/value owners propagate the proof through `InitAction` records.
Lowering reads that flag before choosing a helper; it does not rediscover
semantics. A single scalar/reference argument has no earlier destination store
to observe. A source temporary built by the proved constructor can precede
the helper; its selected move runs inside the helper after preceding fields.
Arrays still require proof for their own elements. Other cases use existing
ordered destination construction, retaining storage identity and lifetime.

An initial overly conservative guard changed three valid PA12/PA19 ABI shapes;
[that rejected validation](../student.tests/pa20/audit97-validation-initial.json)
is retained. The single-argument and completed-constructor proofs resolve those
regressions without altering references. Final controls, earlier suites
and exact unchanged failure identities validate the repair. New implementation
source registration is in `dev/frontend_source_sets.mk`.

## Architecture trace and budgets

| Spec requirement | Current source trace / conclusion |
|---|---|
| §§1–2 source, parsing, identity | Immutable source buffers feed preprocessing/post-token cursors and the parser's lookahead ring. `Parser::translation_unit` cooperates with `Analyzer::consume` on one source graph. `Ast` source/context occurrences project retained template regions without calling the parser; canonical TypeId/EntityId/QueryId and flat scope indexes own equality/lookup. No changed path transports phase data as text or uses rendered names as a semantic key. |
| §§3–5 resolution and demand | Calls retain selected declarations and conversions. Placeholder return demands use active/success/failure body states; repeated calls do not recheck bodies. Fixed ranges keep source-definition recipes; dependent ranges get their own substituted occurrence facts. Class completion, selected bodies, defaults and emission remain separate demands. Targeted demand cursors/dependency indexes are unchanged; no global retry/invalidation was added. |
| §§4–6 closures and lowering | A closure is owned by its source occurrence and enclosing specialization. Lexical defaults retain declaration lookup. Object/pointer entries consume one checked body, parameter identities and shared static entities, with fresh LowIR function slots/labels. `ObjectUse::callable_entry` preserves the selected language declaration separately from its ABI entry. Typed `RangePlan` operations and conversions feed direct `FunctionBuilder` construction, including destination and cleanup facts. |
| §§6–8 allocation and emission | New facts use TU vectors/flat indexes, with no hot owning per-node pointers or global mutable cache. Initializer proof visits each requested expression/constructor key once; array representation classification visits each canonical array tail once. Scratch argument vectors and function builders reset at their normal owners. Source/parser/semantic/lowering state releases at TU exit; the typed LowIR program/linkage graph remains until the required LowIR output is written. There is no retained textual IR copy or production reparse. |
| §§7,9 legality and work | Helper delay now requires the proof above. Unknown cases conservatively retain ordered IR. Required array representation copying excludes volatile/class elements. Existing expansion budget 8 remains; large omitted tails use bounded loop/zero paths. Each helper's work is O(fields), each range has at most five implicit operations and two iterators (or one array index), each closure at most two demanded body emissions plus its small conversion entry. Pipeline work follows source/typed edges/emitted IR; no fixed point, unrolling or unbounded growth transform is introduced. |
| §§7,9 profitability | Removing the captureless pointer wrapper has repeated historical runtime benefit and smaller native payload. Both callable entries preserve ABI, defaults, statics and cleanup. Immediate-call presentation and aggregate array transport are required O0 shapes, not optional speedup claims. The audit safety fallback is a correctness repair: its failing entry executable is excluded from A/B timing. Measurements, spreads and cost classification are in `performance97.md`. |
| §10 compiler ownership | Current `execve/openat` inspections show one compiler process and no reference/cached-answer reads. The compiler constructs its LowIR. Only the independent harness invokes `lowir2native-ref`, as the handout requires. Student MIR/allocation/ELF writing, native debug and self-hosting are later-stage obligations. |

[Final validation](../student.tests/pa20/audit97-validation.json) reruns traces
94–96: deduction and array packs, fixed/dependent ranges with class lifetimes,
and aggregate helpers plus both callable entries/shared statics. Each executes
successfully and emits byte-identical LowIR with stats/audit on and off.
[Current inspection](../student.tests/pa20/audit97-inspection.json), produced by
[the inspection harness](../student.tests/pa20/audit97_inspect.py), additionally
retains exact LowIR, native disassembly, hashes and syscalls for safe and
ordered helper paths. Both demand two template specializations, with repeated
calls reusing completed bodies. The safe trace emits one shared helper;
the ordered trace stores each member before a later member load, has no helper,
and returns zero. Supplied-backend payloads are 1360 and 375 bytes.
Native traces expose normal O0 frames, loads and calls; this audit claims
no improvement to the supplied encoder's spill/allocation policy.

## Reference preservation and checks

The only fixture changes since the stage base are the eleven `.ref` revisions
documented in [reference-corrections95.md](reference-corrections95.md). Bundle
source revision is `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`; the document records
its bundle hash and local revision `pa20-array-images-95`. The PA16 automatic
scalar-array contract requires readonly data plus a copy into distinct storage.
N3485 aggregate/string/value/object-identity rules and the LowIR `copyobj`
contract establish preservation. Reconstruction reads original oracles,
not compiler answers; it verifies contiguous typed spans, extents, values and
absence of later uses of removed addresses. This audit reconstructed all eleven
exact revisions, ran the reduced source, and executed old/new/student outputs.
No new reference correction was needed.

`audit97_verify.py` checks the complete changed-fixture set against that manifest
and compares all 639 entry files covering PA20 fixtures, statuses, handout,
wrappers, shared harness and comparison rules. All are unchanged since audit
entry. Earlier assignment tests were not edited. Required results:

- `make test-pa20`: **121/144**, exit 2, the same 23 failure identities.
- `n=20; if [ "$n" -le 1 ]; then echo '===== ALL TESTS PASSED SUCCESSFULLY! (0/0) ====='; else make test-report-through-pa$((n - 1)); fi`: **3452/3452**, exit 0.
- `perl scripts/cppgm_file_audit.pl --stage pa20 --paths dev/src`: **pass**, exit 0; three inherited header warnings.
- `make test-report-through-pa20`: **3573/3596**, exit 2; stage is not advanced.
- Personal controls: **206/206** (188 inherited plus 18 audit controls); all traces/inspection pass.
- Stage progress: **pass**; no new failures and no reduced coverage. Additional control passes are not used to compensate for any course failure.

## Remaining work and handoff discipline

The [compact plan](plan.md) retains four mandatory implementation groups:
capture environments/composition (17 failures), aggregate member construction
ABI (2), constructor conversions (2), retained declarations/lifecycle (2).
They are not waived or pushed to PA21. The whole accumulated implementation
and its completed owners have now been reviewed; the stage still needs those
features and the full through-PA20 gate before advancement.

Three handoffs were justified by distinct owners, but ten code commits included
avoidable fragmentation: promoted range arithmetic, direct conversion-result
construction and array-classification follow-ups should have been included in
their owner's composition/performance validation. Future handoffs should finish
the broad groups above, retaining cumulative checks across their interactions.

## Ledger

| Audit | Reviewed range / code tip | Finding and evidence | Required gates / stage state | Remaining work |
|---|---|---|---|---|
| 97 | `a9b24ab6..882cf523` (entry `7eca0738`, all 3 handoffs) | Fixed delayed aggregate stores with semantic safety facts; 206 controls, 5 current traces/inspections, independently reproduced reference proof; 1068 performance observations / 114 warmups | Prior **3452/3452**, file audit **pass**, progress **pass**; PA20 **121/144**, unchanged 23 failures and 144 fixtures | Capture environments, aggregate ABI, constructor conversions, retained declarations/lifecycle; stage incomplete |

The code/evidence commit above preceded this plan/audit/performance records-only
commit. `Last reviewed commit` names that exact code tip; no implementation
changes are included in the records commit.
