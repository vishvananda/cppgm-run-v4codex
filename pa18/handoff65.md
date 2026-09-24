# PA18 implementation handoff 65

Implementation: `5d8a136e`, `644dd88b`; proved reference correction `4eaf273d`.
Entry `52d972234f7f4dfdb9ec4c0969f559b9521bdf62`: 312/420. Stage base and
last-reviewed commit remain `94dcb8ad21664137e87d574e878c14a4a047348a`.
Previous handoff was progress. PA18 full-stage remains unfinished.

## Completed group and design

- `template_conversion_deduction.cpp` owns target-driven conversion deduction:
  destination TypeId supplies A and declared conversion return type supplies P.
  C++11 reference/cv/decay and pointer qualification alternatives feed ordinary
  immediate default/signature substitution. Object viability precedes deduction;
  a template's resulting standard conversion must have exact-match rank. Failed
  candidates never demand their bodies. Return-only partial ordering shares the
  existing typed directional deduction/cache with a separate conversion context.
- `conversion_functions.cpp` and `call_selection.cpp` record the chosen function
  and object/result conversions, including explicit conversion calls, constructor
  ties and non-template preference. Previously created specializations are not
  independent overloads. Indexed inherited lookup uses alpha-normalized declared
  conversion types for hiding, so renamed template parameters denote the same
  target without letting an instantiated generic hide an ordinary conversion.
- `template_entities.cpp` builds retained member heads left-to-right. Original
  head identities and the enclosing argument frame feed immutable one-binding
  prefix frames. A later non-type parameter's retained canonical type substitutes
  under that frame; it does not reparse syntax or mutate a cached environment.
  Nested template-template heads retain the corresponding source head/frame.
- The parser continues declaration specifiers after an inline class definition
  and suspends template-head parsing mode in special-member bodies/initializers.
  This supports the relevant local declarations and ordinary less-than expressions
  in conversion-template bodies. `constant_addresses.cpp` consumes the selected
  conditional branch conversion before forming a reference address. ABI lowering
  encodes the original conversion return type with canonical specialization args.

Data flow: indexed conversion declaration + destination → local deduction map →
canonical specialization/default/signature facts → selection → ordinary demand →
recorded conversion/address → typed LowIR/ABI. Work follows candidate/type/query,
inheritance and lexical-frame edges, with per-comparison scratch and TU-owned
canonical facts. No semantic string transport, exception-based candidate rejection,
global scan/cache clear or speculative body demand was added. New source ownership
is registered in `dev/frontend_source_sets.mk`. Frozen compiler latency/RSS,
runtime/text size and scaling evidence is in [performance65.md](performance65.md).

Language ownership: N3485 14.8.2.3 [temp.deduct.conv], 14.8.2.4
[temp.deduct.partial], 13.3.3 [over.match.best], and 12.3.2 [class.conv.fct].
One [reference correction](reference-correction65.md) is justified independently
by mandatory C++11 constant initialization and a reduced executable reproducer.
Only that `.ref` changes; sources, success status, comparison rules, bundle and
coverage are preserved. The 15 fixed fixtures include 14 unchanged oracles and
this proved correction, which also required implementation of conversion templates
and constexpr reference-address consumption.

## Validation and concrete boundary

`make test-pa18`: **327/420**, **15 existing failures fixed**, zero new failures.
Root through-PA17: **2609/2609**; file audit passes with three inherited header
advisories. Root through-PA18 remains red at **2936/3029**, so advancement is
blocked. Commands, log hashes, source hashes, exact fixed/remaining fixture lists
and controls are in [loop65-evidence.json](../student.tests/pa18/loop65-evidence.json).

All **51 conversion**, **33 substitution**, **64 ordering/pack** controls and
**4 direct ABI** checks pass. Controls include rejected ambiguity, deleted/private
selection, explicit conversion, reference/cv/qualification alternatives, discarded
defaults/bodies, constructor ties, inherited hiding, retained head dependencies,
native result assertions and exact conversion symbols. All **17** newly accepted
required programs validate LowIR and execute through the supplied backend.
`400-qualified-member-alias-sfinae` intentionally returns **7**; the other 16
exit zero. The separate static-reference reducer records the pinned reference's
failure and the student's required result, with a C++11 proof rather than compiler
agreement as its oracle.

The remaining **93 failures** are **69 status failures and 24 LowIR mismatches**.
Seventeen original status failures now compile, but two retain LowIR differences:
`500-constructor-sfinae-namespace-constant-symbol` needs the established small
constant-array initialization policy, and
`300-conversion-function-template-object-result-copy-init` needs the established
class-result calling convention (direct `obj4` versus `indirect_result`). Their
native results pass; equivalence does not waive the course LowIR comparison.
These are unfinished initialization/result-lowering work, not oracle corrections.

The completed boundary is conversion candidate formation/selection and retained
head identity through recorded conversion consumption. Extending it further now
requires different semantic facts: `400-unnamed-nontype-pack-static-enable-if-default`
uses reference NTTP packs beyond the current integral value model, while
`500-defaulted-nontype-qualified-alias-value` needs correlated outer/inner pack
lanes in `is_constructible<T,U>...`. Other failures require remaining nested/member
result/default owners, braced/explicit constructor deduction, compound assignment,
destructor/cast/access validity, and pointer/reference/static-member NTTP values.
Those changes cross pack/value/expression and initialization/ABI owners; expanding
the completed conversion rules cannot supply the missing facts. They remain
required implementation, with no waived behavior or coverage.

Independent audit remains separate: review return-only ordering and hiding
equivalence, retained head-prefix/cache identity, the constant-reference proof
and ABI encoding. Earlier handoff 63/64 review items remain open. The unchanged
last-reviewed marker does not certify any of this range; reconcile whole-stage
findings and pass the root through report before advancement.
