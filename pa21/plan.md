# PA21 compact plan — implementation 108 (in progress)

Stage base commit: `ac988ea33d4997b44e82baaca5a86623fff3127a`.
Last reviewed commit: `f65eae8d7d434735a0ce981173a347eb8f8a1e59`.
Target: **PA21 full-stage**. Phase: **implementation**.
108 entry: clean `a2b9e823800997ebbe57263a6ea2306e529698be`, **101/116**, 15 failures.
Previous goal turn: **progress** (107 implementation, gates and frozen evidence).

108 work: finish result/parameter ownership and function exception boundaries,
then extend through their shared construction/handler paths. The table below
records the inherited completed group; current owner ledger:

| Group / owner | Data flow, complexity, validation |
|---|---|
| Result ownership / semantic class returns, lowering cleanup | Checked local/result identity → caller destination; retain local unwind lifetime until successful return. Linear return edges, O(1) identity checks. Native destructor counts and throwing returns. |
| Function exception boundary / semantic exception facts, lowering function body | Declared exception fact + body effects → one outer termination region; no name reconstruction. Body traversal once, cached expression effects. Native termination, nested catches, parameter/subobject cleanup. |
| Construction / aggregate plans and helper transfers | Selected member constructors + completed prefix → ordered construction and reverse cleanup. Work follows member edges and output, bounded array expansion. Native self pointers, throwing copies and partial construction. |
| Deferred owners | List backing identities, nested local template bindings, source handler continuation composition; inspect while shared ownership knowledge applies. |

108 increment `2e392cec`: result ownership, explicit nonthrowing boundaries,
empty aggregate return and `T(*this)()` prediction; **106/116**, earlier
**3596/3596**. The next group reaches **108/116** with completed aggregate
prefixes, conservative independent helper transfers and a proved reference
correction. Default-argument temporary failures extend this same ownership
transition through locals, arrays, heap arrays, members and delegation.

| Extension / owner | Data flow, complexity, validation |
|---|---|
| `aggregate_lifetimes`, initializer plans | Typed completed subobject and persistent address → immutable cleanup prefix; retire on aggregate completion. Cached suffix exception facts avoid needless saved addresses. Bounded eight-element expansion, counted loops above the cap; nested/member/copy failure controls. |
| `initialization`, arrays, constructor cleanup | Successful destination → cleanup owner **before** argument-temporary destruction; array default temporaries end before next element. Prefix rebase preserves published failing-constructor snapshots. Native throwing/normal controls at 1/3/8/9/32 elements. |
| Semantic initializer independence | Completed constructor/action identity plus proof mode → cached private-object operand proof. Actual constructors preserve self pointers; aliases, side effects and potentially throwing early transfers keep ordered lowering. |

Entry compiler/log frozen in `/tmp/pa21-108`; unchanged course source coverage.
Reference correction [108](reference-corrections108.md) is independent of student
output and follows [except.ctor]; original/revised execution and reconstruction
will be pinned in final validation. A/B, A/A and ABBA compiler latency/RSS and
checked runtime/size evidence are prepared under spec §9.

107 entry: clean `1422565795ca5d689fe63bbfbaaba7afa0607858`, **92/116**.
Current: **101/116**, **15 remaining**, **9 original failures resolved**.
Earlier PAs: **3596/3596**; through PA21: **3697/3712**. All 116 original
inputs and comparison rules remain. This handoff does not complete PA21.

## Design/spec alignment and completed group

The cumulative typed semantic graph feeds the direct LowIR builder. No new
source parser, IR transport, optimizer or reference delegation was introduced.
The [105 audit](audit.md), [106 evidence](performance106.md) and reference proofs
[102](reference-corrections102.md) / [106](reference-corrections106.md) remain.

