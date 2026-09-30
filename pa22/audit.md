# PA22 final independent audit 120

Target: **PA22 full-stage**, O0 source-to-LowIR. Final Spec Alignment: **aligned**.
The complete accumulated stage was independently reconstructed from source and
commits, including the previously unaudited 118 and 119 handoffs. Checkpoint
conclusions were treated as leads, not architectural evidence. Stage base is
`a8482d76`; entry is `17bc7a06`; audited implementation is `2b1a02c9`.
The prior turn made progress by committing the completed implementation and its
handoff. No advancement to PA23 is included in this audit.

## Findings and corrections

**Empty-subobject identity.** `class_layout` put every empty base at zero and
checked only a first-base chain when avoiding an empty member collision. Distinct
same-type subobjects consequently had equal addresses, including nested and
array cases. Constexpr pointer/member-pointer equality, static displacements and
constructor/destructor receivers inherited the error. The layout owner now
publishes a bounded canonical summary of contained empty types and reserves
separate storage when summaries overlap. Arrays contribute their element's
identity set without expansion. All consumers still use the one completed layout;
there is no lowering/constexpr offset patch or name-based recovery.

The 64-identity cap is an EBO work budget. An overflow summary means unknown and
uses disjoint storage for potentially overlapping subobjects. It cannot turn an
unknown into a proof of no overlap. Summaries are immutable slices in a TU pool;
local flat indexes/vectors are destroyed after each layout. At most 64 identities
are consumed per direct class-valued base/member edge, and 64 retained per class.
The fallback avoids unbounded hole search and array expansion. Conservative
placement may reserve more storage than maximal EBO; C++11 and PA22 require
identity/correct offsets, not maximal packing. No executable computation is added.

The [oracle correction](reference-corrections120.md) changes three layout facts
in one fixture, with a reducer, standard/contract proof, original/corrected hashes
and pinned bundle revision. The supplied reference itself reproduces the error.
No source, status, comparator or coverage is changed. Overlay 119's four previous
corrections were independently checked against [conv.mem], [expr.static.cast],
[expr.mptr.oper], [conv.bool], [temp.inst] and the LowIR comparison width rule;
all four reducers were rerun successfully.

**Access across later and alternative bases.** `accessible`, protected-object
checking and implicit-object ranking still contained first-base-only walks.
`base_accessible` stopped after the first route, rejecting a public static member
when an earlier route was private. Access now visits the relevant explicit base
edges, deduplicates visited classes, and accepts a permitted path. Public using
introductions and protected member/friend privileges work through later bases.
Implicit-object ranking follows the selected member's related base path. Ordinary
lookup, ambiguity and protected-object restrictions remain intact; private and
ambiguous controls still reject. Completed relatedness queries reuse canonical
path/miss facts; open classes use uncached graph queries without demanding class
completion. No global scan, generation invalidation or speculative body demand
was added. These changes implement C++11 [class.access.base]/1,4,
[class.protected]/1 and [class.paths]/1 ([N3485](../doc/n3485.txt), lines
13413–13502, 13717–13724 and 13803–13805).

The [new controls](../student.tests/pa22/audit120.py) improve **7/26 to 26/26**.
They cover later-base using/access and query substitution, friendship, alternate
static paths, required rejection, empty base/member/array identities, constexpr
and runtime agreement, member constant displacements, lifecycle receivers, the
summary budget, a billion-element layout without expansion, and a demanded
member-pointer template with nontrivial multi-base lifetimes. All 126 inherited
personal controls also pass, including the 118 alias/effect boundaries and 119
receiver lifetime/exception controls. Historical evidence remains unchanged.

## Independently reconstructed architecture

