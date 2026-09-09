# PA8 independent final full-stage audit

Reviewed `spec.md`, PA8 README/format/grammar, testing/layout instructions, every
PA8 implementation owner and stage commit through `1d69e0d35`. The prior
checkpoint record in `7cb3f718e` was an input to inspect, not completion proof.
This audit includes all handoffs since the PA7 final review `2f8c201f5`.

## Final Spec Alignment

| Spec surface | Independently reconstructed architecture |
| --- | --- |
| 1: source and parsing | `dev/lowir.cpp` owns each immutable input string until `Reader::read` returns. The reader has one borrowed lexical view, no token vector or retained instruction strings. Names go directly to the unit interner. The inherited source pipeline remains `Preprocessor` -> `PostTokenCursor` -> syntax cursor -> parser/`Analyzer::consume` on one NodeId graph. |
| 2: identity and facts | Eight-byte inline Type identities encode kind/layout. Distinct 32-bit SymbolId, FunctionId, SignatureId, ValueId, SlotId and BlockId address flat pools; name indexes use interned numeric IDs. A signature owns its parameter slice and complete boundary metadata, including explicit indirect calls. Names, rendered types and serialized records never key semantic decisions. |
| 3: lookup | The reader's unit symbol index resolves cross-file references, and a builder's three local indexes resolve values/slots/blocks. The inherited semantic path indexes lexical scope/name/function shape; candidate-local conversion sequences select the declaration once. No new semantic lookup engine or global search exists in PA8. |
| 4–5: demand and validity | PA8 resolves forward references after all inputs, then validates once; there is no fixed point or retry scheduler. CFG predecessor and handler facts are local to a validation run and rebuilt after edits. The inherited declaration-only template path keys specialization by pattern EntityId + canonical argument pack, records active/success/failure and separate emission demand, and substitutes dependent type structure with local memoization. |
| 6: lowering interface | Exercises construct instructions, typed references and slices directly through FunctionBuilder. The reader/writer are explicit tool adapters over that same model; `serialize_lowir_program` is an output convenience, not a production phase bridge. Alias/extent/passing/effects/unwind/query/storage/TLS/debug/exception/atomic facts are represented without rediscovering source semantics. |
| 7: optimization/native | No optimizer, MIR, allocator or encoder is introduced. Roundtrip order and unused declarations/instructions survive. Only the independent harness invokes the supplied native backend, as PA8 requires. Mandatory structural validation has bounded work; no optional optimization or code growth is hidden in it. |
| 8: ownership | Program owns the interner and 14 geometric pools. Instruction children are slices with no owning pointers or per-node heap children. FunctionBuilder scratch indexes die per function; source strings die per file; validator vectors die per validation; the requested roundtrip model dies at invocation end. Pool IDs survive growth. Builder slice contiguity is now enforced. |
| 9: evidence/work | Phase times, RSS, source/token/name counts, pool growth/capacity, instruction/operand counts and ordinary edges observe existing work. The instruction-validation counter excludes the additional bounded handler scan described below. Frozen compiler/executable and inherited template benchmarks are recorded in `performance.md`. |
| 10: self-containment | Source inspection and the file audit cover all shared implementation files. Required output is built by this compiler. No reference invocation, host compilation, answer cache, fixture recognition or serialized phase transport occurs in the PA8 implementation. The host compiler builds our tools/test APIs; the supplied native backend belongs to the external PA8 harness. |

## Representative end-to-end traces

1. **Cross-file declaration and call.** Reader interns `@later` at a direct call
   and stores a SymbolId operand. The following input's definition binds that
   same symbol to a FunctionId and SignatureId. Validator follows these IDs to
   check return/parameter types and arity; Writer emits the definition exactly
   once, with the original call operands and metadata. No synthetic declaration
   is created at the file boundary. The explicit personal `forward-cross-file`
   and `globals-cross-file` probes cover this, including an address and alias.
2. **Loop-carried values and handlers.** `100-phi-control-flow` interns forward
   block/value references while retaining source block order in `block_order`.
   External validation gathers ordinary edges, sorts/deduplicates them once,
   indexes each predecessor slice and uses stamps for duplicate phi inputs.
   Parallel loop phi values are legal; ordinary uses require a prior definition.
   Handler membership comes from typed `eh_try`/targeted `eh_cleanup` references
   over the complete body, independent of ordinary edges and source order.
   Phi values require the documented directly representable scalar subset and
   are rejected in handlers. Output uses typed operands, never saved text.
3. **Object, literal and boundary fidelity.** Object layout stays in Type's
   byte/alignment identity; bulk spans and parameter extents retain their own
   typed fields. Scalar slots retain their value type rather than implicitly
   decaying at calls. Parameter passing/noalias/extent and function effects,
   unwind, query and return facts survive validation/writing conservatively.
   Numeric operands retain wide integer sign, long-double payload, signed zero,
   infinity and a separate signalling-NaN bit. Debug file IDs and line/column
   fields are written from the model. Existing fixed-point and native probes
   cover these paths; no optimizer consumes or strengthens those permissions.
