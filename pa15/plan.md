# PA15 implementation handoff

Stage base commit: `8000f3c8ef4647d57f2c0775192585f14cab33d8`
Last reviewed commit: `8000f3c8ef4647d57f2c0775192585f14cab33d8`

Target: **PA15 full-stage**, O0 typed LowIR. This is a validated incomplete
implementation handoff; whole-stage implementation and independent review remain
required. Entry **56/177**, handoff **114/177**: 58 original failures resolved,
63 remain, no previously passing case regressed. Coverage stays 177 fixtures.

## Design and spec alignment

| Owner | Completed data flow and complexity |
|---|---|
| Constant/query facts | Shared signed quotient/remainder and shift checks; integral functional casts; ordinary multicharacter literals at the language entry; function-only inline metadata. Parsed source -> typed expression/query -> cached constant. PA2 retains its explicit one-code-point view. |
| Template argument identities | Disjoint compact TypeId/QueryId arguments; concrete values canonicalize by unqualified type and bits, dependent values retain query DAGs. No fake Type nodes or text keys. Expected O(arguments + new dependent nodes), O(arguments) reuse normalization. |
| Heads, defaults, signatures | Integral parameters, dependent parameter types, class/function defaults, renamed out-of-class heads, non-narrowing/kind filtering, canonical member signatures. Immutable frame -> substituted query/type -> ordinary declaration/body; existing flat indexes and TU lifetimes. |
| Lookup, source and ABI consumers | Qualified value/type arguments, functional casts and relational angle classification without grammar replay; type deduction follows explicit base edges; lowering consumes concrete argument identities for ABI. Source bodies and class completion remain separately demanded. |

[Personal controls](../student.tests/pa15/value_arguments.py) exercise 26 value
groups: equivalence, defaults, enum identity, narrowing and wrong-kind rejection,
renamed heads, source scope, member aliases and prvalue rules. Ten additional
[constant controls](../student.tests/pa15/constants.py) cover both evaluator paths.

## Remaining implementation and boundary

| Required group | Owner/data flow still to implement |
|---|---|
| Packs and partitions | Explicit pack boundaries in specialization keys; lockstep and nested expansion of retained patterns into declarations, calls, bases and initializers; sizeof... and literal packs. Work must follow produced elements. |
| Explicit specialization/refresh | Indexed primary/argument selection and narrow reverse dependencies; replace stale primary facts without global retry. Includes inheritance, static members and virtual/lifetime effects. |
| Broader constant and source obligations | Required fixture cases for constant function/conversion execution, string element evaluation, variable templates and unused ordinary member assertions. Need execution/binding and declaration-obligation owners beyond scalar query folding. Handout out-of-scope language does not waive checked fixtures. |
| Other declaration/query/LowIR cases | Remaining function-pointer parsing, static-member query/operator forms and aggregate contract shapes. Exact pending fixture list is retained in the handoff record. |

The coherent completed group is scalar integral argument identity, normalization,
substitution and its consumers. Related member/default/signature/base cases were
extended before stopping. The next groups need different missing facts: pack
partition/expansion identity, specialization-selection dependencies, constant
execution frames, and early body-obligation scheduling. Extending scalar query
folding alone cannot supply them; continuing here would open another incomplete
semantic owner. These are implementation work, not questions waived to audit.

Independent review remains open for whole-stage spec conformance, complete keys,
source/instance sharing, demand and invalidation boundaries, and performance
acceptance. The review markers above are intentionally unchanged.

## Evidence and ledger

| Increment | Commit | PA15 |
|---|---|---:|
| Entry plan | `053af5e3` | 56/177 |
| Constant operators/casts | `cdbfeb8a` | 64/177 |
| Typed value arguments and related consumers | `01b543ed` | 114/177 |

Required final checks: `make test-report-through-pa14` **1935/1935 pass**;
`make test-pa15` **114/177**, exit 2, with strict original-failure reduction;
`perl scripts/cppgm_file_audit.pl --stage pa15 --paths dev/src` **pass**, three
inherited header warnings. Personal controls run explicitly. No fixture,
reference, bundle, harness or comparison rule changed. Two overlapping interim
reports had shared counters; final commands ran serially.

[Handoff proof](../student.tests/pa15/handoff.json) records exact failures, fixture
tree identities and command hashes. [Performance](../student.tests/pa15/performance.md)
retains 336 observations, A/A and ABBA, compiler latency/RSS/text, native runtime
and text, exact common output parity and linear source/key counters. No optional
optimizer or new numerical gate. Raw evidence: `$RALPH_ARTIFACT_DIR/pa15-value/`.
Historical PA14 preflight LowIR was losslessly gzip-compressed after hash checks
to recover scratch capacity; `historical-compression.json` records every file.