| Spec surface | Actual owner, representative data flow and conclusion |
|---|---|
| §§1–2 source and identity | `lowering/driver.cpp` owns immutable `SourceBuffer`/preprocessor, `PostTokenCursor`, ring-buffer `syntax::Cursor`, `Ast` and `Analyzer` per TU. `Parser::translation_unit(&sem)` constructs one source graph with attached NodeId facts; there is no complete syntax-to-semantic tree copy. Tokens borrow ranges and carry IdentifierIds. Types, declarations, scopes and member constants use canonical IDs and flat indexes. Text/mangling are output views. |
| §§3–5 lookup, access and demand | Scope/kind/name indexes and explicit using/base/ADL edges delimit candidates. PA22 owner types participate in deduction, packs, ADL and substitution. `member_address_template_argument` keys query, target, access override and explicit-instantiation context; query identity carries lexical context. Incomplete failures remain retryable, completed facts remain cached. The corrected access walks inspect only related inheritance edges. `finish` drains separate cursor-based queues for storage, specialization, friends, vtables and members; fact states deduplicate publication. It does not retry every declaration when a fact arrives. |
| §§1,4,8 template representation | `template_binding` retains definition-time identities; parent-linked `TemplateSubstitutionFrame` overlays hold parameter/argument slices. `instantiate_function` observes body states, projects retained parsed regions and substitutes dependent facts. `syntax/occurrence.cpp` stores source/context occurrences, not copied trees or tokens; fixed facts use shared source owners. Deferred defaults, nested classes and bodies remain separate demands. |
| §§2,6 layout and constants | Canonical member types include owner and member TypeIds. `base_path` owns selected edges; layout later supplies offsets. `MemberConstant` interns entity plus signed displacement, separately from type. Conversions compose displacement in semantics. `constant_member_receiver` caches object-address/value identity, never mutable object contents; `constant_read` still checks storage lifetime/readability. The new layout summary is owned by the same once-completed class fact. |
| §§6–7 direct lowering | `Procedural` consumes `ObjectUse`, conversions, selected members, layout, initialization/transfer/destruction actions and ABI entries to append typed LowIR instructions/data. Function/data member pointers use target+adjustment/biased-offset representations; unknown function values retain high-word extraction and target-word truth. Qualified field paths preserve both naming-class and declaring-class projections. RTTI void casts test null and consume the existing vtable offset-to-top. No production text roundtrip or reference execution occurs. |
| §§7–9 phase boundary | PA22 ends at the LowIR writer. Explicit reader/validator/roundtrip tools are audit adapters. The supplied native/object backend and host linker are used only to validate generated LowIR and executable performance. Native MIR, selection, allocation, encoding/debug optimization and self-hosting are PA24–34 obligations; they are not implemented or claimed here. |
| §§8–10 allocation and self-containment | TU pools/slabs own nodes, sparse fact records, canonical maps and summaries. Function-local flow maps and lowering scratch are released at their owner boundaries; only the required typed LowIR program survives each TU for final output. Source buffers outlive borrowed token ranges. There is no hot shared ownership, persistent mutable process cache, fixture recognition or subprocess delegation in the production frontend/lowerer. The three file-audit header-organization warnings remain warnings, with unchanged passing status. |

**Nontrivial declaration trace.** The `D : L,R` control constructs bases in order
and copies `L` then `R`, then destroys `R` before `L`. Semantic actions select the
actual functions and completed offsets 0/4. LowIR contains `obj<8x4>`, the +4
receiver projection and those selected calls; execution checks the trace
`12345656`. The repaired empty hierarchy separately emits `obj<2x1>` and member
constants with adjustments 0/1, and checks both receiver addresses. Thus the
review follows layout through constant evaluation, static data and runtime use.

**Demanded template trace.** `call<R,&R::read>` forms the owner-qualified member
type and canonical address argument, obtains one specialization/body state,
projects the retained `return (x.*F)()` region and records the selected receiver
and target. The `D` argument converts to the `R` subobject before the call.
LowIR carries the selected target and typed call signature; ABI rendering uses
that same identity. The supplied backend consumes this output and the executable
checks its result and surrounding lifetimes. Telemetry and hashes for both
traces are retained in the new-control evidence; stats/validation leave IR
byte-identical.

## Optimization proof and performance acceptance

