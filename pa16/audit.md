# PA16 independent full-stage audit — loop 44

Stage base commit: `438d56b164600f4fa19d25dcb5f09a76e2a79776`
Last reviewed commit: `95cdc3d8ec4117d69c53a32bceddcf7de6230abb`

Entry: `241c6870`, clean. Previous checkpoint marker: `7c39a6ed`.
This is a fresh reconstruction from spec.md, the assignment and owning earlier
handouts, stage commits, current source, reducers and measurements. Checkpoint
conclusions were inputs to investigate, not substitutes for reviewing current
behavior. The final [manifest](../student.tests/pa16/final-checkpoint.json) binds
the complete stage range, inherited handoffs and exact reviewed implementation.

## Architecture and final Spec Alignment

**Source to typed facts (spec §§1–4).** `lowering/driver.cpp` constructs immutable
preprocessor source buffers, `PostTokenCursor`, a ring `syntax::Cursor`, the
parser and Analyzer within each TU iteration. Identifiers are interned at entry.
The parser calls semantic declaration construction as regions complete. The
source Node vector and compact `(source, context)` occurrences are one graph;
there is no copied semantic tree or intermediate textual token/AST transport.
Deferred bodies/defaults retain source regions; instantiation does not replay
grammar. Delimiter and angle indexes bound lookahead to the queried region.
The audit repaired named-cast angle classification within that existing index.

`semantic/model.cpp`, template binding and instantiation owners use TypeId,
EntityId, ScopeId, QueryId and argument-pack IDs, with structural interning and
flat indexes. Rendered types/manglings remain output. Parent-linked immutable
substitution frames share non-dependent source/facts; occurrences own substituted
facts. Signature, definition, layout, defaults, exception and body states remain
separate. One specialization context is shared by exception/default/body demand.
Lookup follows indexed scope, using/ADL/base edges. Candidate selection filters
by shape then checks all required conversions, recording the chosen declaration
and conversions for consumers. Routine candidate viability uses status facts;
errors reaching explicit-cast/query validation are terminal semantic diagnostics.

**Scheduling and cache validity (spec §§4–5).** `declaration.cpp::finish` drains
monotonic cursors over deduplicated vtable, storage, specialization and member
queues. Class-boundary completion processes its pending interval; definition
indexes visit the relevant source head. Constructor delegation uses a cycle
state, not repeated global retries. Default/exception/body demand have independent
states and dependencies. Ordinary initializer unavailability does not poison a
later required call after a definition becomes available.

The constexpr body index is keyed by selected function. Activation keys include
body, converted typed arguments, receiver/subobject identity, zero-initialization
mode and the version/liveness/readability of reachable storage. In-progress
activations stop recursive identical demand; success and true failure are cached.
Unavailable prerequisites and resource exhaustion return to NotStarted. Mutation
and frame retirement affect their storage records. Late publication now refreshes
only the previously unreadable declaration storage before reading or forming an
activation key. No process-global cache or whole-program generation clear exists.
Canonical query facts are immutable after successful checking; value queries
consume the same selected conversions, addresses and call execution as ordinary
expressions. Required array-plan proofs cache complete immutable plan identities.

**Lowering and allocation (spec §§6/8).** The Analyzer owns TU vectors/flat indexes
for types, entities, regions, fixed facts, constants, object parts, address paths
and completed queries. Calls own short-lived mutable binding/builder vectors;
address storage has explicit liveness and retirement. Shared evaluated values
retain IDs, not owning pointers or recursive destruction. Constructors normalize
values before publishing subsequent member reads. Static reference materialization
is selected once; lowering reserves all symbols before emitting relocation data.

`Procedural` consumes selected conversions, typed initializer plans, field layouts,
object/lifetime identities and ABI entries directly into `lowir_model::Program`.
There is no fake frontend node, semantic name recovery or serialize/reparse step.
Full validation is enabled by the explicit audit flag, not repeated in ordinary
emission. Function-local lowering scratch ends with its function; TU semantic,
source and lowering owners release each loop. Only typed LowIR plus required
linkage/lifecycle summaries survive until final output. Program lifecycle combines
TU hooks once and preserves ordered actions/reverse destruction. Final temporary
cleanup and result ABI retain potentially throwing suffix cleanup correctly.

**Optimization and native boundary (spec §§7/9/10).** PA16 owns O0 typed LowIR,
constant initialization and required scalar-array copies. It does not introduce
an optional optimizer, fixed-point pass, inlining or speculative code growth.
Own MIR, allocation, encoding/ELF writing, optimized debug and self-hosting are
later milestones. PA16's native tests explicitly pass student-generated LowIR to
the supplied backend; no reference/host tool implements frontend or lowering.
The external LowIR parser is an authorized harness boundary, not production
phase transport. No extra PA16 debug target is mandated by its handout.

## Representative end-to-end trace

