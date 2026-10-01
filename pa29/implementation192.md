# Implementation192 — extended floating formats

Entry HEAD: `4becf437ada14c36b01baae1d608428a621a5317`, clean, **399/403**.
Implementation tip: `dca6f1f2`. Stage base and Last reviewed are preserved in
[plan.md](plan.md). This handoff completes the scalar floating representation
owner; it is not whole-stage certification.

## Ownership and data flow

Posttoken decoding records the actual literal type and bits. Canonical types
retain binary16, GNU binary128 and the distinct C++ `_FloatN` identities.
`__float80` shares `long double`; the standardized-width types retain their own
identities even when their representation matches an ordinary type. Canonical
representation/precision queries drive arithmetic conversions, narrowing,
constants, layout and lowering. Mangling consumes those same identities.
Explicit lexical typedef declarations win over contextual `_FloatN` builtin
names, so the existing captured legacy hosted profile can compile glibc's older
typedef branch without a filename or library-name exception.

The new support owner decodes decimal/hexadecimal half and quad literals by
integer rational rounding, including ties, subnormals and sticky tails. The
constant pool uses a binary128 numeric carrier with both 64-bit halves in its
key. Destination precision is materialized; ordinary operations retain the
existing permitted long-double excess precision. Integer conversion rounds
directly to the destination precision before carrier construction, avoiding a
second rounding through quad. Out-of-range constant integer conversions remain
invalid constant expressions. Static values and representation bit casts retain
all bits, including signed zeros and NaN payloads. The constant-array cache now
compares/hashes both halves of wide integers and complete extended values.

Typed lowering emits `f16`/`f128`; the LowIR reader, validator, writer, native
adapters and debug dump retain these types. Finite quad values use exact hex
text only at explicit inspection/serialization boundaries. NaN payloads and
half constants use typed storage when text cannot express the bits. Production
compilation never serializes/reparses LowIR or delegates compilation.

Native target legalization runs once before selection. Quad operations and
extended conversions become ordinary typed calls to the platform libgcc ABI;
half arithmetic extends to binary32 and materializes the half result. Those
calls share normal argument, clobber, relocation and unwind handling. Negation
uses the sign bit. Floating comparisons preserve unordered semantics. A raw
LowIR branch may require legalization based solely on a parameter value or bit-initialized slot's type.
Scalar half/quad arguments and results occupy XMM registers, with ordinary stack
overflow and alignment. Variadic saves retain all 16 bytes of each XMM argument.
Host linking resolves arithmetic helpers, as it already resolves required host
runtime calls; no host compiler supplies semantic or machine-code output.

## Complexity, budgets and release

Literal scanning is O(source bytes). The decoder retains at most 20,000
significant digits (beyond every binary128 rounding midpoint); later digits
supply a sticky bit. Limb work is bounded by that fixed-format maximum and
scratch is released after decoding. Canonical facts and constant records belong
to the TU and use existing numeric indexes, not printed type/expression keys.
Semantic interpretation continues charging its existing evaluator budget.

Legalization scans input instructions/value types, allocates work pools only
when extended formats occur, and emits at most six operations for one input
operation. Helper declarations are indexed by operation/input/result identity.
There is no fixed-point pass, specialization replay or optional optimization.
Time and storage are O(input + produced IR); replaced pools are released at the
end of preparation. Low-overhead work/added-instruction/helper counters join
existing preparation time. Existing constexpr, inline, native-storage and course
time limits are unchanged. [Performance192](performance192.md) records all four
performance dimensions and applies spec §9's stage-scoped acceptance.

## Validation and correction

[check192.py](../student.tests/pa29/check192.py) explicitly runs O0/O2 semantics,
template constants, narrowing/rejection, subnormal/tie/zero/NaN cases, hosted
header typedef compatibility, mixed field layout through a host pointer,
GCC calls in both directions, register/stack/variadic arguments, exact LowIR
roundtrips, raw floating branches and ELF/MIR/unwind inspection. Its independent
[decoder oracle](../student.tests/pa29/check_decoder192.py) uses Python rational
arithmetic, not compiler agreement. The inherited vector controls exercise the
shared constant/bitcast/cache paths again. Final counts and source binding are
in [the evidence](../student.tests/pa29/evidence192/validation.json).

One [proved reference correction](reference-corrections192.md) repairs the Q
literal's type/bytes. The reducer, C++11 extension rules, explicit GNU contract,
encoding derivation and pinned bundle revision are recorded there. Every input,
status and comparison rule remains; all other contract files retain their bytes.
The preliminary header failures and invalid overlapping report counts are
retained in the validation history; isolated final reports establish coverage.

## Remaining boundary

All three entry floating-format failures are resolved. The remaining nested
member ABI-tag case belongs to attribute propagation and the supported GNU/Clang
extension contract. The existing [implementation176 reducer and proof boundary](implementation176.md#independent-contract-question-nested-member-abi-tag)
show GCC retaining the declared tag and Clang dropping it. C++11 does not choose
that extension encoding. This work supplies no new proof authorizing a reference
change, and the oracle/failure remains counted. This is an independent contract
review question, not an unimplemented scalar floating path or a waived test.
Further floating changes cannot resolve it; a naming-policy decision is a
separate owner boundary. Full PA29/root-through success and whole-stage audit
remain required before advancement.
