# PA9 completion audit

Scope: normalized ABI facts and reusable typed Itanium naming. PA9 neither
parses C++ nor produces LowIR, objects or executable code. The earlier
frontend/LowIR implementations are unchanged. The unused owning-record API in
`abi_mangle.h` has been replaced with the compact graph API; there were no
implemented callers. Existing vocabulary headers and inherited attribution
remain. Later lowering constructs `Graph`/`Target` directly and calls `mangle`.

## Ownership and data flow

- `Graph` owns an identifier byte arena, canonical nodes, variable-child slices
  and flat open-addressing tables. Published nodes expose const access; IDs
  survive pool growth. No node owns a pointer, allocation or child vector.
  Interning hashes typed fields and child IDs, never rendered names. Tag order,
  adjacent cv wrappers, equivalent integral values and named-type spellings
  canonicalize before substitution identity is used.
- The fact adapter reads one line into words, resolves case-local binders by
  interned name, and constructs graph facts directly. Definitions are not
  retained as another record/tree representation. Duplicate binders and invalid
  references/indices reject the case. Compact modifiers are peeled by offsets
  and built in reverse, avoiding copied suffixes. Whole-file parsing is an
  explicit inspection API; the command-line path releases graph facts per case.
- `Encoder` owns one append-only symbol and sparse substitution slots. It does
  not initialize or search an array proportional to unrelated graph contents.
  Prefixes are entered in grammar order, cv/type wrappers after their children,
  function-template prefixes before arguments, and results before parameters.
  Ordinary builtins, standard substitutions and explicitly nonsubstitutable
  parameter facts do not acquire slots.
- Local contexts encode the enclosing function in the containing symbol's
  substitution state. External entity literals create a separate encoder,
  preserving external-name independence and the outer sequence. Raw contexts
  and external symbols are only the explicit normalized escape forms permitted
  by the handout; they are never reconstructed from production mangled text.
- `Function` carries semantic terminals, conversion types, parameters, result,
  tags, context, template-prefix eligibility and member/nonmember shape. The
  latter is explicit for production and can be specified in normalized inline
  facts. Thunk targets retain fixed and virtual result adjustments separately.
- Dependent expressions are canonical DAG nodes with typed operations and
  ordered operands. Literal bits, trait types, array bounds, member owners and
  template arguments participate in identity. Encoding traverses only demanded
  operands; equivalent expressions share a type substitution, distinct ones do
  not. Unknown operator spellings are rejected, not copied into ABI output.
- Fact serialization is an optional view, with memoized definitions. Linear
  type chains use an iterative walk; touched definitions reset between cases.
  The command-line encoder never invokes the serializer or reparses its output.

## Representative traces

`ns::Box<int>` is a Name(ns), Name(Box,parent=ns), builtin(int), type argument
and Template(prefix=Box,args=[int]). Function parameters referencing the same
Template ID emit it once, then its numbered substitution. Adding 100,000
unrelated names leaves the symbol's visited-node count unchanged.

A dependent member followed by a binary expression retains the owner type ID,
member source ID, operation enum and operand IDs. A Decltype type points to the
canonical expression. Equal literals and operands find the same Decltype ID;
changing the literal or a trait operand produces a new ID. The fixtures compare
both equivalence and distinction, including substitutions created inside casts.

A tagged class template canonicalizes its tags on the unqualified template
prefix, before `I...E`. It cannot substitute a previously emitted untagged
terminal. A builtin standard substitution may carry appended tags, as specified
in the local ABI reference. Personal tests exercise both rules beyond the
checked-in tagged named-type fixtures.

A covariant virtual-result thunk retains this adjustment, returned-object fixed
adjustment and vcall slot. The encoder emits call-offset grammar directly; the
same typed target is exercised by a direct API test and serializer roundtrip.

## Complexity, limits and evidence

Canonical lookup is expected O(1) for fixed keys and O(arity) for a new variable
shape. Reading/interning is proportional to source bytes and graph edges;
encoding is proportional to consumed facts and output. Tag sorting is local
O(tags log tags). There are no fixed-point passes, global retries, optimization
levels, generated-code growth or persistent/process-global mutable caches.
Graph pools release together; encoding scratch releases per name. Recursive
branching grammar uses a checked 1024-level nesting guard; common modifier
chains are iterative and explicitly exercised at 20,000 levels.

`--stats` times existing parsing and encoding only when requested and reports
pool storage, interning/probe counts, substitutions and emitted nodes. External
peak-RSS measurements include adapter/encoder/output overhead beyond the graph
counter. [Performance evidence](performance.md) records compiler wall/RSS and
compiler text growth, with executable runtime/text explicitly inapplicable.

The checked-in course fixtures and references are unchanged. Required evidence
is the root PA9 test, cumulative report, file audit and the explicit personal
suite (all 117 fixture status/output checks and serialization roundtrips,
direct graph tests, valid/invalid probes, batch ordering, ASan/UBSan). The compact
plan records final results and commit provenance; review markers are preserved
for Ralph's subsequent independent review.
