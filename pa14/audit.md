# PA14 final architecture audit

Status: **complete for PA14 full-stage / O0 LowIR**. PA13 base
`8af3c149454e4e43e441206e6978f4d1300e079b`; audit entry `78c2f13e`;
last checkpoint `7f401fa8`; final implementation `d359539c`. PA15 has not started. This audit independently read
`spec.md`, the handout, testing rules, stage commits, source and inherited plans.
Checkpoint conclusions were checked against ownership paths and reduced cases.
The authoritative root result is **314 PA14 + 1621 prior = 1935/1935**, all 14
stages; the supplied 1959 count does not match the checked-in report.

## Final architecture and spec alignment

| Surface | Reconstructed ownership and data flow |
|---|---|
| Source and preprocessing | Immutable TU buffers and interned spellings feed streaming preprocessing, post-token and syntax cursors. `lowering/driver.cpp` connects parser and Analyzer directly. Text dumps are explicit adapters. PA5 lexical hints implement that handout's parsing contract. |
| Parsed graph | One retained source AST supplies semantic construction. Eight-byte source/context occurrences project structural edges; deferred body/default regions are demanded by identity. No grammar replay, cloned syntax tree or textual transport occurs during instantiation. |
| Canonical identity | TU-owned type, argument-pack, declaration, scope, query and specialization IDs; immutable parent-linked substitution frames. Manglings and rendered names are output views. Fixed source properties are shared; distinct local nominal classes retain concrete specialization identities. |
| Lookup and selection | Flat scope/name indexes and explicit base/using/ADL edges; compact candidate sequences and shape/arity filters. Common conversion/ranking routines select declarations and typed conversion recipes. Lowering consumes those choices. |
| Facts and scheduling | Declaration, signature, definition, layout, default binding/check/demand, class property, body, lifetime, virtual ABI and emission owners remain separate. Active/Success/Failure states are monotonic for a complete key. Missing prerequisites remain unavailable rather than becoming negative facts. Deduplicated queues, dependency edges and parameter cursors visit affected consumers. |
| Allocation and release | Sparse facts, shared expression properties and per-use evaluation/object records use TU-owned pools and flat indexes. Candidate vectors and builder scratch are shorter lived. The frontend is released after each TU's typed LowIR construction; the combined output program owns only required IR/ABI data. No process-global mutable semantic cache. |
| Typed lowering and ABI | One emission identity per function, initializer, thunk and distinct ABI entry. Builders consume selected calls/conversions, destination/temporary markers, subobject paths, cleanup and virtual slots. Required missing facts are invariant failures. The LowIR writer implements the requested text output, without reparsing it in production. |
| Optimization and native boundary | PA14 exposes O0 LowIR. It adds no optional optimizer or student native backend. Native selection, allocation, ELF encoding, optimized levels and self-hosting belong to later assignments. Supplied backend execution is confined to harnesses and observations. |

The review crossed driver/source/preprocessor/cursors/parser; template declaration,
class, definition, binding, signature and instantiation; type/value/query facts;
lookup/overload, defaults, member access, object construction, statements and
lifetime; virtual declaration/demand and typed symbol emission. It revisited the
stage history through source sharing, sparse publication, signature/demand,
virtual/lifecycle/default ownership and the subsequent body, initializer, mode
and destination commits. The following traces and controls close the outstanding
handoffs rather than treating the previous test pass as architecture acceptance.

## End-to-end traces

**Default-initialized object.** In `struct Leaf { Leaf(int=3); }; struct C { Leaf
part; }; template<class T> void f(){ C value; }`, parsing creates one declaration
and one declarator. Definition binding resolves fixed C and records the selected
default constructor under the source variable identity. Declaration-only
constructor/destructor properties inspect required subobjects, access/deletion
and available defaults without creating actions, temporary objects or called
bodies. A demanded `f<Tag>` gets a canonical specialization and immutable frame.
`declare_object` passes the exact declarator occurrence to `default_initialize`:
the source mapping retrieves the checked recipe. The concrete object owns its
storage and cleanup; selected default arguments and required member bodies are
demanded by their own owners. Constructor actions and ABI facts then reach typed
LowIR exactly once. The new native control checks four million required calls
and checksum 18,000,000. The inspection probe observes M source checks and KM
uses, then repeats property/finish queries 10,000 times without extra work.

