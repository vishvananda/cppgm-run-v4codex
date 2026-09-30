# PA25 implementation140 ownership and review trace

The 15 entry failures share two owners: source exceptions (14) and source
floating evaluation (1). All course inputs, reference outputs and comparison
rules are preserved. The private PA25 object contract advances from version 2
to 3 for the runtime-role changes; old internal objects must be rebuilt.
PA26/27 retain ownership of host-compatible metadata and objects.

## Exception data flow and lifetime

Semantic analysis records constructor-handler scopes, original function-try
syntax and template bindings. Lowering wraps constructor initializers and
destructor work in the protected region. Completed subobjects are destroyed
before a function handler executes. Falling out of a constructor/destructor
handler rethrows; returning from a constructor handler is rejected. Returning
from a destructor handler ends the catch without destroying subobjects twice.

Native selection indexes each block's leading clause markers once into
function-owned handler/clause arrays. Selection consumes those indexes directly;
synthetic labels cannot index the source-block workspace. Work is linear in
blocks plus clause markers. A 96-byte stack registration saves the exceptional
edge state and records cleanup retention/activation. Catch-only registrations
pop on transfer; cleanup registrations retire at `eh.end` or `resume`.
An exception escaping active cleanup terminates. Label ownership validation
rejects unresolved and cross-function branch targets before encoding.

Retained typed symbol/fixup edges demand a finite LowIR runtime program. Runtime
state entities are remapped explicitly into the native image; no symbol-name
guessing, reparsing or external runtime supplies behavior. The existing indexed
linker visits actual definitions/relocations. Runtime IR is released after
native object construction; selection storage is released per function.

An 80-byte exception header owns the payload, type/destructor, adjusted pointer,
caught-stack link and saved pending state. Signed handler counts preserve the
payload across nested catches and rethrows. Final catch exit destroys and frees
the payload; escaping its destructor terminates. Failure while constructing the
payload frees the allocation. Exception-allocation failure terminates, avoiding
recursive `bad_alloc`; ordinary `new` throws a typed `bad_alloc`. `bad_cast` and
`bad_typeid` use the same protocol. Direct C `malloc` failure returns null.

Matching consumes canonical RTTI identity, pointer qualification and hierarchy
facts. Base adjustment requires a public unambiguous subobject; private paths
still contribute to ambiguity. Null pointers traverse type/subobject identities
without dereferencing the pointer. Virtual identities coalesce by anchor type
and offset. Runtime traversal is proportional to inheritance paths, with stack
proportional to depth; no graph-linear claim is made. `nullptr` conversions use
the existing source member-pointer representation (zero is null), not a guessed
foreign ABI representation. Compiler runtime construction has fixed size.

## Floating evaluation and bounded selection

Production source F32/F64 binary operations carry evaluation type F80 in the
existing typed instruction field. This agrees with the constexpr evaluator and
the permitted excess-precision behavior observed in the unchanged calculator
contract. Plain PA24 LowIR retains exact-width evaluation. The optional
`[eval=f80]` view survives reader/writer/validation, selection and MIR dumping;
the encoder evaluates at 80 bits then stores at the declared precision.
The source type and instruction count do not change.

For multiplication only, a finite nonzero constant power of two proves the
product of any binary32/64 operand exactly representable in binary80 before
the final narrowing. Normal and subnormal powers, signs, zero, infinities and
NaNs are covered. Selection therefore uses the shorter ordinary SSE sequence
and marks MIR `exact=scale`. Unknown constants retain the F80 sequence.

The explicit selection budget is two constant-size bit tests per candidate,
zero additional allocations or IR nodes, zero additional frame bytes and zero
native text growth. There is no retained analysis or invalidation state: the
proof is recomputed from the current typed operands during selection. ABI and
debug ownership remain on the same instruction. The scale path removes x87
operand spills/reloads; unrelated general arithmetic retains the necessary
extended-evaluation traffic. [Performance140](performance140.md) records its
measured benefit against the first correct implementation and the remaining
cost against the entry compiler.

## Validation and independent review

[Exceptions](exceptions.py) exercises nested payload ownership, rethrow,
cleanup, qualification/base matching, function-try lifetimes, failure services,
termination and separate/direct/mixed three-TU linking. [Runtime trace](runtime-trace.py)
validates the typed runtime, object roundtrip, MIR ABI and a demanded class
template through source/LowIR/native/ELF. [Production trace](production-trace.py)
validates source evaluation and a demanded thrown-class template through the
typed production path; its textual roundtrip is an inspection test, not phase
transport. The scale control checks 60,060 double and 60,060 float comparisons
against explicit long-double evaluation, including subnormals and NaNs.

The inherited unused-dependent-local item is closed by the standard proof in
[deferred-member-proof140](deferred-member-proof140.md), with both invalid and
valid deferred-body controls run explicitly. No reference correction is needed.
Whole-stage independent review remains required, particularly the EH state
machine, matching identities, canonical demand and precision/profitability
proofs. Passing implementation checks is not that independent review.
