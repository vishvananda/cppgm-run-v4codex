# PA29 implementation160: intrinsic invocation

Entry `8bdc6bbf` passed 321/403. Code `2fd6963e` passes 323/403 with
no new failures. This completes the member-invocation behavior group; it does
not certify PA29 or resolve independent whole-stage audit questions.

## Ownership and data flow

The probe and semantic recognizer share `invoke_builtin_name`. Ordinary callable
objects/functions keep their existing call owner. Member pointers use the
canonical call query, with a snapshot of ordinary `operator*` lookup and the
source access environment in its key. The query selects the member receiver:

- Same/derived class objects use the object directly, before considering `*`.
- Other operands use a checked unary `*` operation, including raw pointers,
  pointer conversions, member operators, nonmember operators and ADL.
- The existing member-pointer owner checks cv/ref categories, access and base
  ambiguity, and records the base adjustment. Member data retains its lvalue or
  xvalue category. Member functions reuse indirect-call argument conversions.

The selected unary operation is a typed `RangeOperation`, the existing common
representation for implicit language operations. It owns selected declarations,
conversions, return type, virtual slot and temporary facts. No syntax node is
manufactured. A TU-owned vector holds these records; `ObjectUse` carries their
compact indices. Query recipes remain immutable. Evaluated uses copy only the
materialization record and establish their own temporary/body demands.

Fixed template invocation calls retain source-owned recipes and project their
operand identities into each use. Dependent calls use the canonical substituted
query. Query construction now enters an expanded operand's immutable lane
environment, as evaluated expression construction already did. Previously, a
heterogeneous named function parameter pack could bind every query operand to
the same parameter; the pointer-like invocation fixture exposed this shared bug.
No source region is reparsed and no whole-program retry is added.

Native lowering consumes the recorded unary recipe and member-pointer base
adjustment. Existing typed calls, loads and indices feed LowIR, per-function MIR
and direct ELF encoding. The serialized LowIR roundtrip reproduces execution.
Constant evaluation consumes the same receiver and conversion facts, through
both source expressions and canonical queries. Cleanup and exception analysis
include the implicit dereference, its conversions, and any returned temporary.
Member-data access itself has no call/unwind effect.

## Work, lifetime and invalidation

Query identity includes canonical operands, source access context and the
ordinary operator candidate family. Existing query dependency edges retain
incomplete prerequisites; class completion does not publish an unsupported
negative answer. An overload insertion changes the captured family identity.
Completed query results are reused. Source occurrences still incur source-name
binding: the repeated `declval` spelling control visits two explicit template
names per occurrence. That is distinct from recomputing invocation queries.

Selection visits only required candidates, arguments and inheritance edges.
The query/overload work is O(candidates × arguments), plus required cached base
paths. Runtime preparation and lowering are O(arguments + emitted operations).
Each dereference introduces one bounded unary operation; no fixed point, global
scan or optional optimization is added. TU facts release with the analyzer;
argument vectors and per-function lowering state have their existing shorter
lifetimes. The added index increases `ObjectUse` storage by a bounded amount.

## Validation and boundary

Explicit controls cover direct/base/raw/pointer-like receivers, virtual and
virtual-base dispatch, adjusted member pointers, cv/ref result categories,
SFINAE and rejection, ADL, implicit conversions and nontrivial argument copies,
reference/void returns, single evaluation, fixed/dependent template calls,
heterogeneous packs, constexpr evaluation, temporary cleanup, and unwinding.
An unevaluated pointer-like invocation does not instantiate a dependent invalid
operator body. Existing free/function-object invocation controls remain covered
by the unchanged course suite. Inspection checks query sharing, selected native
calls, LowIR roundtrip execution, and telemetry/object byte equality.

The remaining source-location and constant-evaluation/fence intrinsics require
new source-context, evaluation-mode and memory-order facts. Atomic/assembly,
extended type/syntax/layout, and template/hosted ABI groups retain separate
owners. Extending the completed receiver/conversion recipe cannot supply those
facts; this is the concrete handoff boundary. All remaining failures stay in the
ledger as implementation work or explicitly retained contract questions.
Independent audit must still review the new recipe lifetime and environment
keys, as well as retained whole-stage findings. Nothing here waives that audit.
