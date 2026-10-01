# Implementation185 — bit-precise integer types and boundaries

Entry: `c6d7a46abacfb0862c3955f2367dd902a6a714e6` (**390/403**).
Implementation commits: `8ee20450`, `27012cf5`. Target remains **PA29 full-stage**.
This is an implementation-group handoff, not whole-stage completion or audit.
The preceding implementation184 turn made progress: committed cast repairs and
validation changed authoritative state. This turn rechecked the clean worktree,
actual failure log, specification, handout and review ledger before editing.

## Behavior and extension contract

Both dormant out-of-class member-template fixtures now retain `_BitInt(N)` as a
dependent type. The same implementation supports demanded signed and unsigned
widths, aliases, casts, exact type identity, layout, traits/transforms, overload
conversion rank, constexpr evaluation, ordinary arithmetic, fields, template
width deduction, packs, partial specializations and ABI names. Bit-precise
operands retain their precision instead of undergoing ordinary integer
promotion; ordinary integers rank above equal-width bit-precise types. Constants
and runtime values wrap unsigned results and sign-extend signed representations.
Signed constant overflow and invalid widths are rejected.

This is the C23 vendor extension in C++11, as described in
[Clang's extended integer documentation](https://clang.llvm.org/docs/LanguageExtensions.html#extended-integer-types)
and [WG14 N2763](https://www.open-std.org/JTC1/SC22/WG14/www/docs/n2763.pdf).
The [Itanium ABI builtin-type grammar](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#builtin-types)
encodes precision and signedness as `DB`/`DU`, including dependent expressions.
The scalar implementation supports signed widths **2–128** and unsigned widths
**1–128**, matching the existing constant/native scalar engine's capacity.
Larger concrete widths receive an explicit unsupported-precision diagnostic;
they are not represented by a narrower integer. Arbitrary-precision native
arithmetic is outside this completed group and remains an explicit implementation
limit, rather than a claim of general C23 implementation.

Wide bit-precise scalars have 8-byte storage and stack alignment, including
variadic overflow arguments. Their source precision remains semantic; their
128-bit LowIR carrier records its ABI stack alignment as **`i128a8`**. This is a
serialized typed scalar decoration, not a hosted-only side table: all existing
`i128` operations apply, full type equality includes the ABI fact, and both call
sides and `va_arg` consume it. Ordinary `i128` keeps its existing 16-byte stack
alignment. LowIR readers/writers, validation, MIR rendering and native adapters
preserve the decoration. Imported padding is normalized before a value is used.
Extended scalar RTTI is emitted through the ordinary fundamental-type-info
owner, so throwing/catching these types works with the host runtime.

The inherited atomic/vector engines do not support bit-precise element types.
Their declaration owners reject these operands, consistent with Clang, including
a dependent atomic operand after substitution. Existing ordinary atomic/vector
support and every course fixture remain unchanged.

## Owner, data flow, complexity and lifetime

- Syntax specifier parsing retains the width expression once on `BitIntType`.
  Bounded lookahead skips its balanced delimiter without reparsing the region.
  Functional, named and C-style casts use the ordinary type-id owner.
- `semantic/bit_integer.cpp` forms either a canonical `DependentBitInt` carrying
  an existing width-query ID, or a fundamental type carrying exact width and
  signedness. `Types::intern` includes that width in equality and hashing.
  Substitution uses the existing complete frame/query cache; invalid widths
  return the existing compact failure result. Pack collection and declaration
  deducibility consume those typed edges. No rendered spelling is a key.
- Shared conversion/rank, constant and layout owners consume exact precision.
  Integer constants use the existing bounded 128-bit intern pool above 64 bits;
  bit-integer width does not create a parallel constant representation.
- Lowering consumes selected conversion/type facts. At a required normalization
  boundary it emits at most two ordinary shifts; literals normalize directly.
  A lowering `Value` retains the type for which its representation is already
  normalized, preventing repeated shifts while that fact remains valid.
  New operations/conversions reset that local proof naturally with their value.
- ABI graph nodes retain the width query or concrete width and signedness.
  Encoding and explicit ABI fact adapters consume them. Typed LowIR carries
  stack alignment into shared call placement, variadic selection and ELF output.
  The RTTI owner emits each demanded support object once by canonical type.

Additional scalar work is O(1) per type/conversion/operator for the bounded
128-bit target; dependency/deduction work follows existing query/type edges.
Width expressions incur their ordinary constant-evaluation cost, not a separate
scan. No new fixed point, global retry, cache family, owning AST node, text
transport or host compilation dependency is introduced. Types, queries and
constants have translation-unit lifetime; lowering value proofs are transient;
LowIR facts have program lifetime; native transient state retains its existing
per-function release boundary. A new normalization contributes no branches or
calls and at most two instructions. Existing whole-pipeline inline work/growth
limits are unchanged.

## Evidence

[Required validation](../student.tests/pa29/evidence185/validation.json):
PA29 **392/403**, PA1–28 **4538/4538**, through PA29 **4930/4941**.
[The failure delta](../student.tests/pa29/evidence185/stage-delta.json) is exactly
**13 → 11**, with no new failures. File audit passes with four inherited header
warnings. All **403 inputs** and **1,707 contract/harness paths** remain unchanged
against entry; no reference correction or comparison change was made.

All **31** explicit controls pass with this compiler and Clang; positive objects
are host-linked and executed, and their LowIR is validated. Controls include
widths 1/7/9/19/32/64/65/93/128, constants versus runtime, signed division,
unsigned wrapping, bit-fields, references, padding, substitution rejection,
width packs/deduction, native stack/register crossings, variadic arguments,
aggregate layout and exception destruction. Host ABI tests exercise calls in
both directions with Clang-built peer objects.

All **169** inspection commands pass across the ten positive controls. Inspection
follows source and demanded template type facts through LowIR and direct ELF;
checks AST/typed facts, concrete/dependent ABI fact roundtrips, LowIR roundtrips,
serialized/native section equality, symbols, MIR, relocations and unwind data;
and proves `--stats` leaves objects unchanged. Host symbol checks compare source
function names, including dependent widths and bit-integer non-type arguments.
The preliminary attempt to compare *all* host symbols found this compiler's
extra weak implicit assignment function, which Clang folds into its caller.
The evidence preserves that observation; final checks compare the named source
boundaries and still compare **all** student/adapter symbols and sections.

[Performance185](performance185.md) records frozen compiler latency/peak RSS,
checked runtime/text size, A/A noise and ABBA comparisons, and new width-demand
scaling. Required semantics introduce no optional optimization pass or claimed
speedup. Mandated limits and all historical performance evidence remain intact.

## Handoff boundary and remaining obligations

The initial two parse failures were extended through every discovered related
width, constant, deduction, padding, bit-field, ABI and RTTI defect. The current
[11-case ledger](../student.tests/pa29/evidence185/remaining.json) retains **8
unfinished implementation cases** and **3 independent contract questions**.

The remaining numeric cases need floating formats (`_Float16`, `_Float64`,
`__float128`) or complex representation and ABI. They cannot use integer width
normalization: quad precision needs its own floating constants, conversions and
operations, and complex values need paired component/return classifications.
Vector expressions/deduction and contextual coroutine syntax also have different
semantic and native/parser owners. The char-traits fixture remains recorded
work; preliminary examination of forward declarations/reserved library names
does not constitute a standard/contract proof for changing its oracle.

Independent contract questions remain nothrow shorthand, invocable-cache
expectation and nested-template ABI-tag policy. They still count as failures;
none is waived. Independent review must inspect the cumulative implementation,
including `i128a8`, the explicit precision limit and the retained query/deduction
facts. [Audit182](audit.md), earlier handoffs/evidence, Stage base commit and Last
reviewed commit are preserved. Full PA29/root-through success and whole-stage
audit remain necessary before advancing to PA30.
