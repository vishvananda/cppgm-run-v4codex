# PA18 implementation handoff 64

Implementation: `d15a0328`, `e01b8763`; execution-harness correction `3f49ce3a`.
Entry: `30a610065e586c481a094c60698f7ae8a7952904`, 282/420. Stage/review base
remains `94dcb8ad21664137e87d574e878c14a4a047348a`. Previous turn: progress.
This is an incomplete implementation handoff; PA18 full-stage remains the target.

## Completed group and design

- `specialize` establishes the same immediate-substitution mode for explicit
  and deduced candidates, including defaults. A failed signature is discarded
  before retained type-access recipes run. Source class completion still resets
  probing so definition side effects are hard errors. Selected bodies retain
  their existing demand owner; no unselected body is instantiated here.
- Query/type owners return compact failure for invalid direct constructor
  participation, abstract by-value arguments, allocation, qualified-value/member
  categories, sizeof and pointer arithmetic/increment. Constant evaluation never
  manufactures a value from a failed query. Array substitution rejects invalid
  element types and bounds, including abstract template-class elements. Alias
  and class argument failures propagate zero without qualifying that sentinel.
- `query_dependencies.cpp` owns class→query and query→consumer edges. Only
  incomplete-dependent facts publish reverse edges. Completion invalidates
  affected query/constant records through a deduplicated worklist, including
  successful fallback answers whose discarded candidates were incomplete.
  Failed alias/signature records retain their blocking QueryId and retry only
  after its invalidation. Immutable failures remain cached; type failures with
  an unresolved prerequisite are not published as permanent negatives.
- Increment/decrement result expressions lower directly to typed Itanium unary
  nodes. Prefix uses `pp_`/`mm_`; postfix uses `pp`/`mm`, excluding the semantic
  dummy argument. No names/types/IR are serialized to transport semantics.

Trace: parsed `decltype(sizeof(T), char())` → canonical dependent query/type →
substitution frame and concrete query → candidate discard or recorded result →
selected ordinary declaration/conversions → existing typed LowIR lowering.
A forward-declared class contributes its identity as a dependency, not a global
lookup generation. Its completion signals exactly its consumers. New source
ownership is registered in `dev/frontend_source_sets.mk`.

Canonical keys remain TypeId/QueryId/frame/entity identities. Hot completed
lookups are O(1) average; signature work is proportional to required type/query
edges. Invalidation is proportional to notified dependency edges; no registry
scan or retry-all loop. Flat maps and geometric vectors release with the TU;
local probes/worklists release on return. See [performance64.md](performance64.md)
for frozen compiler/RSS/runtime/text measurements, scaling and limits.

Language proof: N3485 14.8.2 [temp.deduct]/7–8, including immediate-context vs
instantiation side effects and invalid array/qualified-name examples
(`doc/n3485.txt:20450–20516`); sizeof completeness is 5.3.3 [expr.sizeof].
ABI proof: `doc/itanium-mangling.txt:511–515`, unary and prefix productions.
No contract fixture, reference bundle/output, harness comparison rule or required
coverage changed. No reference correction was needed.

## Validation and boundary

`make test-pa18`: **312/420**, **30 existing failures fixed**, zero new failures,
same 420 fixtures. Earlier root through-PA17: **2609/2609**, all 17 stages.
The root through-PA18 report remains red at **2921/3029**. File audit passes
with its three inherited header-division advisories. Commands, hashes, fixed and
remaining fixtures, and results: [loop64-evidence.json](../student.tests/pa18/loop64-evidence.json).

All 33 substitution controls, 64 earlier ordering/pack controls, eight exact
increment ABI tool/API roundtrips and the existing PA9 direct API/control suites
pass. All 30 fixed fixtures have additional validation: 26 execute through the
supplied backend and four have no main and retain their validated-LowIR oracle.
The using-declaration fixture intentionally returns its selected overload's
value **2**. An initial extra execution check assumed zero; the corrected check
and successful re-execution are recorded. Required course comparisons never
changed. Completion controls use type/overload assertions; an initial constexpr
ellipsis fallback also exposed inherited unsupported constexpr variadic
execution, which is separate unfinished constant-evaluation work.

The coherent boundary is immediate-candidate failure propagation and its
completion-dependent cache lifecycle. The remaining **108** failures comprise
**86 status failures and 22 LowIR mismatches**. Further fixes require different
facts: retained member/lexical declaration frames, compound assignment/braced
construction/destructor queries, cast/access and selected conversion validity,
constructor/conversion-target deduction, pointer/reference NTTP values, and
initialization/result metadata lowering. Existing throwing conversion/access
validators mix immediate obligations with demanded side effects; merely catching
more exceptions would violate the completed group's boundary. Missing virtual
base facts also prevent proving the remaining downcast case. These are known
unfinished implementation, not review questions. No coverage or language rule
is waived, and the full root through-PA18 gate must pass before advancement.

Independent review remains required over the complete stage range: verify the
immediate/side-effect boundary, negative-fact identities, active-query completion
and reverse-edge integration, and direct ABI operation encoding. Earlier
ordering/pack review markers remain open. Reconcile whole-stage spec findings
before advancement; this implementation handoff does not certify the stage.