4. **Direct construction through executable behavior.** `construct_exercise`
   creates one helper symbol/signature, parameters and one block. For sum,
   `n*(n+1)/2` is valid over the handout's 0..1,000,000 domain (the product is at
   most 1,000,001,000,000, within i64). This is a chosen source algorithm, not a
   general loop rewrite. Swap loads both values before either store so identical
   pointers work. Call-twice reuses one explicit indirect signature and retains
   both ordered calls. The shared writer emits helper-only LowIR; our adapter
   combines it with an independently generated caller; the supplied `-O0`
   backend emits ELF; checked dynamic execution proves the helper behavior.
   All permitted sum inputs, aliased swaps and callback effects are tested.
5. **Inherited declaration and demanded template.** For
   `consume(static_cast<void(*)(int)>(&target<int>))`, the preprocessor streams
   interned tokens, the parser retains one pattern/source graph, and each parsed
   declaration immediately calls `Analyzer::consume`. Function families and
   normalized signatures use canonical types. `explicit_template`/deduction
   reuse argument-pack and specialization IDs; `substitute_type` returns closed
   type nodes unchanged, caching dependent replacements locally. Overload/target
   conversion stores the chosen entity and conversions on the source node;
   `demand_specialization` appends each emission identity once; semantic output
   consumes those facts. The fixed template benchmark asserts two specializations
   and two argument packs per TU despite thousands of uses. This trace ends at semantic
   output: C++-to-LowIR starts at PA10, template body lowering later, owned native
   encoding at PA24 and self-hosting at PA34. None is a missing PA8 handoff.

## Findings and changes

- **Phi handler legality:** the checkpoint excluded EH edges from predecessors
  but failed to prohibit a phi if a handler also had an ordinary predecessor.
  Baseline A accepted the reduced case; both the format rule and reference
  observation reject it. `Validator::run` now derives handler membership before
  checking any phi, including cleanup targets and later registrations. API
  mutation/revalidation tests verify both adding and removing a handler target.
- **Phi type legality:** the scalar predicate also admitted f80, outside the
  documented phi subset. The shared local shape check rejects it, covering both
  reader construction and external validation of edited models.
- **Builder ownership:** interleaved builders could append into another
  function's contiguous instruction/block/slot slice, and `start_block` checked
  the global last instruction instead of its own previous terminator. The model
  now rejects broken contiguity in O(1) and checks the owned previous block.
  Direct API probes verify rejection before appending an unrelated record.
- **Evidence provenance:** old temporary benchmark artifacts were absent;
  their JSON remains historical evidence, not a fresh verification claim.
  The new runner records the actual baseline revision, verifies observation
  order/status and hashes/text sizes, and freezes a handler corpus before timing.
  The old B hash exactly matches this audit's A. No observations are discarded.
- **Stale incoming count/review markers:** the supplied primary log and fresh
  cumulative command report 793/793, not the incoming 811/811. The root report
  selects all eight unchanged stage suites; no tests were removed to obtain it.
  Entry-only review markers are replaced by this independent full-stage record.

## Pipeline budgets and invalidation

Read/build/write are O(bytes + IR), with expected constant-time interning.
External validation is O(IR + E log E), O(IR + E) scratch: one cheap instruction
scan for handler membership, one full instruction/type/metadata traversal,
one ordinary-edge sort/deduplication, and predecessor stamps. No whole-array
clear per block, repeated fixed point, whole-program retry or duplicate semantic
validation occurs. `validated_instructions` counts the full traversal; the
handler scan adds exactly one visit per instruction and a bit per block.
Signatures are checked once at function boundaries; each call inspects its
own arguments and explicit signature. That work is bounded by represented
parameters/operands, not unrelated functions. Builder ownership checks add
constant work per append; writing/validation add zero IR or executable growth.

There are no optimization levels/transforms to justify by runtime gains here.
Sum's domain proof is local to the exercise and is not published as a general
range promise. Unclassified effects, aliases and unwinding remain conservative;
no invalidation permits removal or reordering of calls/memory/exception/debug
records. Future optional transforms must keep valid input on budget/proof
failure; structural errors at this external boundary must be rejected. The
numeric performance budgets were frozen before measurement in
`student.tests/pa8/final-audit-protocol.md`.

## Validation and handoff ledger

- `69f031998`: initial plan; `ef37a2c9e`: model, adapters, validation, exercises;
  `0152284b7`: shape/literal fidelity/telemetry; `7cb3f718e`: checkpoint evidence.
  All four handoffs have now been inspected against source and current behavior.
- `f3f9c4176`: phi and builder ownership fixes, four additional semantic
  rejection cases and direct API/edit checks. The incoming binary accepts the
  new handler/f80 errors; the audited binary rejects them.
- `1d69e0d35`: frozen final measurement protocol and stronger provenance checks.
- Fresh `make test-pa8`: 109/109. Fresh `make test-report-through-pa8`: 793/793,
  with all eight stages passing (684 earlier cases +109 PA8).
- Fresh file audit: 97 files, pass without warnings. Personal checks: 24 valid
  +56 invalid cases, writer fixed points and CLI failures; native full-domain /
  alias / callback / floating checks pass. Final ASan/UBSan with leak detection
  passes for the standalone tool and the direct API. The 63 inherited PA7 audit
  cases and its selected-fact/demand API also pass freshly.
- Final controlled performance results and final clean/committed exit state are
  recorded in the plan and performance report. No course fixture, reference,
  harness, coverage selector or earlier compiler implementation was changed.
- Remaining PA8 groups and unaudited PA8 handoffs: **none**. No advance to PA9.