The local zero-adjustment proof is frozen after all writes and exposures are
recorded, uses active/proven/unknown states and a shared 64-node root budget.
The 118 storage proof keys local declaration plus physical offset and records
facts per source occurrence. It follows lowering's RHS-before-destination and
member-receiver-before-arguments order. Calls, alias/unknown writes, overloaded
arrows, control/lifetime boundaries, union/reference/volatile storage and budget
exhaustion retain generic adjustment. Its cap is 4096 visits/function, depth 64;
each attempted value proof retains its separate 64-node bound. Only requested
completed functions are visited. No fixed-point rescan or callee analysis occurs.

The 119 conversion-result owner inspects at most eight return wrappers; its
receiver use proof inspects at most eight wrappers and requires fresh, empty,
effect-free construction with inert destruction. Named/nonempty/effectful/unknown
receivers retain evaluation. Selected conversion IDs index use edges. Lowering
consumes the omit-receiver bit; it does not rediscover purity or demand a body.
Both proofs remove constant-size executable work with **zero code-growth budget**.
Initializer expansion retains its eight-element cap and loop fallback. There is
no inlining, body cloning, unbounded higher-level pass or new optimizer policy.

[Performance120](performance120.md) reports final frozen A/A + ABBA compilation
latency/RSS and separately checked runtime/text size on common and affected
workloads. Broken entry controls are excluded from performance comparisons.
The 114–119 records, including noisy/regressing observations, are preserved.
The 117 and 118 proof profitability experiments establish live-loop benefit with
less shift/load/store/stack work; no runtime claim is inferred from IR counts.
The 119 scalar-loop slowdown is documented, including its four-address placement
diagnostic; it depends on the supplied native backend's layout and remains a
reported constraint. Receiver elision is the O0 contract, and no padding or
unprofitable optional loop transform was introduced to conceal it.

Spec §9 stage-scoped acceptance applies. PA22/O0 mandates no numeric latency/RSS
threshold. Inherited +15%, +16 MiB and 5.5× targets were self-selected diagnostics,
not exit gates. Their observations and misses remain recorded; this audit
confirms that classification without weakening timeouts, mandated work/growth
bounds, correctness, comparison or coverage. Native optimization and self-host
benchmarks remain later-stage requirements. No unsupported performance win is
claimed for the semantic fixes.

## Validation and review ledger

The [final evidence record](../student.tests/pa22/audit120-validation.json) retains
commands, exit codes, log hashes, the commit/path manifest, source/binary hashes,
fixture inventory, all personal results, performance evidence and stable LowIR
roundtrips. Fresh entry and final root reports both print **3811/3811**, all **22**
stages passing; focused property controls also pass. This is the observed current
report, independently of the supplied status summary's 3835 count. The unchanged
contract inventory and hash comparison establish that coverage was preserved.

Required `perl scripts/cppgm_file_audit.pl --stage pa22 --paths dev/src` passes
with the three inherited warnings. Required `make test-report-through-pa22`
passes; `make test-pa22` passes **99/99**. All **95** accepted PA22 outputs validate
and roundtrip stably; all **four** required rejections remain. The 26 new and
126 inherited controls pass, as do all five reference-correction reducers.
The inherited exception controls retain the documented standalone-backend RTTI
limitation and execute unchanged LowIR through the supplied hosted object lane.
No timeout setting or comparison policy was changed.

| Reviewed range | Final disposition |
|---|---|
| 114–117, `a8482d76..e90fa3fa` | Source architecture and earlier audit fixes rechecked as part of the whole stage. The original [checkpoint audit](audit117.md) is preserved with its five then-open failures. |
| 118, `f18dfb62..e10bdd7f` | Local member storage, equivalent/repeated qualified paths and overloaded-arrow invalidation independently reviewed; inherited controls rerun. |
| 119, `247c7de4..321c93db`, handoff `17bc7a06` | Constant result/use/lifetime ownership, width validation and all four oracle proofs independently reviewed and rerun. |
| 120, `2b1a02c9` | Empty identity and all-path access owners repaired, new reducers/controls and one minimal reference overlay validated; final performance and full-stage exit evidence consolidated. |

No unaudited PA22 handoff or known unresolved in-scope defect remains. PA23 owns
virtual inheritance, polymorphic multiple inheritance and the broader RTTI ABI.
