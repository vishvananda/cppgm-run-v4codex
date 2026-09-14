# PA17 implementation handoff — loop 47

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`

Target: **PA17 full-stage**, incomplete. Clean turn entry:
`ae5087da8794d159b16b716e3c48cf64a271414f`, **242/343**. Current:
**248/343**, **6 original failures fixed**, **95 remain**, **no new failures**.
Previous implementation turn: **progress** (committed 207→242, preserved in
`handoff-loop46.json`); this turn revalidated the worktree and failure set.
Fixtures, references and comparison/status rules are unchanged. This records an
implementation handoff; independent audit and whole-stage completion remain open.

## Completed group: out-of-class partial-owner definitions

- `template_definition_owner` and `class_pattern_selection` share a structural
  owner-shape key: canonical parameter kinds/packs and normalized argument tuple,
  indexed beneath the primary entity. A definition names its exact declared
  primary or partial pattern. Parameter spelling, nested type arguments, values
  and packs do not become textual keys. The new source is registered.
- `template_definition` records paths beneath the selected pattern, including
  nested non-template classes. Member demand consumes the selected partial's
  deduced tuple. Definition applications remain indexed by specialization/source;
  no unrelated specialization traversal, parsing replay or eager body demand is
  added. Renamed source heads retain their own parameter slices.
- `template_checks` recognizes current-owner aliases against the selected
  partial's normalized arguments. This preserves dependent return and parameter
  types, signature matching, overload separation and rejection of duplicate or
  mismatched definitions. Primary and partial owners keep distinct nested paths.
- `template_type_facts` gives a renamed partial head an explicit selected tuple.
  Frame identity includes specialization, parameter slice/count, parent and tuple.
  Ordinal argument lookup is O(1) within a head; nested lookup follows parent
  frames. This replaces the initial per-parameter overlay chain before handoff.

Data flows from immutable parsed declarations through canonical owner selection,
retained member signatures and source heads, then ordinary demand/substitution
and typed LowIR lowering. Shape formation visits the head/argument graph;
completed owner lookup is O(1) average. Demand follows actual definition edges.
Flat indexes, retained IDs and immutable frames live for the TU; normalization
scratch dies on return. No owning syntax copy, semantic text transport, global
retry, output delegation or optional optimization was introduced.

The six fixed course cases cover selected nested paths, member typedefs, nested
argument types, dependent returns, current-specialization iterator aliases and
binding the partial's deduced arguments. Explicit controls extend this to packs
(including empty), value parameters, static storage, overloads, late bodies,
namespace ownership, constructors/destructors, copy assignment, explicit
instantiation and dormant invalid bodies, plus required rejection edges.

## Validation and performance

`make test-pa17`: **248/343**. `make test-report-through-pa16`: **2266/2266**.
`make test-report-through-pa17`: **2514/2609**, all failures in PA17. Reports ran
serially. File audit passes with the same three inherited header-division warnings.
Explicit controls pass: **13 native / 5 rejection** definition controls; prior
**21 native / 13 rejection / 4 LowIR** member controls and **24 native / 10
rejection** entity controls. [Evidence](../student.tests/pa17/handoff.json) and
[verifier](../student.tests/pa17/verify_handoff.py) bind source/fixture integrity,
logs, failure-set delta, controls and frozen measurements.

[Current performance evidence](definition-performance.md) records compiler
latency/peak RSS and native runtime/text size, A/A noise and ABBA observations,
frozen flags/binaries/inputs and checked equivalent outputs. Newly supported
partial definitions are measured separately from the rejecting entry binary.
PA17/O0 has no mandated numerical ceiling and needs no optional optimization.
Historical percentage/scaling targets are diagnostics, not added gates; all
measurements and mandated limits remain preserved. Native optimization, MIR and
self-hosting are later-stage owners. Historical [loop 45](entity-performance.md)
and [loop 46](member-performance.md) evidence and handoff JSON remain retained.

## Remaining implementation and concrete boundary

The **95 failures** are unfinished implementation, not independent review:

| Group | Owning implementation / next data flow |
|---|---|
| Multiple heads, nested member/alias templates, partial/late enclosing definitions | Source declaration binder and prototype index must retain each nested template head and its enclosing owner before member demand can consume them. |
| Friend templates, access and ADL | Friend entity relationships must feed indexed lookup and access checks. |
| Dependent typename/template obligations | Definition-time source-name checking must retain introducers and current-instantiation context, including dormant nested bodies. |
| Remaining pack, variable-template and dependent lookup interactions | Argument, specialization and substitution owners must preserve source scope and structural matches. |
| Required LowIR differences | Cleanup, scalar/constant storage, initializer and emission owners must preserve required representation and demand. |

This handoff closes definitions whose owner head selects a primary/partial class,
including nested **non-template** members and special-member/storage edges. All
observed failures in that group are fixed. It does not close multiple independent
template heads: `bind_template_declaration` and `index_template_members` currently
skip nested `Kind::Template` declarations in source class patterns. Their later
concrete declarations cannot supply definition-time heads, aliases or signatures.
A complete fix requires retaining those nested declarations with distinct head
and enclosing-owner identities, then composing their substitution frames and
late-definition demand. Extending the selected-tuple patch cannot supply these
missing facts. Further work crosses that source-declaration ownership boundary;
no remaining fixture or architectural requirement is waived.

## Review ledger

`aea1f09b`: structural partial-owner matching, selected nested paths and aliases.
`3bef03e0`: selected-tuple frames and performance harness. Both **unreviewed**.
Prior `fabfe92e`, `ca67b17d`, `5ea7f7f6`, `622e486a`, `8deaa931`, `da8594b7`,
`f6dbacde` remain **unreviewed**. Review markers above are unchanged.
Independent audit must examine complete structural head/argument keys, partial
selection/coverage, definition-owner cache publication, frame/source reuse,
access-recipe keys and naming-exemption boundaries, and declaration/body/storage
demand lifetimes across the full history. These are review obligations distinct
from the 95 known implementation failures; neither is waived.