`final_audit.py::architecture_trace` declares a Record with an unsigned 3-bit
field and double member, binds a constexpr reference to `Record(9)`, demands
`step<Record>(T const&) noexcept(sizeof(T)>1)`, and uses its result in two
identical local constexpr arrays. A dynamic local Record uses `seed()` once.
The retained temporary exists before construction; bit-field normalization
publishes `n=1`, while the double member is 4.5. Both ordinary evaluation and the
selected template reference conversion observe that same object identity.

Actual telemetry: 224 produced tokens, maximum 29 pending, 346 source nodes,
46 template occurrences, one deferred/demanded region, one substitution frame,
one body transition, two constexpr activations, four completed-call hits and
ten execution steps. Direct lowering emits 100 instructions, one local-static
owner and one readonly array image. LowIR contains two distinct stack slots and
two `copyobj 16x8` operations from that image. The semantic nonthrowing fact
reaches the `step` signature as `unwind=no`. The guard encloses the real `seed()`
call and updates after successful construction. The supplied backend produces
ELF; native checks confirm results 12 and 14 and exactly one seed call. Hashes,
LowIR, ELF and full telemetry are bound in the final evidence.

The useful fact through lowering is a complete nonvolatile scalar initializer
image. Its proof includes conversions, active subobjects, persistent addresses
and selected typed data. Structural interning compares item kind/type/value,
symbol/addend, size and alignment; x87 padding is not semantic identity. Separate
stack objects preserve address inequality and mutation independence. Data dedup
releases the tentative duplicate tail. Unknown/nonconstant values retain ordinary
initialization; volatile stores and class lifetime actions retain their semantics.
PA16 mandates the copy, so its legality is not conditional on a timing threshold.
Runtime and code/data costs are measured rather than inferred from IR-node counts.

## Findings repaired across owners

| Finding and reduced trigger | Complete repair and governing obligation |
| --- | --- |
| `unsigned n:3` initialized with 9 was observed as 9 during constexpr evaluation and packed as an ordinary scalar in evaluated static objects. | Field metadata now normalizes aggregate and constructor publication, including later member initializers; evaluated fields retain field identity and lowering uses the shared bit-field packer. Nested class/array/signed/overlap controls execute. N3485 §9.6 [class.bit] governs target storage width and signed policy. |
| Static references to class/subobject/list/converting temporaries lacked persistent identity before constexpr validation; aggregate scalar reference members lost their backing object. Self-pointers could reach an unreserved LowIR symbol. | Initializer reference ownership selects retained scalar/class identities before evaluation. Construction receives its destination before `this` escapes; nested aggregate reference plans retain their own temporaries. Semantic reads, static data, dynamic initialization and symbol reservation consume those identities. §12.2 [class.temporary]/5 extends the required lifetimes; function-parameter and constructor-member exceptions remain rejected. |
| Taking `&extern const n` before its definition cached permanently unreadable storage. | Refresh that storage after publication, increment its version, and include refreshed state in dependent activation keys. Early ordinary initialization remains dynamic when required. §3.6.2 [basic.start.init] and §5.19 [expr.const] distinguish current availability from later constant reads. |
| Template value queries rejected ordinary supported pointer/reference calls, defaults, string/array access, operators, functors, arrow chains and explicit casts. | A dedicated query adapter consumes shared call/conversion/address engines, inherited receiver projection and selected arrow/surrogate facts. Literal queries retain their actual source identity. Explicit builtin conversions are shared with ordinary casts. Parser angle lookahead recognizes named casts; nested static/const casts are covered. No source reparse or fake AST repair. |
| Reinterpretation could pass required constexpr pointer/array validation because a relocatable static image was mistaken for a core constant expression. | Selected conversions carry a packed constant-forbidden flag; required array plans validate semantic values, with immediate proof for literal/value-init actions. §5.19 excludes reinterpret_cast and void-pointer downcasts; §7.1.5 includes initializer conversions. Ordinary proven early-static relocation remains permitted by §3.6.2/3, preserving PA10 behavior and comparison. Query checking now admits well-formed reinterpretation in an unselected conditional branch; §5.19/2 expressly excludes unevaluated conditional/logical branches from the constant-expression prohibition. A selected cast still fails constant evaluation. |
| Shared object DAGs were revisited exponentially while collecting dependency/persistence facts. | Per-query object-ID dedup joins existing storage-ID dedup; persistence uses an explicit work vector and address-containing parts. Counters measure visited values. Work is proportional to distinct reachable values/edges per activation, without weakening keys or lifetime checks. Both frozen implementations produce identical output. |

The first repaired campaign exposed eager constexpr-array value construction.
That avoidable cost was removed: immutable literal/value-init plans prove validity
without materializing a duplicate array; other plans use the complete evaluator.
Conversion flags were packed instead of growing every hot conversion record.
The first campaign and all outliers remain preserved; it is not used as final
acceptance for the corrected code.

## Work budgets and performance acceptance

