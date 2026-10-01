# PA29 function-context string ownership

Implementation164 starts at `e5690337`, 345/403 passing. The preserved stage base
and review markers remain in [plan](plan.md). This document covers the completed
function-name group; source-location and evaluation-mode intrinsics remain
unfinished implementation.

## Semantic ownership and data flow

The parser retains one `FunctionName` node for `__func__`, `__FUNCTION__` or
`__PRETTY_FUNCTION__`. These reserved expression forms cannot enter the parser's
fallback type-name prediction (including in `sizeof`). No grammar is replayed.

A canonical `QueryKind::FunctionName` stores its lexical function scope and
interned predefined-name identity. The scope is part of the query key. A pattern
scope makes the query dependent: substitution projects that scope using the
existing specialization frame, then demands the concrete function's string.
There is no implicit pattern variable to recover through declaration lookup.
The concrete query publishes the same entity, const-char array type, bound and
lvalue category used by ordinary expression analysis and constant-address reads.

`semantic/function_context.cpp` owns demand and presentation. Each concrete
function scope/name pair binds one immutable static array entity. Pretty text
uses canonical types, selected specialization arguments, enclosing class owners,
member facts and indexed inline-namespace edges. Elaborated-type syntax and
aliases do not become semantic identity. Rendering supports function and member
pointer types, references/arrays, nested owners, cv/ref qualifiers, packs, partial
specializations, boolean/integer/address arguments, constructors/destructors,
conversion functions and placeholder returns. Text is an output payload only.

The existing predefined-string storage and constant-address interpreter share
that entity. Lowering consumes its array type and bytes and emits typed LowIR
static storage; LowIR/MIR and direct ELF emission retain the ordinary path. No
host compiler, assembler, reference tool, library-name shortcut, fake syntax node
or textual IR transport implements the result.

## Work and ownership bounds

A string is demanded at most once per function scope/name. Repeated queries use
existing canonical query caches; repeated uses use indexed scope lookup. Work is
linear in visited type/argument components, lexical owner depth and emitted text.
Prefix/suffix type rendering streams into one temporary buffer without repeated
concatenation of growing declarators. Signature normalization uses the existing
canonical type cache. Source and final text reside in the translation-unit's
identifier arena; renderer scratch and owner lists die after that demand.

`semantic_function_string_objects` reads the existing flat index's entry count
in O(1). Existing query, specialization, memory and IR counters observe the rest
of the work. A small member-query helper and virtual-telemetry helper preserve
existing behavior while keeping the owning functions within the file-audit
240-line bound. No optional optimization, compiler fixed point, global scan,
cache invalidation or generated-code growth policy is added. Optional work and
growth budgets are zero; required string data scales with demanded declarations.

## Validation scope

Four required failures share this owner: inline-namespace type spelling, inline
elaborated type spelling, enclosing-template argument rendering and constexpr
array-bound queries involving pretty-function strings. Personal controls expand
this to multiple specializations, repeated entity identity, array-reference
binding, constant string indexing, nested scopes and addresses. Explicit runs
also cover LowIR validation/roundtrip, native execution, MIR transport and
telemetry-on/off object equality. Reference inputs and comparison rules remain
unchanged. Final counts and performance observations are recorded at handoff.

## Concrete boundary and unfinished adjacent work

`__builtin_FILE/LINE/FUNCTION/COLUMN` need invocation-point facts, including
caller substitution for defaults. Function-name strings have declaration context
instead. [Clang's source-location extension contract](https://clang.llvm.org/docs/LanguageExtensions.html#source-location-builtins)
describes default arguments and member initializers; reusing the declaration
scope would supply the wrong location. The required source-location fixture is
still an implementation failure, not waived by this boundary.

A trial extension of `__builtin_is_constant_evaluated` exposed a separate
initialization defect: constexpr execution computed true, while local constexpr
storage still lowered the initializer's runtime call and stored false. The
[retained reducer](../student.tests/pa29/pending164/evaluation-context.cpp) checks
that distinction. The incomplete extension was removed; the final compiler
continues to reject this unimplemented builtin. The GNU builtin's meaning is
documented in [GCC's builtin contract](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html).
Its owner must track evaluation context through cached constexpr activations,
queries and mandatory/trial constant initialization, then publish the chosen
storage value for lowering. Always returning true, or folding every initializer
merely because the builtin appears elsewhere in the TU, would be incorrect.

Those two surfaces require new context-sensitive fact ownership beyond the
immutable declaration/string group. Their reducers and course failures stay in
the unfinished ledger. Independent review still owes verification of this
group's query keys, ownership, rendering and performance evidence; this handoff
does not certify PA29 or the surrounding template/ABI implementation.
