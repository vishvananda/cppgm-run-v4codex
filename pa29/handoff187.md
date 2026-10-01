# Implementation187 — floating GNU complex values

Entry `5ef7f89a3bd6be203c6a845432b417bcf518eb07` had **392/403** PA29
passes. Implementation `5c93ddef`, with MIR reporting repair `f714397c`, has **394/403**: both
`600-builtin-complex-return` and `600-gnu-complex-template-constructor` pass.
The other nine original failures remain counted. This is an implementation
handoff; the stage and whole-stage review are unfinished.

## Shared ownership, data flow and complexity

| Owner | Representation and consumers | Work and lifetime |
|---|---|---|
| Syntax/type formation | GNU complex specifier and component operators become tokens; the specifier forms one of three canonical fundamental types. Dependent component expressions retain their operator in the existing query graph. | Existing token/type/query indexes; no source-text recovery or library-name matching. |
| Semantic values | The canonical component type drives conversion, arithmetic, traits, projection cv/value category, constant execution, component addresses and initializer facts. `__builtin_complex` requires two equal real floating types and forms a typed intrinsic call. | Two interned floating-constant IDs fit the existing constant payload. At most three builtin signatures per TU, keyed by the first canonical type after checking equality with the second. Existing constant execution limits apply. |
| Typed lowering | Complex values use object storage with a retained component tag, scalar loads/stores and typed signatures. Construction preserves signed zeros and exceptional values. Addition/subtraction are component operations; multiplication/division call typed runtime helpers. | Fixed two-component expansion. Six cached runtime declarations per linked LowIR program. No optional optimization, retained syntax body, new per-node arena or unbounded worklist. |
| LowIR/native ABI | `c32`, `c64`, `c80` preserve complex identity through validation, printing, parsing, calls and `va_arg`. Native placement uses packed float pairs, two double registers or the x87/stack long-double convention. | Constant work per scalar pair or ABI argument. Register exhaustion rolls back the whole pair. Long-double results and ignored results balance both x87 entries. |

The intrinsic registry also owns its probe answer. Compiler-intrinsic addresses
are rejected. Real-to-complex conversions zero the imaginary component;
complex-to-bool tests both components. Ordinary implicit complex-to-real
conversions are rejected. Explicit conversion follows the Clang GNU concession.
Comparisons support equality/inequality; ordering and integer-only operators
diagnose. Component assignment retains const/volatile qualification.

Constant values and runtime values use the same semantic type and component
order. Static arrays, class fields, constexpr objects and component references
consume existing initializer/address facts. There is no fixture-specific answer
or hosted-only output path. The ordinary LowIR signature carries runtime helper
names and ABI types into object emission. The `__mul{sc,dc,xc}3` and
`__div{sc,dc,xc}3` calls are ordinary libgcc arithmetic runtime dependencies;
the compiler implements parsing, semantics, LowIR, call lowering and objects.

Two shared gaps exposed by this integration are repaired at their owners:
native `Index` addresses object-valued SSA storage as well as slots, and lowered
`va_arg` retains its semantic result type for downstream conversion. The LowIR
validator admits complex variadic arguments specifically, without accepting
arbitrary object varargs.

Final inspection also corrected the MIR ABI summary to name the actual complex
parameter/result carriers. Execution already used those carriers. The explicit
adapter checks now verify the printed XMM/x87 placement as well as execution.

## Validation and semantic evidence

`make test-pa29`: **394/403**, exit 2. `make test-report-through-pa29`:
**4932/4941**, with the same nine PA29 failures and every earlier test passing.
The explicit earlier-stage report passes **4538/4538**. File audit passes with
the same four header-division warnings. All **403** course inputs and **1,707**
contract/harness paths are unchanged; references and comparison rules are intact.

`python3 student.tests/pa29/check187.py /tmp/pa29-187/checks` passes **99 commands**,
including **18 rejection inputs** checked by both this compiler and a host:

- Three precisions: layout, types, builtin construction, constexpr execution,
  template construction and dependent component references.
- Static arrays/fields, component addresses, copies, volatile access, scalar
  side effects, zero initialization, bool/real conversions, compound operations,
  signed zero, infinities/NaNs, and scaled division of large values.
- GCC and Clang callers linked with compiler objects, reverse calls into host
  functions, register overflow, long-double returns and discarded returns.
  Variadic calls retrieve every precision through registers and stack storage.
- Host-mode LowIR validation; a standalone three-precision adapter is parsed,
  canonicalized twice, executed before/after canonicalization, and inspected as
  MIR. ELF symbols and disassembly are retained as hashed inspection evidence.
- Wrong arity/types, invalid specifiers/operators, const component writes,
  forbidden implicit conversion, dangling constexpr references, nonaddressable
  imaginary scalar projections, builtin address-taking and complex vector lanes.

The [GNU extension documentation](https://gcc.gnu.org/onlinedocs/gcc/Complex.html)
defines the builtin's equal real floating operands, component operators and
construction behavior with exceptional values. Component cv preservation also
follows the underlying C++11 const-object rules. Clang accepts two additional
concessions (writing a const complex component and taking the address of an
imaginary projection of a real scalar); GCC is the rejection control for these
two cases. No checked-in oracle is changed on host agreement alone.

The [Itanium ABI](https://itanium-cxx-abi.github.io/cxx-abi/abi.html) already has
`Cf`, `Cd`, `Ce` complex builtin encodings. Dependent component operations use
its vendor unary form, yielding GCC's `v18__real__` and `v18__imag__` encodings.
Clang erases these operations in dependent return-type mangling. The retained
typed operator and GCC encoding are deliberate; cross-vendor dependent-template
symbol agreement remains an independent audit question, not a failure waiver.

[Performance187](performance187.md) records the frozen entry/final comparison,
all samples, startup calibration, demand scaling and compiler/runtime/text
costs. No optimization benefit is claimed.

## Remaining work and concrete handoff boundary

The [remaining ledger](../student.tests/pa29/evidence187/remaining.json) preserves
every remaining original failure, its diagnostic and disposition:

| Group | Failures | Next required work |
|---|---:|---|
| Extended floating representation | 3 | Binary128/half types, precision, conversions, constants and ABI. Aliasing these to existing real types would give incorrect layout and arithmetic. |
| Vendor vector/contextual syntax | 2 | Dependent extended-vector deduction; contextual coroutine operator parsing/semantics. |
| Hosted template demand | 1 | Primary `char_traits` conversion-shim reducer and incomplete-type demand. Existing contract discussion is preserved; no special standard-library recognition. |
| Independent contract questions | 3 | Nothrow shorthand, nothrow invocability and nested-template ABI-tag expectations from the previous audit. Requirements and failing results remain counted. |

This turn extended its initial two complex fixtures through constant/runtime
semantics, storage, serialized IR and fixed/variadic ABI consumers. That owner
is now coherent and validated. The next numerical group requires real formats
that the existing floating-constant store and native arithmetic cannot represent;
the completed two-component model does not supply those primitives. The other
groups require independent parser/template/contract work. Further unrelated
architecture investigation is deferred to the next implementation turn and
scheduled Ralph audit. Full PA29 and root-through success is still required
before PA30. Existing audits and all historical measurements are preserved.
