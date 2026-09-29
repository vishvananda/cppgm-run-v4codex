# PA19 implementation plan

Stage base commit: `e5f4c3ed78972c8d161671d145bf525cb99033f4`.
Last reviewed commit: `e5f4c3ed78972c8d161671d145bf525cb99033f4`.
Target: PA19 full-stage. Phase: implement; validated incomplete handoff 91.
Entry: 397/423 required cases; handoff: **407/423**, 16 original failures remain.
All 423 source inputs are byte-identical to entry; no coverage/comparison reduction.

## Design/spec alignment and completed group

Preserve streaming syntax, canonical typed identities, immutable substitution
frames, lazy fact demands and direct typed LowIR. No production reference calls,
text transport, new source unit, cache, global search/retry or optional transform.

| Owner | Data flow and completed behavior | Work bound / validation |
|---|---|---|
| Parser name/type classification | Function type arguments retain ellipses; friend template-ids retain indexed category; ADL-only explicit template-id call syntax follows the course contract | Bounded delimiter lookahead; no second grammar parse. Function/relational/ADL positive and negative controls |
| Type/template substitution | Outer template identity → canonical TypeId/EntityId → one occurrence frame → qualified alias/result; inner function arguments are not substituted twice | Existing complete-key substitution caches; linear dependent argument projection. Alias forwarding, default queries and dormant-method controls |
| Class declaration/lookup | Unqualified elaborated declaration → namespace/block tag identity; fixed/defaulted class arguments → complete deduction sequence, including base alternatives | Lexical/base edges and argument sequence work; namespace identity, fixed-head type/value pack and rejection controls |
| Signature and ordering | Actual expansion parameters distinguish packs from comma-optional varargs; empty-tail call ordering projects unmatched trailing expansion before directional deduction | Existing cached ordering owner; linear sequence projection. Constructor/function/query permutations, fixed-pack tie and inherited ordering controls |
| Explicit conversion | Related reference cast → existing glvalue binding fact → typed LowIR, without user conversion search | Existing relation/path checks; identity, base adjustment, unrelated conversion and rejection controls |

The implementation owns semantics and LowIR; the supplied backend only executes
validation outputs, as required before PA24. Later native optimization/debug and
self-hosting requirements remain in their owning stages.

## Remaining groups (requirements remain open)

1. **Member variable-template facts: two compilation failures.**
   `declare_variable_template` rebuilds projected initializer queries;
   `QueryKind::QualifiedValue` finds a member without applying its template
   arguments. Correct flow must retain the original initializer query with its
   composed environment, select the variable specialization, and separately
   demand declaration, initializer and storage. Class-valued results also need
   constant-object emission. Keep complete-key memoized success/failure and
   compact SFINAE rejection; avoid eager initialization. Validate both failing
   leaf-SFINAE/class-value cases plus defaults, dormant definitions, repeated
   specializations and native storage identity.
2. **Fourteen LowIR mismatches: implementation or independently proved oracle
   corrections still required.** Group by owning fact: five automatic-array
   initialization cases (PA16 readonly/copyobj contract); five static-member
   demand/constant-storage cases; discarded-reference consumption; reference
   constant initialization; explicit-specialization instantiation metadata;
   function-pointer NTTP constant conversion. Historical PA18 corrections are
   leads, not proof. Each needs a reducer and standard/LowIR contract review,
   then an implementation repair or independently reconstructed pinned oracle
   revision. Keep the existing comparisons. Exact cases are in the
   [handoff manifest](../student.tests/pa19/handoff91.json).

The completed group was extended through alias identity, defaulted argument
packs, function-result queries, elaborated lookup, casts and constructor ordering.
Further related work now crosses into a separate variable declaration/initializer/
storage state owner and constant-object lowering; patching another projected
query would leave that behavior group incomplete. This is the concrete handoff
boundary, not a minimum-progress cutoff. No known correctness/spec defect in the
completed group is deferred as an audit question. Whole-stage correctness,
architecture and performance audit remains an **independent review obligation**;
it does not replace either unfinished implementation group above.

## Validation and performance

- `make test-pa19`: **407/423**, exit 2 (two rejections, fourteen mismatches).
  Ten entry failures fixed, no new failures; one proved reference revision below.
- Required prior report: **3029/3029**, exit 0, plus 22 focused PA10–PA12 properties.
  The through-PA19 report before the reference revision retained all prior passes
  and reported 406/423; final stage check covers that revision.
- File audit for `pa19 --paths dev/src`: exit 0, three inherited header advisories.
- Explicit personal controls: **42/42** (32 checked native, ten required rejects),
  versus **17/42** on the entry binary; inherited ordering controls **64/64**
  (53 checked native, eleven rejects). Defaulted-pack reducer executes with exit 0;
  independent oracle reconstruction passes.
- [Frozen performance evidence](performance91.md): 22 completed workloads,
  576 observations plus 68 warmups; compiler latency/RSS and checked native
  runtime/payload sizes, A/A, ABBA and all spreads preserved. Nine shared inputs
  have exact LowIR/native equality; thirteen report newly correct final-only
  costs. Compiler text +4,352 bytes (0.218%). Largest new workload latency scales
  unevenly despite proportional measured work; diagnostic and shared comparison
  retained. No speedup claim. PA19/O0 stage-scoped acceptance passes; inherited
  +15%, +16 MiB and 5.5× targets remain diagnostics under spec §9, not invented
  exit gates. No mandated limit, correctness rule or coverage is waived.

## Handoff ledger

- `1747113b`: entry markers, ownership groups and performance protocol.
- `dbe5e97c`: first type identity/substitution/deduction/conversion increment;
  25 controls pass; PA19 403/423 and prior template stages 1254/1254.
- `57e27df2`: related elaborated lookup, ADL syntax and empty-tail ordering;
  final implementation frozen; 42 composition and 64 ordering controls pass.
- `4dd6e737`: [defaulted-pack oracle proof](reference-correction91.md), reduced
  cardinality/identity checks and independent revision transformer. Nine trailing
  arguments include defaults per C++11; fixture input and comparison stay intact.
- Final evidence commit: frozen observations, scope boundary, coverage/check
  hashes and this compact plan. Changes committed and clean before handoff.
  Implementation handoff is complete; **PA19 remains incomplete** pending the
  two remaining groups and independent whole-stage audit. Preserve both review
  markers above during further implementation. Run the through-PA19 report as
  a full passing exit check before advancing.