The existing 512-call / 1,000,000-execution-step limits remain implementation
resource limits, not invented PA16 numerical mandates. Incomplete/budget-limited
probes retain conservative dynamic lowering where permitted; required constant
contexts fail. Sparse initializer plans retain omitted ranges, index explicit
array ranges and emit work proportional to required bytes/actions. The inherited
eight-element expansion cap retains a loop fallback. Object dependency/persistence
walks are bounded by distinct reachable identities/edges per query. The earlier
32-byte copy cutoff cannot override PA16's mandated one-copy form.
No optional transform adds instructions or multiplies pipeline work. Shared-data emission is
bounded by distinct images plus required destination copies.

[Final performance evidence](final-audit-performance.md) reports compiler wall
latency/peak RSS, executable runtime and code/data sizes together, with frozen
binaries/flags/input hashes, A/A calibration, ABBA pairs and every observation.
It includes template, memory/floating, array, result-lifetime, new-behavior and
shared-object workloads, plus long runtime loops with volatile inputs and checked
results. The startup-amplified multi-TU control tests the DAG improvement and TU
release boundary. No speedup is claimed for new behavior rejected by entry or for
byte-identical native programs. Small/noisy movements remain disclosed.

The inherited +15% latency, +16 MiB RSS and 5.5x scaling diagnostics, plus
earlier PA14/15 text targets, are not exit gates: neither the PA16 handout nor O0
specifies those limits,
and spec.md expressly requires stage-scoped acceptance. This reclassification
preserves raw measurements, historical misses, correctness, coverage, timeout and
resource limits. The avoidable array regression was resolved; necessary semantic
checks and modest compiler text cost are disclosed. Later-stage native optimization,
allocator/debug and self-hosting obligations have not been waived or fabricated
as PA16 requirements.

## References, validation and accumulated ledger

No source fixture, expected result, status sidecar, comparator, bundle or reference
binary changed during this audit. The inherited 24 initializer revisions and one
result-ABI revision were reviewed independently: six ordering reducers plus local
reference ordering establish static initialization; the automatic-array reducer
and PA16's explicit one-copy rule establish the 16 older representation repairs;
the nontrivial empty destructor establishes indirect result ABI and cleanup.
[Initializer proof](reference-corrections.md) and [result proof](result-reference-correction.md)
retain N3485/LowIR/ABI citations, old/new hashes and bundle revision. Scalar stores
are not asserted to violate the C++ abstract machine; their correction is mandated
by the cumulative PA16 output contract. Compiler agreement alone is not the proof.

Fresh final-source validation passes fileAudit (three inherited substantial-header
warnings), PA16 **154/154**, root through PA16 **2266/2266**, all 16 stages, and
all 11 explicit suites (**289 native / 93 rejection**, including 53 new controls).
The root also executes its focused property checks. Earlier failed audit trials
are retained and do not replace the passing final checks. The evidence verifier
binds current implementation, binary, complete control results/logs, fixed-course
coverage and every performance artifact. Storage cleanup removed only disposable
build objects, with a retained deletion manifest; evidence was preserved.

| Accumulated handoff | Independent disposition |
| --- | --- |
| Stage base through `7c39a6ed`, record `caac2cda` | Re-reviewed scalar/floating/storage/validity/noexcept owners and their interaction with current object and query machinery. Prior precision policy, cache unavailability and specialization-context repairs retained; all inherited controls rerun. |
| `5048b92b`, `a707845c`, `135242c1`, `f1497ca2`; record `fe4dbd88` | Object/address/frame execution, canonical queries, ABI/storage/literal demand reviewed. Current audit closes bit-field, retained-reference, late-definition and shared-value work defects across semantic and lowering owners. |
| `9e967c33`, `fc309df7`; record `c9c4ddb7` | Initializer storage, readonly copies and single program lifecycle reviewed. Standard/contract proofs rechecked; both TU orders, dynamic fallbacks, identity and cleanup controls pass. |
| `ee190270`, `048014e8`; record `241c6870` | Result ABI/final-expression cleanup and represented member-pointer constants reviewed; potentially throwing cleanup, persistent reference storage and demanded relocations covered. |
| `08476ddf`, `95cdc3d8` final repairs | Combined whole-stage source reconstructed and checked; final native/rejection, course, file and frozen performance evidence pass. No PA16 handoff remains unaudited. |

Cross-owner member-pointer conversions and a virtual member-pointer ABI remain
inherited source-model extensions: PA12 explicitly excludes member pointers;
PA13/14 add no mandate, and PA15 explicitly defers non-integral template arguments.
PA16 owns constant evaluation over represented inherited semantics. Same-owner
member values and applications (including compatible inherited objects) are
implemented and exercised, not waived. PA17/18 extensions remain the explicit
next-stage boundary. Final disposition: PA16 full-stage accepted on its current
behavior, architecture, bounded work and measured stage-scoped performance.
