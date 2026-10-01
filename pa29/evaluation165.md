# Evaluation-context implementation165

Entry: `61e023f39a4c1bafdccc484257ab51df98f87088`, 349/403.
Owner: shared semantic constant execution and initialized storage, with builtin
registration and typed lowering consumers. No fixture/reference change.

## Contract and semantic ownership

The required `800-builtin-functions-and-offsetof` program uses
`__builtin_is_constant_evaluated` in a constexpr function called at runtime.
[GCC's extension contract](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html)
defines its result by manifest constant evaluation. The result cannot be a
per-function constant. Fixed arity, bool result, global qualification, builtin
probing and nonthrowing behavior use the existing declaration/intrinsic owners.
Taking the intrinsic's address or decaying it to a function pointer is rejected.

The evaluator carries an explicit, scoped evaluation mode. Required constant
expressions and admitted initialization trials use manifest evaluation; ordinary
automatic initializers and existing optional array folding use runtime mode.
The emitted intrinsic is false. Semantic checking of a demanded body restores
its own constant-expression context before executing that body in the caller's
mode. A constexpr/const-integral local with an independently established value
retains that initialization even inside a runtime-folded enclosing activation.

The work expanded beyond the required call to automatic/static scalars,
constexpr arrays and class objects, volatile reads, reference temporaries,
references selecting existing local/global subobjects, inherited fields, unions,
bit-fields, pointer/function-pointer constants, defaults, templates, recursive
memoization and failed constant-initialization trials. Defaulted constructor
arguments now reach constant execution even when the declaration has no explicit
initializer node. Non-constexpr early-static constructor summaries use runtime
mode for their bodies and arguments.

## Data flow and complete keys

| Fact owner | Identity / inputs | Consumers and invalidation |
|---|---|---|
| Builtin registry | Interned name -> canonical function entity -> typed intrinsic | Probes, selected-call facts, exception analysis, evaluator, LowIR. |
| Parsed expression value | Source occurrence, evaluation mode | Manifest values retain the existing fact slot; runtime values have a separate flat index. |
| Query value | Canonical query ID, evaluation mode | Existing reverse dependency edges invalidate both mode entries on required class completion. |
| Constant activation | Body ID, converted typed arguments, receiver, zero-initialization, evaluation mode, reachable-storage snapshots | Recursive/in-progress and success/failure states remain separate. Body checking is still once per body. |
| Static scalar value | Source occurrence, destination type, evaluation mode | Separate flat mode indexes share the typed result arena. |
| Array proof | Initializer-plan ID, evaluation mode | Emission uses the owning declaration's required/runtime mode. |
| Mode dependence | Constant/activation IDs, then initialized entity ID | Cached hits propagate dependence. Only affected stored values require the new materialization path. |
| Reference initialization | Initialized entity -> storage entity, byte offset, whether storage needs initialization | Lowering consumes the selected object and offset without lookup, source reconstruction or reevaluation. |

Trace: `work<N>` in `controls165/evaluation-templates.cpp` retains one parsed
pattern. Its demanded function specialization selects `active` by entity.
Manifest and runtime executions have distinct activation keys. Required values
become typed scalar/class/reference initialization facts; runtime array proof
uses false for the intrinsic. Lowering writes those facts through the existing
LowIR builders, constant-data writer and native/ELF path. Host linking is only
the PA27/28 boundary. No host/reference compiler participates in implementation.

Accepted constexpr storage is materialized only when evaluation mode affected
its proof. Ordinary unaffected class construction stays on its inherited path.
This is required observable initialization, not a new optional optimization.
The existing typed constant-data sharing handles class images. Local reference
plans retain actual semantic storage identities, including selected subobjects;
they do not invent global symbols for automatic objects.

## Complexity, lifetime and bounds

The new registry entry is bounded compiler metadata. Mode selection, dependence
propagation and completed-fact lookup are O(1) average operations on compact
IDs. Mode-dependent caches admit at most two variants of each existing key;
there is no global generation counter or unrelated invalidation. Each affected
reference has one small record. Automatic reference trial evaluation is skipped
entirely in translation units without this intrinsic.

All new indexes and records belong to the Analyzer/TU, with no process-global
mutable cache or individually owned graph nodes. Constant frames keep their
existing activation lifetimes. Lowering emits each required class image through
the existing typed-data path and releases function-local backend work normally.
Work follows demanded expressions, executed steps, explicit initializer actions
and emitted bytes. Existing 1,000,000-step and 512-depth limits remain. No grammar
replay, serialized-phase transport, optional transform or new growth budget was
introduced. Optional work/growth budgets for this change are zero.

Telemetry reads existing mode-use counts and sparse index sizes; it performs no
additional analysis. Performance evidence is PA29/O0 and will be recorded in
`performance165.md`, with compiler latency/RSS and checked runtime/text size.

## Boundary and review

The completed group concerns evaluation mode and initialized storage. The source
location family remains unfinished implementation: caller defaults, nested
defaults and default-member initializers need source-invocation identities in
addition to lexical source coordinates. The retained
`pending165/source-invocation.cpp` reducer distinguishes these cases. Emitting
lexical constants for that family would leave incorrect default behavior;
implementing it needs a separate invocation-fact owner and propagation through
call/default/construction recipes. It cannot reuse the evaluation-mode bit or
immutable function-name string facts as its context key.

Independent review must examine complete mode keys, dependence propagation,
reference lifetime/storage identity, and the performance evidence. This document
records an implementation handoff, not review approval. Earlier review markers
and remaining whole-stage correctness/architecture questions remain in plan.md.
