# PA19 implementation plan

Stage base commit: `e5f4c3ed78972c8d161671d145bf525cb99033f4`.
Last reviewed commit: `e5f4c3ed78972c8d161671d145bf525cb99033f4`.
Target: PA19 full-stage. Phase: implement; independent PA19 audit pending.
Entry: 397/423 required cases pass (26 failures); earlier PA1–PA18 pass.
The previous interrupted turn has no evidenced implementation or live job;
this turn revalidated the clean checkout and failure log.

## Design and work groups

Retain the streaming parser, canonical typed graph, specialization fact owners,
lazy demands and direct typed LowIR path. Group failures by their shared owner
before editing; record concrete data flow and complexity with each repair.

- Type composition and deduction: function/reference/array shells, qualified
  aliases, defaulted class packs, constructor ordering.
- Dependent lookup/substitution: member aliases/variables, qualified results,
  explicit-template ADL, default-argument queries and no-eager SFINAE.
- LowIR mismatches: inspect selected semantic entities, demand and ABI facts;
  distinguish implementation faults from independently proved oracle defects.

## Performance and validation

PA19/O0 has no mandated numerical latency/RSS ceiling. Preserve inherited
measurements; their self-selected thresholds are diagnostics under spec §9.
Freeze entry/final binaries and inputs; measure compiler wall time/RSS with
A/A calibration and ABBA, verify equivalent output before comparison, and
measure checked native runtime/payload size through the supplied backend.
Newly correct behavior receives final-only costs, not spurious speedup claims.
No optional optimization is planned. Record scaling and existing work bounds.

Required: `make test-pa19`, `make test-report-through-pa18`, file audit for
`dev/src`; finish with the through-PA19 report when the stage passes. Personal
controls live in `student.tests/pa19/` and run explicitly. Preserve all 423
course cases and their comparison rules.

## Handoff ledger

First increment: retained type/declaration composition repairs in parser name
classification, argument substitution, class deduction and explicit conversion.
Data flow: parsed argument -> canonical TypeId/template EntityId -> one
occurrence-frame substitution -> ordinary signature/conversion -> typed LowIR.
Function types retain ellipses; friend template-ids retain indexed categories;
comma-optional varargs do not become function packs; member alias templates keep
declaration identity; dependent function arguments are substituted once; fixed
class heads use sequence deduction; related reference casts bind their objects.
Complexity: existing indexed lookups and type/frame caches; linear argument
sequences and reachable base edges, with no new retries or ownership graph.
Validation: 25 explicit controls pass (19 checked executions, six rejections).
PA14/15/17/18 retain all 1,254 passes; PA19 improves to 403/423, without oracle
or coverage changes. Performance measurement remains pending.

Second increment extends the same group to elaborated-type namespace identity,
ADL-only explicit template-id syntax required by the course, and empty-tail
constructor/function ordering. Owners remain class declaration, token prediction
and cached function ordering. Bounded delimiter lookahead constructs no grammar;
ordering projects at most one argument sequence and caches the complete key.
44 personal controls and 74 inherited ordering controls pass. The complete root
report retains all 3,029 prior-stage passes plus 22 focused properties; PA19 is
406/423. No course inputs, oracles or comparison rules have changed.

Implementation unfinished: two member variable-template rejections. Their
separate variable specialization owner rebuilds projected initializer queries,
and qualified value queries do not apply the member variable-template argument
list. Class-valued results additionally require declaration/initializer/storage
demands and constant-object emission to compose. This is a new semantic state
group, beyond the completed type/declaration substitution repairs. Fifteen
output mismatches remain for demand, initialization, discarded-value behavior,
explicit specialization metadata, NTTP conversion and defaulted-pack deduction.
Some match historical oracle defects; no correction is claimed without the
separate reducer/proof/bundle revision required by the contract.
Independent review: whole-stage correctness/architecture/performance audit
remains pending; review markers above must not move during implementation.
No completed behavior group or handoff boundary yet.
