# PA17 implementation — loop 47

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`

Target: **PA17 full-stage**, still incomplete.

Loop 47 entry: clean `ae5087da8794d159b16b716e3c48cf64a271414f`, **242/343**,
**101 failures**. Active group: retained out-of-class definition owners. Semantic
ownership: `template_definition` selects the canonical primary/partial owner;
`template_checks` normalizes signatures; source head frames feed ordinary member
demand. Extend to related nested/multiple-head definitions where those same facts
suffice. Match indexed structural owner shapes, with work proportional to the
head/argument graph and actually demanded definitions; do not replay parsing or
scan unrelated specializations. Validate required fixture deltas, explicit native
and rejection controls, earlier stages, file audit, frozen A/B latency/RSS and
runtime/text measurements. Prior handoff and review obligations below remain
historical evidence until refreshed.

Previous handoff: Entry was clean at
`4bbb712abc6047e4821f850116aa0465df1eedbd`, **207/343**. Current result:
**242/343**; **35 original failures fixed**, **101 remain**, **no new failures**.
All fixtures, references and comparison/status rules are unchanged. This is an
implementation handoff, not independent audit acceptance or stage advancement.

## Completed behavior, ownership and spec alignment

- `template_declaration` and the ordinary declaration builder give member
  templates canonical class-owned entities and distinct parameter environments.
  Qualified definitions retain lexical return-type lookup and class lookup after
  the declarator. Equivalent alias/function heads share identity; renamed heads
  preserve dependent defaults without replacing an established alias definition.
- `template_instantiation`, `construction` and member demand retain static/cv/ref,
  access, constructor and initializer/body facts on selected specializations.
  Constructor deduction uses the existing candidate engine; qualification and
  structural template ranking preserve the implicit copy constructor. Explicit
  member specializations own their bodies, including declarations preceding the
  primary definition. Static-local initializer relocations demand their targets.
- `explicit_instantiation` resolves the actual function/member entity, validates
  namespace and operator rules, records declaration/definition state, and feeds
  ordinary demand plus typed lowering retention metadata. Extern declarations
  suppress non-inline members, including non-template nested classes; inline and
  implicit members remain available. Explicit specializations make subsequent
  instantiations inert. No blanket body instantiation or global retry was added.
- `template_type_access` retains source-scope access obligations separately from
  canonical type identity. Source subtree summaries exclude dormant bodies;
  checks use complete substitution-frame/recipe keys and pack lanes, with
  in-progress/success/failure states. `dependent_type` shares selected-member
  identities between type substitution and access checking; only complete class
  scopes publish cached lookup results. Explicit-instantiation naming exemptions
  end before demanded class declarations are checked. The new source is registered.

Work follows head parameters, related candidates, source type obligations and
actual storage/demand edges. Head normalization/default merging is O(head size);
source summaries are built once, access checks scale with relevant recipes and
lanes, and each completed owner/name lookup is indexed. These TU-owned flat
indexes and immutable frames have TU lifetime; traversal/default scratch dies at
operation return. No parser replay, rendered semantic keys, new owning syntax
copies, output delegation or optional optimization was introduced. Existing
structural argument/partial-selection work from loop 45 remains intact.

## Validation and performance

`make test-pa17`: **242/343**. `make test-report-through-pa16`: **2266/2266**.
`make test-report-through-pa17`: **2508/2609**, all failures in PA17. Root reports
were run serially because concurrent invocations share report counters.
File audit passes with the same three inherited header-division warnings.
Explicit personal controls pass: **21 native / 13 rejection / 4 LowIR** member
controls, plus the prior **24 native / 10 rejection** entity controls.
[Handoff evidence](../student.tests/pa17/handoff.json) and the
[verifier](../student.tests/pa17/verify_handoff.py) bind source/fixture integrity,
required logs, the exact failure delta, control results and frozen measurements.

[Performance evidence](member-performance.md) records frozen A/B flags, hashes
and inputs, A/A calibration, ABBA observations, compiler latency/RSS and native
runtime/text size. Common outputs are checked for equivalence; newly supported
member workloads are measured separately from the rejecting entry binary.
PA17/O0 imposes no numerical latency/RSS/text ceiling and requires no optional
optimization. Historical percentage/scaling targets remain diagnostics, not
extra gates; measurements and evaluator limits remain preserved. Native
optimization, MIR and self-hosting remain later-stage owners. Loop 45's
[measurements](entity-performance.md) and
[handoff record](../student.tests/pa17/handoff-loop45.json) remain historical.

## Remaining implementation and concrete handoff boundary

The **101 failures** are unfinished implementation, not review questions:
multiple template-head declarations and nested member/alias source retention;
partial-owner and late out-of-class definition attachment; friend-template
relationships/access/ADL; dependent typename/template obligations; remaining
pack/variable-template interactions; and required LowIR output differences.
In particular, nested extern-member suppression and static-local relocation
ownership now work, but the member-coverage and static-table fixtures still have
LowIR cleanup/representation differences. No failed fixture is waived.

This handoff closes member-template behavior with concrete enclosing owners,
its redeclaration/default and constructor edges, and explicit-instantiation
selection/demand. The remaining multi-head cases need a retained declaration
graph that pairs each head with the selected enclosing owner, including partial
specializations, before publishing nested templates. Extending the concrete
head overlay cannot supply those source facts. The remaining LowIR differences
belong to cleanup, scalar/constant storage and initializer lowering. These are
separate implementation owners beyond this completed semantic group; the
handoff does not assert whole-stage completion.

## Review ledger

`fabfe92e`: member ownership, constructors, explicit instantiation and access
obligations. `ca67b17d`: defaults/alias equivalence, nested suppression, static
relocation demand and completed selected-member lookup. `5ea7f7f6`: enforce definition-scope
access for alias instantiation and keep naming exemptions out of access fact keys.
All three are **unreviewed**.
Prior `622e486a`, `8deaa931`, `da8594b7`, `f6dbacde` remain **unreviewed**.
Independent audit must examine the full history's argument/head identities,
structural ordering/coverage, source/frame reuse, access-recipe keys and naming
exemption boundaries, and declaration/body/storage demand lifetimes. These are
review obligations distinct from the 101 known implementation failures; neither
is waived. The review markers above remain unchanged.
