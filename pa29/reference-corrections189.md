# PA29 reference correction 189: template definitions and declared values

Correction ID: **pa29-189-template-demand**. The three original inputs are
unchanged. Only their `.ref.exit_status` changes from `EXIT_SUCCESS` to
`EXIT_FAILURE`. Empty `.ref`/`.ref.stdout`, discovery, timeout, diagnostics and
comparison rules remain unchanged. This is a correction to the checked-in
reference layer, not an implementation of implicit library definitions.

The observed reference bundle is pinned by
[`reference-binaries/manifest.tsv`](../reference-binaries/manifest.tsv): source
revision `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, bundle SHA-256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`, `cppgm++`
SHA-256 `e32741e2504f7c75a91abdf5cea0d5fc0b56eb43deb503d2ffbddbcc2f854f70`.
That binary/bundle is not rewritten. Regenerating these sidecars from that
uncorrected binary would reintroduce its errors; retain this documented local
correction until a corrected upstream bundle exists.

## Rules and proof

The assignment's C++11 reference is the checked-in [N3485 text](../doc/n3485.txt),
also available from [WG21](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3485.pdf).
Relevant rules are §14.7.1 [temp.inst]/1, /5, /7 (class completeness and a required
instantiation of an undefined template), §5.3.1 [expr.unary.op]/9 (boolean
negation), and §7 [dcl.dcl]/4 (a false static assertion is ill-formed).
These are semantic proofs; host/reference observations below are corroboration.

| Original compile fixture | Reduced personal input | Derivation |
|---|---|---|
| `600-hosted-nothrow-default-constructible-shorthand.t` | [forward-member-reject.cpp](../student.tests/pa29/controls189/forward-member-reject.cpp) | The source only forward-declares three class templates. Looking up `value` requires their member lists; [temp.inst]/1 and /5 require completion. No definition or explicit specialization exists, so /7 makes the required instantiation ill-formed. Constructor properties of the argument type cannot create a member in an undefined template. |
| `700-hosted-char-traits-primary-conversion-shims.t` | [forward-nested-reject.cpp](../student.tests/pa29/controls189/forward-nested-reject.cpp) | Instantiating `probe<UChar>` demands `char_traits<UChar>::int_type`, which requires the undefined primary's member list. The same /1, /5, /7 rules apply. Neither an implicit conversion of `UChar` nor the name `char_traits` declares that nested type or either conversion function. |
| `700-hosted-nothrow-invocable-cache-default.t` | [defined-false-reject.cpp](../student.tests/pa29/controls189/defined-false-reject.cpp) | The source **defines** `__is_nothrow_invocable<F,Arg>` to inherit `integral_constant<bool,false>` for every argument pair and provides no specialization. Its inherited `value` is false; `__not_` makes it true; the outer `!` makes the static assertion false. [expr.unary.op]/9 and [dcl.dcl]/4 therefore require rejection. `Hash::operator()` being `noexcept` does not change the declared initializer. |

The `std` and double-underscore spellings require an explicit qualification of
this proof: §17.6.4.2.1 [namespace.std] and §17.6.4.3.2 [global.names] reserve
those names. Thus an ordinary-name reducer alone does not establish a portable
diagnostic obligation for arbitrary reserved-name programs. Here the **course
contract** also matters: [spec §10](../spec.md) forbids recognizing library type
spellings/expected answers, and §§2/4/6 require declaration-owned facts and
language-required demand. The [PA29 handout](README.md) calls for shared language
behavior, not implicit definitions of these standard-library templates or a
builtin overriding a declared class primary. Its builtin registry does not
declare `__is_nothrow_invocable` a type trait. Accepting these cases by inventing
members or overriding an explicit false initializer would violate that contract.
There is no required name-based extension that defeats the core-rule derivation.

No header is included by these fixtures, and none has an `.env`, `-include`, or
language-mode sidecar supplying the missing definitions. The fixture's comment
about an undefined hosted primary is descriptive, not a class definition.

## Positive coverage and observed behavior

`python3 student.tests/pa29/check189.py OUT` runs **180 commands**. The three
original inputs are rejected by the student compiler, GCC and Clang. Each
ordinary-name reducer is also rejected by the pinned reference compiler; that
reference accepts all three original inputs. The change of spelling therefore
exposes its non-general behavior; compiler consensus is not the proof above.

All original assertions are also checked with real definitions supplied through
explicit personal `-include` headers, without editing the fixtures:

- [trait-definitions.h](../student.tests/pa29/controls189/trait-definitions.h)
  defines the three class templates using the actual constructibility builtin.
- [character-definitions.h](../student.tests/pa29/controls189/character-definitions.h)
  defines the primary's nested type and conversion functions in C++ source.
- [invocation-definition.h](../student.tests/pa29/controls189/invocation-definition.h)
  defines a partial specialization computing `noexcept` from an actual call.

All three original inputs then compile, link and run with all three compilers.
Additional positive controls cover primary and explicit specialization identity,
throwing/deleted/move constructors, false primaries despite nothrow callables,
true partial specializations despite throwing callables, pointers preceding a
definition, missing/private-member SFINAE, and dormant invalid member bodies.
A demanded invalid body is rejected. Five positive LowIR outputs validate;
symbol/LowIR inspection proves dormant bodies are not emitted.

[Control evidence](../student.tests/pa29/evidence189/controls.json) and
[pinned-reference observations](../student.tests/pa29/evidence189/reference-observations.json)
retain commands, statuses and diagnostics. No reference compiler contributes to
the implementation or generated program. The original three failures do not warrant a library-name workaround. Extending
the controls found and fixed a separate shared defect: static assertions now
select and execute contextual boolean conversions, validate their message
literals, and retain message text in failure diagnostics. These controls cover
false, nonconstant, deleted, private and ambiguous conversions, template demand,
pointers and encoded messages. The corrected sidecars remain subject to
independent Ralph review.