| Owner and data flow | Bound and validation |
|---|---|
| `branch_lifetimes.cpp`, `full_expression.cpp`: semantic temporary/destructor and selected destination-conversion/default edges → lazy cleanup/effect facts → regions. | Four boolean facts per node (omitted/result × ordinary/observable), packed in the existing two cache bytes; each computed once per TU. Expected O(1) fact lookup, work proportional to consumed expression/default edges. Effect-free destructors retain PA12 behavior. |
| `control_flow.cpp`, `expression.cpp`, `cleanup.cpp`: full-expression root/final consumer → scalar result slots and branch-local retirement. Subsequent constructors/user conversions retain their operands through the conversion. | Only a terminal logical RHS retires at its join; other branches keep the existing immutable live-prefix/selector graph. Work and storage track expressions and emitted cleanup suffixes. Evaluated/short paths, nested logical results, later operands, conversion observation and throwing RHS execute correctly. |
| `typed_operations.cpp`, `initialization.cpp`: synthesized range result identities and destination addresses → the same region/lifetime owner. | No fake syntax or repeated resolution; one region transition per activation. Range-step destruction, guarded static execution and constructor defaults have explicit controls. |
| `semantic/scalar_consumption.cpp`: observable class receiver → existing private, unmodified constant-selector proof → final scalar consumption. | One traversal per eligible initializer, no new propagation pass. Modified/aliased/volatile selectors retain conservative paths. Required conditional-member reference now matches. |
| `symbols.cpp`: source spelling → collision-safe display name; canonical SymbolId/native ABI identities remain separate. | Existing monotonic collision allocator; no rendered semantic keys. Resolves ambiguous ordinary-global pairing in the throw/sibling fixture; native aliases and earlier stages are checked. |

## Remaining implementation and concrete boundary

- **Class destination/aggregate ownership:** NRVO with observable destruction;
  nontrivial aggregate member helper selection; indirect parameter prologues;
  empty aggregate value/fallthrough presentation; inherited special-member
  grouping in shared-call and template-member fixtures.
- **Backing-array ownership:** initializer-list construction prefixes, retained
  element addresses and static/local backing cleanup region presentation. Their
  execution controls do not waive the two remaining contract comparisons.
- **Function/source EH:** implicit nonthrowing destructor body termination
  wrappers; handler context/exit ordering and nested miss continuations. This
  also affects value-parameter and local-class-specialization fixtures.
- **Nested local template context:** `Box<local Piece>::Guard` remains a required
  compile failure. Remaining guarded-static and nested-catch comparisons also
  need support-global/RTTI pairing and region placement work.

107 completed the full-expression evaluation group, then extended it through
range operations, constructor defaults, scalar member results and conversions
that observe still-live operands. The next failures require changing the owners
of class result transfer, partial subobject construction, function termination
or nested template binding. Adjusting the expression scheduler alone cannot
establish those facts; changing destruction/copy behavior to match their output
would be unsafe without those owner changes. This is the handoff boundary, not
a waiver of any remaining implementation or whole-stage requirement.

Independent review questions: verify final-consumer classification across
conversion/default edges, lazy-cache key completeness, range-operation ownership,
and the cumulative context/terminal composition from 106. These are separate
from the known implementation failures above; preserve the review marker.

## Validation and performance

[Validation](../student.tests/pa21/validation107.json) records exact failure sets,
coverage hashes, sequential root gates, file audit and explicit personal suites.
[Through report](../student.tests/pa21/through107.json) retains the final scope.
Root reports share scratch: exploratory concurrent totals were discarded and
required gates are run sequentially. [43 full-expression execution controls](../student.tests/pa21/full_expression107.py)
include a discovered/repaired conversion-lifetime defect, not just passing
fixtures. The inherited standalone RTTI backend discrepancy remains recorded
with its passing host-runtime counterpart.

[Performance](performance107.md) reports frozen A/B compiler latency/RSS and
checked executable runtime/size, A/A calibration and ABBA spreads. PA21/O0 uses
spec §9 stage-scoped acceptance: there is no optional optimizer or mandated
numeric speedup gate. Required region costs and supplied-backend constraints
are explicit. The eight-element array expansion cap is unchanged. Historical
+15%, +16 MiB and 5.5× diagnostics remain measurements, not additional exit gates;
all 102–106 evidence is preserved. Native optimization/ELF and self-hosting
remain later stages. No advancement until the through-PA21 report passes.

## Handoff ledger

102: `30fe6353`, `9f2181f9`, `f4224e0b` — RTTI/casts, 37/116.
103: `e835d6dc`, `1c541f84`, `fa079cde` — captures/copies, 49/116.
104: `e2af8963`, `f03b9371`, `511fe9b9`, `3b87e462` — lists/demand, 71/116.
105: audit through `f65eae8d`, recorded by `06b2d989`; 45 failures retained.
106: `9c4f64da`, `301ef6fd`, `e80ad0c7`, `c64e88fb` — source EH/lifetimes,
proved reference corrections, empty lifetime-record cost removed; 92/116.
107: `b821682b`, `20476a77` — full-expression regions/results/consumers,
range temporaries, constructor defaults and source display identity; 101/116.
Evidence commits follow implementation without compiler edits. Previous goal
turn classification: **progress** (106 changed authoritative implementation and
validation). Ralph owns acceptance and independent whole-stage review.