**Fixed callable and conditional.** A template body containing fixed `C&` or C
operands retains operator/cast/construction/condition recipes in the definition
environment. Runtime identity or a dependent initializer does not make a known
non-constant object's type dependent. Source selection uses the same overload,
default, variadic and surrogate rules as ordinary expressions. Recipes retain
typed operands, selected functions, conversions and receiver paths. A concrete
occurrence projects operands, copies the recipe's per-use conversion state,
demands the selected body/ABI entry and owns materialization and cleanup. The
surrogate receiver is evaluated once. Conditional lowering applies only the
selected runtime branch; both branches were checked semantically. True/false,
reference, scalar, unrelated-class and class-result programs check conversion
effects and live-object balance. The operator benchmark checks sixteen million
effects and checksum 40,000,000; its compiler scaling separates source M, concrete
K, unrelated N and repeated Q work.

**Ordinary linkage declaration.** `extern int value;` in a block receives a
namespace-owned entity with a separate lexical binding. A hidden namespace/name
index connects an earlier block declaration to a later namespace definition;
recording the local declaration does not expose its name to unrelated lexical
lookup. Lowering emits a reference to that global identity and allocates no
local storage for the block declaration. The two reduced native cases previously
read the wrong object; both declaration orders now produce the required result.

**Failure and demand.** A fixed deleted default constructor/destructor fails its
property owner; later body/action queries cannot reinterpret partial publication
as success. An unavailable complete-class prerequisite resets NotStarted and
retains its typed dependency. A dependent jump/cleanup failure reaches the
existing Active body interval, then terminal body/lifetime failure. The inherited
10,000-query controls verify stable IDs, actions and queues on success and failure.
Declaring a virtual destructor now obtains only its declaration identity;
usability, exception/triviality, body/actions and emission have distinct demands.

**Useful fact through O0 lowering.** Transfer/effect and destination facts retain
proven object representation, selected constructors, subobject paths, lifetime
and ABI distinctions. A representation transfer may lower directly only when
its existing eligibility/effect proof permits it; otherwise it retains the call.
Required copies, overlapping object identity, source effects, debug/source
identity and cleanup survive the decision. An existing destination does not
allocate a duplicate temporary; a required source prvalue still does. Actions
are prepared once and consumed directly, with no name-based recovery. The
supplied backend observes the resulting loops/calls and output payload; this
audit makes no claim about a student allocator, spill policy or ELF writer that
PA14 does not yet own.

## Defects corrected across their owners

| Finding | Final correction and evidence |
|---|---|
| Source defaults skipped uninitialized references, deleted/inaccessible subobject constructors/destructors, const and local-class properties | Declaration-property producers are separate from actions/bodies. Source defaults retain fixed selections; concrete declarators reuse them. Known local fields/bases are inspected, unresolved dependent properties remain concrete obligations. |
| Fixed callable/operator/cast/construction queries missed errors or repeated selection | Shared ordinary selectors publish source recipes without runtime materialization. Concrete uses project operands and prepare conversions/defaults/lifetimes. Class casts use explicit typed operand lists rather than fake ASTs. Braced values reuse the list object's temporary. |
| Conditions/list returns stopped at value dependence | Fixed type facts remain usable; known clauses/conversions are validated and retained. Dependent nominal/type facts are substituted at their concrete owner. Const integral and dependent bit-field values retain value dependence. |
| Conditional conversions lost directional ranking, ambiguity, cv/ref category or selected conversion functions | The common conditional routine returns branch conversions once, including direct-reference constraints and builtin arithmetic/pointer candidates. Query, source and ordinary consumers share it; lowering evaluates one selected branch. |
| Unevaluated function-call result over-demanded completeness/destruction | `decltype` tracks its direct result through parentheses and the right comma operand. The function-call exemption does not exempt functional construction such as `C()`. Query evaluation is scoped and does not demand called bodies. |
| Local class redeclarations, known members and default flags lacked source ownership | Forward/definition identity is stable; later forwards preserve known properties. Typed pattern member/property keys retain fixed facts, bases and an unresolved-base marker without pretending the source local class is a completed concrete class. |
| `C()`/`C(2)` statements and for-initializers were misparsed as declarations | Bounded declaration lookahead distinguishes expression-only operands from a declarator prefix without constructing abandoned trees. Binding follows the chosen declaration/expression kind. |
| Block extern declarations created local storage / split identity | Namespace entity ownership and hidden extern index preserve lexical visibility and later definition joining; lowering respects the global storage owner. |
| Other exposed declaration paths | Duplicate source variables/parameters are rejected with tag hiding and extern redeclarations retained. Out-of-class static definitions keep in-class initializers. Anonymous-union storage without an active variant is not separately default-constructed by a user-provided enclosing constructor. |

