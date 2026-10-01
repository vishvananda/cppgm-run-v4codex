# PA29 implementation191: vector expressions and representation intrinsics

Entry: `32c1f5f43cde4e09f623f0e5d606a51d170e8203`, 398/403, clean.
Implementation tip: `f542b376` (the following record commit changes evidence only).
The stage/review markers in `plan.md` are preserved. This is an implementation
handoff, not a claim that PA29 or its whole-stage audit is complete.

## Owner, data flow and spec alignment

The failing vector/deduction fixture belongs to one owner spanning type-operand
syntax, canonical queries, vector operations, representation and ABI. The parser
records an intrinsic enum, optional TypeId syntax and expression operand. A
bounded shared registry supplies names and feature probes. Semantic queries
retain the operation, canonical target type and child query; ordinary expression
facts retain the result and conversions. Template demand substitutes these
facts, including SFINAE, without reparsing syntax or replaying type names.

`semantic/value_builtins.cpp` owns operand legality, comparison mask types and
constant evaluation. `lowering/vector_values.cpp` consumes those facts to emit
ordinary typed LowIR. `__builtin_bit_cast` copies equal-sized trivially-copyable
representations, evaluates its source once and does not insert a C++ copy or
array decay. `__builtin_convertvector` converts equal-count lanes;
`__builtin_reduce_or` reduces integer lanes. Comparison results use signed
integer lanes of the corresponding width, including long for eight-byte lanes.
Boolean extension vectors use packed bits. Constant vectors, globals, local
lists and constructor members use their existing object/initializer-plan owners.
Scalar constant bit casts retain negative zero and NaN payload/signaling facts.

GNU and Clang extension vector conventions remain distinct typed LowIR facts:
`vec<bytesxalignment>` and `evc<bytesxalignment>`. Reader, writer, validator,
native calls/returns and MIR use that identity. Small vectors use integer or XMM
registers according to width. Wider arguments use stack storage; GNU results
over 16 bytes are indirect, whereas extension results of 32/64 bytes use
multiple XMM registers and larger results are indirect. Host interoperability
controls cover both directions, including 128-byte extension vectors. No source
spelling or hidden hosted side channel controls native emission.

The ABI graph has a vendor-expression node whose children are typed template
arguments, preserving the intrinsic's type/expression operand order. Its text
reader/writer are explicit adapters. The integrated compiler constructs the
graph directly. Encoding follows Itanium's
[vendor expression grammar, §5.1.6](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#expression).
Vector/bool and intrinsic behavior is checked against the relevant
[Clang language extensions](https://clang.llvm.org/docs/LanguageExtensions.html).
No course oracle is revised by this work.

Facts and constant objects live in the translation-unit semantic arenas with
canonical type/query keys. Existing completion dependencies own query invalidation;
no new global cache, retry mechanism or cross-TU state is introduced. Lowering
temporaries are per expression/function, and function MIR is released after
encoding. Initializer plans and conversions remain the sole recorded semantic
decisions, including retained template-definition checks and constructor members.

## Complexity and related repairs

For a vector operation, lowering emits at most eight lane bodies or one counted
loop. Wider runtime work is O(lanes), with O(1) instruction growth per operation;
storage follows the vector's representation. Explicit initializer clauses cost
O(clauses); an omitted suffix is zeroed in bulk. Constant evaluation costs
O(lanes), charges the interpreter while active and caps a vector traversal at
1,000,000 lanes. Existing 1,000,000-step/depth-512 evaluator limits, native bounds
and forced-inline budgets remain intact. ABI arguments and template demand cost
O(consumed edges/facts); no unrelated declaration scan or Cartesian product is
added. [Performance191](performance191.md) retains measured counters and costs.

The initial owner expansion repaired array/deleted-copy bit casts, volatile and
comma-expression sources, malformed operands, packed static storage, comparison
mask types and vector ABI distinctions. Scaling then exposed fixed vector lists
with dependent initializer values being treated as scalars. The retained
initializer checker now consumes vector lanes through the sequence path, checks
fixed excess/type errors before demand, and reuses the ordinary list owner for
fixed constructions. A related constructor-member path bypassed initializer
plans; it now reaches the same vector initializer owner, including packed bools.
All discovered defects in this group are repaired, not reclassified as audit
questions. The preliminary measurements and failure evidence are retained.

## Validation and handoff boundary

`student.tests/pa29/check191.py` explicitly runs 191 commands: O0/O2 compilation
and checked execution, source-generated LowIR validation/roundtrip/native MIR
and execution, GCC/Clang interoperability, negative definitions and operands,
ELF sections/symbols/relocations/unwind, global qualification and ABI fact
parse/write/parse/encoding. Clang corroborates the positive/negative semantics;
its dependent bit-cast return is checked with syntax-only because Clang itself
cannot mangle that expression. Our compiler emits and executes that symbol.
These controls supplement unchanged course coverage.

The remaining three floating-format failures require binary16/binary128
representation across literals, constants, typed IR, operations and ABI. They
are unfinished implementation. The fourth failure concerns nested-template ABI
tag suppression and remains an independent contract question recorded by the
prior reducer (GCC and Clang disagree). Its existing oracle/failure remains.
Further vector fixes do not provide the representations or proof those owners
need; starting either is a separate broad implementation or contract review.
The failure ledger preserves their full identities and requirements. Passing
PA29/root-through and the whole-stage audit is still required before PA30.
