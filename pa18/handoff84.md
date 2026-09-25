# PA18 implementation handoff 84

Entry `09a77fca41efaa8eefc7913bc49898e7cd81e660`: **411/420**, nine failures.
Implementation commits `cc4cd563` and `b4d66361`, with proved oracle revision
`7a7ce959`, reach **414/420**, six failures. This completes the constructor and
empty-value behavior group; it is not whole-stage certification or an independent
audit. Stage-base and last-reviewed markers are preserved in [the plan](plan.md).

## Ownership, data flow and bounds

| Owner | Data flow | Work / lifetime | Validation |
|---|---|---|---|
| `lowering/aggregate_helpers.cpp` | Checked empty aggregate group → existing distinct destination → no helper/call | One action-head check, O(1); no body allocation, cloning or later pass | Local/temporary/template/nested/array/returned empty aggregates; effectful nonempty controls |
| `semantic/class_values.cpp`, `conversion.cpp` | Canonical source/target → implicit trivial empty-copy facts or ordinary selected transfer → typed materialization/lifetime | O(1) eligibility checks; existing indexed constructor candidates and class caches handle required selection. Facts and conversions remain TU owned | Empty classes with destructors, explicit/private/deleted copies, move-assignment suppression, const/volatile sources, prvalues/lvalues and defaulted copies |
| `semantic/construction.cpp`, `declaration.cpp` | Checked selected delegation edge + demanded entry kinds → deduplicated monotonic worklist → base/complete/polymorphic facts | Each of three entry bits crosses an edge at most once; O(demanded members + edges), O(visited vertices) temporary flat index/vector released at completion | Base-only/complete/both orders, convergent chains, member-template and inherited constructors, polymorphic uses, cycles/access rejection |
| `lowering/construction.cpp`, `symbols.cpp` | Published entry kind → same typed action plan → selected callee entry; local-type function specialization → retained internal symbol | One lowering per existing entry; canonical type/scope locality caches. No semantic replay or text keys | Base-entry calls and roots, complete/base variants, local member-template roots, six source-to-native traces |
| `semantic/operators.cpp`, `lowering/expression.cpp` | Selected usual arithmetic conversions → retained widening policy → explicit typed O0 conversion | O(1) per operand, at most one widening instruction; offsets/comparisons retain existing immediate rules | Signed/unsigned/template multiply, division/remainder, bitwise/compound operations, negative operands and offsets |

The initial empty-helper repair was extended into source and materialized empty
copies. Layout emptiness had bypassed constructor legality and destruction:
private/explicit copies and volatile sources could be accepted, while an empty
class with a nontrivial destructor produced invalid argument LowIR. Only classes
with implicit transfers, no declared destructor and a nonvolatile source retain
the empty-copy shortcut. All other cases use ordinary selection, access/deletion
checks, destructor demand and ABI materialization. No language obligation is
skipped because a class has no data members.

Delegation no longer invents a complete-object use while checking a shared
constructor body. The completed graph forwards actual base/complete/polymorphic
uses, including uses discovered after a target was checked. Lowering consumes
that entry fact directly. Existing source identities, aliases and separate-entry
rules remain the owners of emitted functions; the worklist does not demand any
unrelated body. The new `semantic_delegation_entry_work` counter counts existing
work only. No implementation source was added, so source sets are unchanged.

## Contract correction and required zeroing

[The proof](reference-correction84.md) and independent transformer add five
required one-byte `zeroinit` instructions to three PA18 references. PA11 explicitly
requires zeroing of a retained exact contiguous value-initialized span. C++11
[dcl.init]/6,8 and [expr.type.conv]/2 establish `T()` value initialization. Empty
aggregate `T{}` has no member actions and does not need a synthesized empty helper.

The transformer reads only entry references and checks each destination extent;
all original instructions, metadata, fixtures, status sidecars and comparison
rules remain. The bundle revision and before/after hashes are recorded. Reduced
bundle observations and all revised oracles validate and execute. These changes
are a cumulative O0 contract correction; they do not claim an observable runtime
error where padding is unused. A trial that omitted the zeroing was removed after
it contradicted two unchanged earlier fixtures. No earlier reference was changed.

## Validation and performance

Final check artifacts are bound by [the evidence manifest](../student.tests/pa18/loop84-evidence.json)
and [performance report](performance84.md). The required stage suite has **414/420**
passing, with exactly three entry failures resolved and no new failures. All
**2609/2609** earlier tests and file audit pass. All **420** source inputs and
**1686** fixture paths remain; the only course-byte changes are the three proved
oracle revisions. Personal controls are run explicitly, including the 51 new
constructor/value controls, 21 structural inspections, inherited controls and
ABI/scaling checks. Performance uses frozen binaries/inputs, A/A calibration and
ABBA wall-time blocks; compiler latency/RSS and checked native runtime/payload size
are reported together. PA18 uses O0 LowIR and the supplied backend as its harness.

No optional optimization pass was added. PA11 permits omission of a semantically
trivial constructor action; the empty-group guard removes work before constructing
a helper. Its work limit is one action-head check and its code-growth budget is
zero. Required delegation and transfer checks do not clone code or retry unrelated
facts. Existing eight-lane initialization and named-result summary budgets remain.

## Handoff boundary and independent review

**Unfinished implementation:** six course mismatches remain: three class-result
ABI cases (friend alias, object conversion, defaulted result type), two static
member publication cases (keyword instance and string-like npos), and discarded
reference-result consumption in transitive base deduction. The inherited class
ellipsis reducer is still unfinished. These remain required work, not waived
failures or review-only questions.

Their next fixes require different complete fact owners. Class-result convention
must agree across definitions, calls and function-pointer signatures. Static
publication must distinguish storage/odr-use demand from dormant definitions.
Discarded-value output needs source-category and consumption rules. Class ellipsis
needs a representation for values beyond LowIR's scalar variadic boundary. The
completed constructor-entry and empty-copy facts cannot supply those decisions;
further edits in these owners would not resolve those groups. No related known
constructor/empty-copy defect remains open in this handoff's controls.

**Independent review:** audit the delegation worklist's complete input set and
base/complete/polymorphic propagation, empty-transfer eligibility and retained
source recipes, local-specialization roots, the zero-init proof and scalar
conversion policy. The accumulated [audit 82](audit.md) remains the last review;
handoffs 83 and 84 remain unaudited. Passing implementation tests do not waive
these questions. Do not advance until the full through-PA18 report and whole-stage
independent audit pass.
