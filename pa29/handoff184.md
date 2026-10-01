# Implementation184 — qualification-preserving explicit casts

Entry: `68ee6f8d12349e4ba9576963c200178cf4ae629e` (**389/403**).
Code: `52c00f4d`. Target remains **PA29 full-stage**; this is a completed
implementation-group handoff, not assignment completion or independent audit.
The preceding implementation183 turn made progress: its committed guide/default
owner and required validation changed authoritative state; this turn inspected
the clean entry, actual failure log, requirements and existing review ledger.

## Completed behavior and language rules

The Darwin wait-status fixture removes `const` from an `int` pointer before
reading its original constant object. Explicit conversion selection incorrectly
classified that C-style cast as reinterpretation and prohibited constant
evaluation. C-style pointer and data-member-pointer casts now select a valid
cv-only conversion before static/reinterpretation alternatives. Constant member
values preserve member identity when qualifiers change.

The same owner was extended through nested pointers, reference category rules,
array references, dependent substitution, null values and casts that combine
qualifier removal with an inheritance conversion. Those composed conversions
now retain the selected base displacement, including nonzero and virtual-base
upcasts. Private unambiguous bases retain C-style access rules; ambiguous and
virtual downcasts remain rejected. Named reinterpret casts check the complete
qualification chain, including the intervening-const rule, rather than only the
outer pointee. Actual reinterpretations still prohibit constant evaluation.

[C++11 N3337 expr.cast/4](https://timsong-cpp.github.io/cppwp/n3337/expr.cast#4)
defines the ordered cast interpretations and the C-style base-access exceptions.
[expr.const.cast/3–6,8–12](https://timsong-cpp.github.io/cppwp/n3337/expr.const.cast)
defines object identity, reference categories, data-member pointers, nulls and
qualification removal. [expr.const/2](https://timsong-cpp.github.io/cppwp/n3337/expr.const#2)
excludes evaluated reinterpretation, forbidden reads and mutation; it does not
exclude identity-preserving qualifier casts. No reference output or course
fixture changed.

## Owner, data flow, complexity and lifetime

- `semantic/explicit_conversion.cpp` owns selection for both ordinary expressions
  and substitution queries. It consumes canonical `TypeId` chains and existing
  class-relation facts, then publishes the existing `Conversion` with target,
  reference category, base adjustment and constant-evaluation prohibition.
- `semantic/constant.cpp` consumes the selected explicit member conversion while
  retaining the member identity and displacement. Existing constant storage and
  address records preserve the object's declared type, lifetime, readability
  and array/subobject boundaries, even when the expression loses qualifiers.
- Constant execution consumes the recorded conversion. Ordinary typed lowering
  consumes its target/adjustment; it neither repeats overload selection nor
  reconstructs the cast from its spelling. Base-pointer and member-pointer
  lowering, per-function MIR selection and direct ELF writing remain shared.
- Qualification checks use O(depth) time and O(1) new temporary storage. Selection
  visits only the two type chains and required indexed base paths. No new cache,
  parser replay, whole-program retry, owning node, source-string identity or
  production serialization is introduced. Published facts and address/member
  identities have the existing translation-unit lifetime; native transient
  storage retains its per-function release boundary.

The inspection follows `base-adjustments` from a constexpr multiply-inherited
object through pointer/reference/member conversions to constant and runtime
accesses, and `dependent` through source template, substituted default/argument
facts and emitted specialization. Typed cast records show retained offsets and
constant restrictions. `--stats` leaves objects unchanged. Explicit LowIR
roundtrip, native adapter, MIR, relocations, symbols and unwind views cover all
12 positive controls. The virtual-base adapter presents constructor sections in
a different order; comparing every complete named section proves identical
instructions and relocations without discarding sections or symbols.

## Validation and performance

[Validation](../student.tests/pa29/evidence184/validation.json): PA29 **390/403**;
PA1–28 **4538/4538**; through PA29 **4928/4941**; file audit passes with the same
four inherited warnings. The [stage delta](../student.tests/pa29/evidence184/stage-delta.json)
proves **14 → 13** original failures, with no new failure. All **403** inputs and
**1,707** contract/harness paths are unchanged.

All **42** explicitly executed controls pass, including Clang checks, generated
native executions and LowIR validation. Class-prvalue reference casts also
preserve full-expression/scope destruction and exception cleanup, and nonliteral
temporary reads remain unavailable to constant evaluation. All **194** inspection commands pass.
The first personal rejection assertion for a C-style conversion to a const
scalar reference was incorrect: direct initialization may create a temporary.
It was replaced by positive `reference-temporaries`, covering both constant and
runtime temporary values. Preliminary observations are retained in the evidence;
this did not change course coverage or an implementation requirement. A later
personal lifetime check was corrected to observe destruction after the full
expression instead of inside its condition; both compilers agreed before and
after that test correction. No implementation change was needed.

[Performance184](performance184.md) records the frozen compiler latency/peak RSS
and checked executable runtime/text evidence, A/A noise calibration, ABBA
comparisons and newly accepted owner scaling. This is required semantic work,
with no optional optimization or claimed speedup. Existing mandated work/growth
limits and all prior performance evidence remain intact.

## Handoff ledger and concrete boundary

| Disposition | Owner / evidence | Outstanding obligation |
|---|---|---|
| Completed implementation group | Explicit cv-only cast selection, composed base adjustments, reference categories and nested qualifier rejection; controls/typed/native inspection above | Independent review of the cumulative committed range remains required. |
| Unfinished implementation | **10** cases in the [remaining ledger](../student.tests/pa29/evidence184/remaining.json) | Extended integer/floating/complex types and operations, vector deduction/lowering, contextual coroutine syntax and hosted template behavior. |
| Independent contract questions | **3** unchanged counted failures in that ledger | Nothrow shorthand, invocable-cache expectation and nested-template ABI-tag policy require independent resolution; none is waived. |
| Whole-stage review | Preserved [audit182](audit.md), earlier audits and plan review markers | Audit cumulative changes and resolve whole-stage findings before advancing. |

The original cv-only repair was extended through all discovered related
selection and constant-value defects, including native base displacements and
negative substitution/qualifier controls. None of the remaining failures reaches
the repaired cv-only path. The hosted functional cast needs new floating-type
representation and native operations; dependent `_BitInt` fails type parsing;
complex/vector cases need their own type and ABI/expression owners. Coroutine
syntax and the hosted-template/contract cases likewise cannot be completed by
extending qualifier selection or its retained conversion facts. They remain
implementation or review work as individually recorded, rather than new gates
or waived requirements. Full PA29/root-through success and whole-stage audit
remain required before PA30.