The [242-case entry proof](../student.tests/pa14/source-obligations-proofs.json)
records **160 invalid programs accepted, nine valid programs rejected and two
wrong native results** under the destination checkpoint: 171 incorrect outcomes.
The proof cites local N3485 [dcl.init]/7,9,12, [class.ctor]/5–6, [class.dtor]/5,
[class.temporary]/1, [expr.call]/11, [expr.type.conv], [expr.static.cast],
[over.match.ctor], [over.match.call], [over.match.oper], [dcl.init.list]/7,
[expr.cond]/1,3–6, [stmt.ambig]/1, [basic.scope.block], [basic.link]/6 and
[temp.res]/8. The const-default compatibility retained by existing fixtures
follows the correction in [CWG 253](https://cplusplus.github.io/CWG/issues/253.html),
not an assertion that every case follows the literal uncorrected N3485 wording.
The handout requires supported fixed template-body checks; this is not a claim
to diagnose every ill-formed-no-diagnostic-required template. Compiler agreement
is not used as a language proof. **No reference output or bundle was changed.**

## Keys, invalidation and whole-pipeline budgets

Source recipes are indexed by the immutable source use, whose initialization
mode and declaration/access environment are fixed; target identity is checked
on reuse. Concrete operands, object identity, conversions and lifetime state
belong to occurrence/specialization uses. Default slots combine declaration
identity with the concrete specialization. Direct/copy empty-list plans keep
separate canonical-target/access keys. Pattern property keys distinguish default
initialization from destruction; contextual access is checked outside cached
intrinsic properties. Complete-class default properties are terminal only after
the class prerequisite is available. Query keys include type, bound entity,
context and value-sensitive provenance. No global generation invalidation or
retry scan was added. TU release ends every cache lifetime.

Retained budgets include one parse per source region; source/key/use-proportional
semantic storage and work; at most four value-conversion variants per fixed
operation; one writable publication view per visit; one transition per completed
fact/dependency edge; and one lowering per emission identity. Required lookup
candidate relationships and aggregate/subobject walks determine semantic work,
not products of unrelated declarations. Transfer actions are prepared once;
reference actions retain the six-instruction-plus-fixed-setup bound, cumulative
array expansion is eight, and deleting-entry sharing covers at most one
nontrivial subobject. Larger array work stays loop based. The final reuse change
adds zero generated text growth on every comparable measured executable.

No optional executable transform, speculative search, fixed-point optimizer or
new invalidation policy was added. There is therefore no optional runtime
profitability decision to use as an excuse for semantic cost. Source selection
reuse has a measured compiler benefit on the affected large-K operator shape.
Other timing changes are disclosed without extrapolation. Native execution
provides no runtime speedup claim from node/count changes.

Under `spec.md` stage-scoped acceptance, inherited zero-overhead, zero-occurrence,
historical-live-layout equality and fixed historical benchmark-count targets
are diagnostic choices, not additional PA14 gates. Concrete object/lifetime
identity and eight-byte projection views are required storage, not duplicated
parsed graphs. Historical layout probes now check their exact committed sources;
the final probe checks all 24 current headers. Earlier +6,714 KiB source-cache
and roughly +15 MiB calls observations, subsequent ownership fixes and all
other measurements remain recorded; the exact allocator/high-water causes of
those historical deltas are not retrospectively claimed. No mandated limit,
correctness check, comparison rule or coverage was weakened. Avoidable selection,
publication and duplicate destination costs were corrected, not reclassified.

## Performance and validation

The [final performance report](../student.tests/pa14/source-obligations-performance.md)
contains all 47 main / 15 repeat compiler rows, 14 main / four repeat native
rows, paired ABBA changes, A/A calibration, wall/RSS ranges and text sizes.
The new **1,120 observations** bring the verified ledger to **19,726**. The
K=128 fixed-operator shape improves compiler latency about 5% in both campaigns;
other effects are mixed. Native text is unchanged for all 14 workloads and
12 images are identical. The other two have equivalent course-canonical LowIR
and equal text size. No native or corpus-wide speedup is claimed. Compiler text
increases 53,952 bytes (3.94%); MemberFacts adds four bytes and Analyzer 208.
Memory increases, reversals and large timing outliers remain in the report.

[Final validation](../student.tests/pa14/source-obligations-validation.json):
79 passing groups, including both required gates, 349 entry/current comparisons,
349 release/sanitizer comparisons, all 35 existing PA14 native programs,
242 source controls per build, four branch programs per build, and inherited
statement, initializer, mode, destination, demand, lifecycle, default, virtual
and PA13 ABI controls. Property/finish probes repeat 10,000 times with stable
state. All **1,266 fixture/reference hashes** are unchanged. The file audit
passes with the same three inherited large-header warnings. The exact commands
are `perl scripts/cppgm_file_audit.pl --stage pa14 --paths dev/src` and
`make test-report-through-pa14`; the root report passes **1935/1935, 14/14**.
`python3 student.tests/pa14/verify_performance.py` verifies the cumulative ledger.
The [final command record](../student.tests/pa14/final-audit-closure.json) pins
the implementation commit, binary and final gate/verifier logs.

The [journal](../student.tests/pa14/source-obligations-journal.json) preserves
initial failing course runs, two preflight failures, the default-declarator reuse
correction and a personal source-file collision detected by hash verification.
Both complete control groups were rerun with unique paths before final timing.
Initial binaries, source snapshot, layout and 77-check validation remain;
acceptance uses only the final corrected binaries and 79-check validation.
Completed untimed sanitizer probes were losslessly archived under the shared
20 GiB ceiling, with original/archive hashes and checked decompression. No
compression, build, test or verifier overlapped either final timing campaign.

## Consolidated ledger and handoffs

| Increment | Added observations | Cumulative | Closure |
|---|---:|---:|---|
| Source/signature/demand/virtual/lifecycle through `460f495a` | 14,896 | 14,896 | Independently traced; historical evidence retained |
| Default slots, dependencies, access and elision through audit entry | 952 | 15,848 | 85 validation groups; source/body/default owners separated |
| Body/lifetime publication and fixed statements `2ab55111` | 532 | 16,380 | 51 groups; terminal failure, source return conversions |
| Initializer recipes and projected list operands `dc112d7e` | 616 | 16,996 | 67 groups; 82 controls per build |
| Copy/direct/list modes `b13f567a` | 728 | 17,724 | 71 groups; 75 controls per build; destination cost identified |
| Existing destination ownership `7f401fa8` | 882 | 18,606 | 73 groups; KM duplicate entities/scopes removed |
| Final source defaults, operators, queries and declaration consumers | 1,120 | **19,726** | 79 groups; 242 controls and four branch programs per build |

All five default/query cases in the last checkpoint's handoff are closed under
both final builds; the [closure manifest](../student.tests/pa14/source-obligations-handoff.json)
pins their original sources and current outcomes. Earlier initializer/mode,
body/lifetime, signature, virtual and default handoffs are covered by current
validation and the cumulative verifier. No unaudited PA14 handoff remains.

Dependent local nominal identities and properties still require their concrete
specialization; partial source facts do not imply full class completion. General
SFINAE, fuller two-phase lookup, template tiers excluded by the PA14 handout and
native/optimization/self-hosting surfaces remain at their documented later-stage
boundaries. One valid local abstract-base control produces valid LowIR but the
supplied backend reports `undefined native symbol: pure_virtual`; the exact
limitation is retained alongside a concrete-base native dispatch companion.
It is not a compiler workaround, altered reference or waived PA14 LowIR check.
