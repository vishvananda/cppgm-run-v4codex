# PA17 implementation — loop 46

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`

Loop 46 entry: `4bbb712abc6047e4821f850116aa0465df1eedbd`, clean,
207/343 (136 failures). Review markers remain unchanged. Work in progress:
member-template declaration/signature environments, specialization body ownership,
and explicit-instantiation demand. Owners are `template_declaration`,
`template_instantiation`, `explicit_instantiation` and the ordinary declaration
builder; data flows from retained heads/signatures through canonical entities and
parent-linked substitution frames to existing body demand and typed lowering.
Work must follow each head, signature and demanded entity; no global retry or
grammar replay. Validate the affected required fixtures, explicit personal
controls, all earlier PAs and file audit. Freeze entry/final binaries and measure
compiler latency/RSS plus common executable runtime/text under spec.md's PA17/O0
acceptance. Nested class/member source retention and out-of-class partial-owner
attachment remain required implementation; existing review questions below are
separate and preserved.

Loop 46 checkpoint: **241/343**, all **2266** earlier fixtures pass; **34**
entry failures fixed with **no new failures**. Member head/class environments,
constructor/signature/body facts, explicit specialization and instantiation
ownership now share ordinary canonical entities and demand. Dependent qualified
type accesses retain source-scope obligations separate from canonical type
identity, validated per complete substitution frame (including pack lanes).
Source-subtree summaries skip bodies; access exemptions end before demanded
declarations. Controls: 16 native, 9 rejection and 3 LowIR ownership probes;
the prior 34 entity controls pass. Next: finish related redeclaration/default
and demand edges, then freeze/measure and run final handoff checks. Performance
evidence and the old handoff sections below are historical until refreshed.

Target: **PA17 full-stage**, still incomplete. This is an implementation
handoff, not independent audit acceptance. Entry was clean at **132/343**.
Current result: **207/343**; **75 original failures fixed**, **136 remain**,
**no new failures**. All 343 fixtures and their comparison/status contracts are
unchanged; no reference corrections were made.

## Completed behavior and spec alignment

- **Template argument identity:** `template_entities`, `template_arguments`,
  `template_class` and `dependent_type` retain nested template parameter heads,
  canonical head shapes, template entities and alias application by entity/typed
  tuple. Alias entities belong to their declaring namespace/class; parameter
  environments are separate. Matching checks kinds, arity, dependent non-type
  types, packs, defaults and access. Namespace-qualified identities remain distinct.
- **Structural selection:** `template_deduction` and `class_pattern_selection`
  distinguish class patterns from call deduction, preserve array bounds/cv,
  function qualifiers and adjusted parameters, repeated/nested/symbolic packs,
  fixed prefixes and constant value patterns. Selection records its definition
  and arguments once. Immutable candidate-pair ordering has its own cache;
  omitted-default positions belong to the actual match and are computed only
  when viable candidates compete. Crossed coverage remains ambiguous.
- **Retained substitution:** definition-time argument facts feed immutable
  substitution frames; only dependent facts are rebuilt. Pack lists own ellipsis
  substitution, including empty packs and dependent constant expressions.
  Reapplying a dependent cast preserves its canonical identity. Class/base
  dependence includes non-type parameters and sizeof-pack expressions.
- **Source boundary:** split `>>`, qualified function parameters, empty function
  types, and direct-member decltype preserve their source meaning. No grammar
  replay, rendered semantic keys, global retries, fixture dispatch or output
  delegation was added. Existing typed semantic-to-LowIR lowering is reused.

Work follows parameter/argument edges and viable candidates. Deduction and
substitution are linear in consumed shapes; coverage sorting is O(n log n) in
matched positions, and selection compares candidate pairs required for ordering.
Canonical head/source facts and alias results use TU-owned flat identity indexes;
match vectors and coverage indexes die after selection. Published source facts
are keyed by source identity only for original occurrences; substitutions use
complete frame identity. New sources are registered in frontend_source_sets.mk.

## Validation and performance

`make test-pa17`: **207/343**. `make test-report-through-pa16`: **2266/2266**.
Through PA17 reports **2473/2609**, with failures confined to PA17.
File audit passes with three inherited header-division warnings. Explicit
`student.tests/pa17/entity_controls.py`: **24 native / 10 rejection** controls
pass, including every newly exposed regression. [Handoff evidence](../student.tests/pa17/handoff.json)
and [verifier](../student.tests/pa17/verify_handoff.py) bind source/fixture hashes,
required logs, the exact failure delta, controls and frozen measurements.

[Performance evidence](entity-performance.md) records A/A and ABBA observations,
compiler latency/RSS, native runtime/text size and identical common outputs.
Historical campaigns remain preserved. PA17/O0 mandates no numerical latency,
RSS or text ceiling; inherited percentage/scaling targets remain diagnostics,
not exit gates. No optional optimization or executable-speed benefit is claimed.
Native optimization/MIR/self-hosting remain later-stage owners.

## Remaining implementation and handoff boundary

The 136 failures remain required work: nested/member templates and aliases need
source declaration retention and concrete enclosing-owner publication; partial
owner/out-of-class definitions need correct attachment and late demand; friends,
ADL/access/hiding, typename/template obligations, and explicit instantiation need
their declaration/emission state. Some completed semantic cases still need LowIR
output fixes (for example ordinary wide scalar assignment in the forward alias
fixture). These are implementation gaps, not review questions or waived tests.

The next coherent group crosses nested declaration, access, substitution-frame
and body/emission lifetimes. Extending the argument matcher cannot supply those
missing owner facts. This handoff finishes structural argument selection and
substitution, including the related defects found during validation; it does not
claim all alias/member interactions or the whole assignment are complete.

## Review ledger

`622e486a`: heads/aliases/shape deduction; `8deaa931`: retained arguments and pack
selection; `da8594b7`: alias ownership/access regressions; `f6dbacde`: dependent
conversion identity and demand-scoped coverage. All remain **unreviewed**.
Independent audit must examine complete keys/source-frame reuse, structural
ordering/coverage and enclosing access environments across this full history.
Review markers above are preserved; implementation checks do not replace audit.
