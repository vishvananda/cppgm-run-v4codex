# Implementation181 handoff ledger

Entry `c0efbf2992a9e58dbaa2d1e7300496012eed5988`: **387/403**, 16 failures.
Code tip `0e47335f`: **GNU zero-extent array identity, layout and lifetime** is
complete as an implementation group. PA29 remains unfinished. The preceding
turn made verified implementation progress; no inherited process was live.

## Ownership, data flow and complexity

| Owner | Retained fact and consumers | Work and lifetime |
|---|---|---|
| Canonical types | `unknown_bound` distinguishes `[]` from `[0]`; participates in hashing/equality and declaration-shape keys, and survives cv, signatures, aliases, partial ordering and substitution. | Average O(1) interning/equality; no rendered keys. Type remains 40 bytes. TU-owned records and caches; no new per-node allocation. |
| Declaration and demand | Hosted declarations admit zero; strict earlier semantic tools retain positive-bound rejection. Composite declarations preserve known zero; initialization cannot infer a new bound for `[0]`. An absent static-member bound demands that member's indexed definition even in `sizeof`, independently of emission. | Existing selected-definition state/cache and dependency owners. No global retry, unrelated member scan or parsing replay. |
| Template signature | A narrow signature check rejects zero arrays formed during immediate substitution while retaining the same valid canonical type for class and function bodies. It follows typed dependent edges and actual expansion lanes; failure belongs to the function specialization. | Once per attempted declaration key; linear in relevant type edges and actual pack lanes. Completed type substitutions are cache reads. Scratch is released on return. No exception for ordinary candidate rejection. |
| Nested declarators | Pack discovery follows only parenthesized declarator edges to the ellipsis; nested function parameter lists remain separate owners. Deduction, parameter instantiation, defaults, exception queries and pack binding share this operation. | O(declarator nesting), no token replay or cloned grammar. Controls cover pointer/reference packs and out-of-class members. |
| Layout and constants | Zero size retains element alignment and field offsets; a class containing only zero-extent fields can have size zero. Completeness is a separate size-query result. Empty-subobject placement sees no nonexistent elements. | Existing per-class layout cache; scalar array-size arithmetic, no work per absent element. Overflow and incomplete-element rejection remain intact. |
| Lifetimes and allocation | Accessibility/deletion still validate element operations. Zero elements emit no calls. Positive element counts with zero-sized element types still call constructors/destructors once per element. Allocation retains the inner count, independently of byte stride. | Existing eight-element expansion bound and counted loops; one extra eight-byte field per allocation. No division by zero, synthetic frontend nodes or name-based semantic recovery. |
| Typed LowIR and ELF | Object layouts can carry zero bytes with positive power-of-two alignment; zero-byte bulk operations emit no IR after operand evaluation. Empty constant copies create no backing support global. Explicitly typed zero-byte globals roundtrip. Object construction checks 64-bit producer widths before packing into the existing 32-bit extent representation. | O(1) local construction/validation and ordinary linear native processing. Untyped empty globals, zero bulk spans and oversized encoded extents still reject. ABI ignores zero aggregate register chunks; alignment and zero ELF symbol size survive. |

The hosted layout follows [GCC's zero-length-array extension](https://gcc.gnu.org/onlinedocs/gcc/Zero-Length.html):
zero size, element alignment and normal member offset/tail padding. The
immediate-substitution distinction follows N3485 §14.8.2 [temp.deduct]/8
([local draft](../doc/n3485.txt), zero/negative array formation). No checked
reference is changed, and compiler agreement is not offered as standard proof.

## Validation and repaired interactions

- PA29 **388/403**, exactly `700-gnu-zero-length-array-member-compile.t`
  removed from the entry failure set; no new failures. PA1–28 **4538/4538**;
  through PA29 **4926/4941**. File audit passes with the same four inherited
  warnings. [Validation](../student.tests/pa29/evidence181/validation.json) binds
  commands and summaries to the final source/binary hashes.
- **43 controls** check absent/zero identity, cv, arrays in class/union/interior
  layout, constant copies, static template members, alias/pack substitution,
  default-argument effects, deleted/incomplete elements, range loops, zero-sized
  by-value objects, heap count/cookie/unwind behavior and actual over-alignment.
- **166 inspection commands** check AST, LowIR validation/roundtrip, source vs
  serialized object emission, MIR, symbols, relocations, unwind records and
  execution. Host calls work in both directions, including zero aggregate
  arguments/results and distinct `A0_`/`A_` ABI names. Stats on/off objects agree.
- All **403 inputs** and **1,707 contract/harness files** remain byte-identical
  to entry and the preserved review boundary. Personal tests are explicitly
  invoked and do not change the required test count or comparisons.
- Controls exposed and fixed: zero-byte constant support objects; byte-derived
  allocation counts; zero pointer stride; static member definition bounds;
  parenthesized parameter packs; immediate substitution; and 4-GiB extents
  narrowing to zero. Wide semantic `sizeof` remains valid when no storage is
  emitted. Preliminary and final performance observations are both retained.
- The first exploratory suite runs shared report files when run concurrently;
  their summaries are not used as exit evidence. All final suite gates run
  sequentially. Final controls, adapters and source checks are tied to the
  corrected binary. [Performance181](performance181.md) records all four
  dimensions and stage-scoped acceptance.
- Evidence review corrected four negative LowIR invocations that had omitted
  the output argument. The final rerun verifies object alignment, zero bulk
  spans and untyped empty globals fail for their intended validation reasons.

## Independent review and handoff boundary

The [remaining ledger](../student.tests/pa29/evidence181/remaining.json) retains
**12 unfinished implementation** cases and **3 independent contract questions**,
all counted failures. Char-traits conversion remains implementation work;
legacy nothrow, invocable-cache and nested ABI-tag contract questions remain
unresolved. Audit178 and earlier audit records are preserved, not waived.

One new extension observation is recorded for review, without altering a course
oracle: GCC and Clang disagree on effects for arrays of a class whose GNU layout
has zero size. The compiler deliberately uses the declared element count for
both construction and destruction. The [reducer and raw executions](../student.tests/pa29/evidence181/host-effects-observations.json)
show compiler `3/3`, GCC `3/0`, and differing Clang counts for three elements.
This is disclosed interoperability evidence; it does not replace the checked
per-element lifetime controls or claim general host agreement.

Further related array work was pursued through deduction, constants, runtime,
lifetime/unwind, ABI and numeric-storage limits, rather than stopping at the
first passing fixture. The remaining required failures concern distinct
extended-number/complex representations, vector operations, GNU expression or
syntax forms, library conversion, or ABI policy. None consumes the corrected
zero-extent identity, count or storage encoding. Extending those owners needs a
separate behavior group; that is the concrete incomplete-handoff boundary.

Implementation commits: `e0619de9`, `f69601ef`, `0e47335f`; entry plan `42a2bac7`.
The final evidence/documentation commit returns control to Ralph. Stage-base and
last-reviewed markers remain unchanged. Full-stage completion and independent
whole-stage findings still must be resolved before advancement.
