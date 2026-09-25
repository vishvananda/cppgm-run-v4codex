# PA18 checkpoint audit 82

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Previous Last reviewed commit: `82fca940b1849d90deffbaba29ee162946f3e23c`.
Entry: `85ea42c0a20d5cc8b051fc53a948c8c4df20233a`.
Last reviewed commit: `ecc308bc5ee33ed40fa773f7981961f3018a5867`.

**Checkpoint audit passes; PA18 full-stage remains incomplete.** This review
covers all 18 commits from the prior reviewed code tip to entry, the combined
changes to **35 implementation/registration files**, their interactions across
handoffs 79–81, and the audit repair. [Audit 78](audit78.md) is preserved verbatim.
The [evidence manifest](../student.tests/pa18/loop82-evidence.json) records full
commit/patch/source identities, exact failure sets, coverage, commands, results,
controls, traces and the frozen performance record.

## Complete accumulated range

| Commits, chronological | Review scope |
|---|---|
| `ef0e43c0` | Previous audit records and frozen evidence; retained obligations and stage-scoped acceptance. |
| `78f826de`, `67c4f685`, `50407fc1`, `42312227`, `00738e66`, `c5e2c271` | Signature plan, both implementation commits, both validation/performance extensions and handoff. Qualified angle heads, typedef/alias declaration syntax, explicit member arguments, first-declaration signature identity versus body parameters, fixed/dependent query selection, ABI projection, and the reference correction. |
| `cec91d23`, `a87dd911`, `ce7d3e7f`, `f660fae4`, `69247877` | Scalar conditions, pointer/reference materialization, shared ordinary/query cast selection, constant storage, unrounded narrowing-check results, harness and evidence. Reviewed the intermediate rounding error and its correction as well as the final combined code. |
| `86f33d49`, `7b8c98a6`, `4ab09a9b`, `9d980cdc`, `25b89fc2`, `85ea42c0` | Runtime-summary plan, proof/conditional/emission implementation, address/alias repair, benchmark preflight classification, request-driven refinement and handoff. Reviewed actual symbol retention, selected-arm cleanup, proof eligibility, query interactions and all retained performance observations. |
| `ecc308bc` | Validated ownership repairs below, explicit audit controls/trace and reproducible cumulative validation/performance harnesses. This is the reviewed code tip; subsequent commits contain records only. |

The prior goal turn supplied the committed implementation and evidence, so it
was progress. The entry process inspection found no inherited compiler/test job
requiring a wait or restart. The baseline stage run independently reproduced
**396/420**, including the exact **24** entry failure paths.

## Findings and repairs

**A — conversion-name lookup was owned by one expression adapter.** Handoff 81
resolved qualified conversion-function-ids in `resolve_expression`, leaving
unqualified calls and retained-template/query consumers on identifier lookup.
`operator int()` therefore failed even when a matching conversion existed, and
qualified-name scope agreement was incorrectly applied to unqualified aliases.

The shared resolver now delegates conversion names to the conversion owner.
Qualified names check both required type contexts; unqualified names use lexical
type lookup and the enclosing class. Retained declarations publish targets in
the existing flat `(scope, canonical target)` index. Fixed targets and conversion
templates remain in the candidate set, including competing cv overloads. Lookup
uses explicit fixed-base edges on a source-local miss, without completing a
class or demanding a body. Aliases, access/deletion checks and target deduction
continue through the ordinary semantic owners.

**B — query substitution confused fixed declaration identity with fixed naming
context.** A fixed base member could retain the source class as its naming class
inside a concrete specialization. Conversion templates could similarly retain a
target-selected specialization owned by the source class. Receiver checking then
compared unrelated source/concrete identities or demanded an unavailable base
path. This is an interaction between 79's signature/query identity and 81's
conversion-name/address handling.

Source member queries now consume the existing typed source-object context and
its fixed base edges. Query substitution remaps the naming class even when the
callee's value/type is nondependent. A selected source function specialization
projects its canonical template and arguments through the frame and reuses the
normal specialization cache; it does not repeat conversion-target deduction.
The query/frame key owns this work. There is no syntax replay, global retry,
name-based recovery or new body demand. Checks include ordinary fixed-base
queries, local patterns, multiple enclosing specializations, dormant poison
bodies, conversion templates, cv overloads and required rejection.

The language anchors are N3485 §3.4.1 [basic.lookup.unqual], §5.1.1/12
[expr.prim.general], §7.1.6.2 [dcl.type.simple], §12.3.2 [class.conv.fct],
§14.6 [temp.res] and §14.7.1 [temp.inst], in [the supplied draft](../doc/n3485.txt).
The qualified conversion-type agreement rule does not constrain a local alias
used in an unqualified conversion-function-id. Unevaluated type determination
requires valid lookup/selection but does not instantiate an unused member body.

**C — angle prediction allocated scratch per template-id.** Handoff 79's local
binding vector allocated once for every angle probe. It now shares the existing
delimiter stack's parser lifetime, reuses capacity, and retains only maximum
active nesting depth. Entries are cleared between probes and released with the
parser. This removes avoidable allocation while preserving lexical prediction,
angle caching, bounded checkpoints and one parse per source region.

The new audit controls improve **13/27 → 27/27**. With the **1137** inherited
controls, **1164** pass. Every positive execution control validates student LowIR
and checks native results; rejection controls check failure status. Two exploratory
explicit throw/catch probes reached the inherited unsupported-statement boundary;
their initial log is retained and identified in the evidence. Explicit exception
syntax is later-stage work, not an added PA18 gate. No existing control was removed.

## Architecture and optimization trace

[The combined audit trace](../student.tests/pa18/audit82_trace.cpp) follows an
ordinary `receiver` declaration, the redeclared `apply` template, and demanded
`Scalar<65543>` members through the production frontend. First-declaration lookup
keeps `apply`'s parameter/result `long` despite the later overload. A selected
conversion binds a narrowed scalar reference; an effectful receiver selects
conditional values; explicit calls and a member pointer retain the actual
conversion definition. Checked execution verifies values, distinct reference
storage and exactly three receiver effects.

The trace records **305 tokens, 649 nodes, 169 occurrence records, two template
body transitions and one runtime-summary inspection**. It emits **six functions,
129 instructions and 201 operands**, with zero query-completion invalidations.
Its LowIR validates and the supplied backend's ELF exits zero. All three handoff
traces also validate and execute on the reviewed binary.

Immutable sources feed the streaming preprocessor/post-token cursor and integrated
parser/semantic consumer. Parsed regions remain source records; substitution uses
compact source/context occurrences and immutable frames. First-signature records
retain canonical type, declarator, environment and frame, while parameter facts
preserve the definition's names, cv and adjusted forms. Query keys retain typed
operations, children, naming/access context and source dependence; substitutions
are keyed by the complete frame. Source-name comparison shapes are separate from
semantic lookup and ABI facts. Lowering consumes selected declarations/conversions
and emits typed LowIR directly. Serialized LowIR is the explicit PA18 output,
not transport between this compiler's production phases.

Source, occurrences, semantic facts, queries, constants and flat indexes are
TU-owned. Candidate/head maps are operation-local, parser angle storage dies with
the parser, and lowering function state is reset at its existing boundary. The
driver releases frontend/lowering TU owners before the next input; program/linkage
identities survive only through output. No process-global mutable cache, owning
hot-node graph, semantic text key or whole-program recovery was added. Both new
implementation sources from handoff 81 remain registered in the source lists.
Inherited list/forwarding/nested scaling checks preserve localized completion
invalidation, one required body/definition transition and compressed array tails.

The useful optimization fact is an already-established named scalar constant
forwarded by a requested completed conversion body. Its legality proof excludes
extra statements, effectful/nonconstant/volatile/reference results and virtual
dispatch; it never grants constexpr eligibility. The O0 budget inspects one return
and at most eight wrappers and retains one result per eligible function. Unknown
or over-budget bodies keep calls. Lowering preserves receiver effects, required
second conversions, storage and cleanup; known conditionals emit only the selected
arm. Actual explicit/address/root uses retain ordinary function emission through
one deferred-list visit. No code cloning, growth, fixed-point pass or broader
optimizer budget was introduced.

[Performance 82](performance82.md) checks compiler latency/RSS and executable
runtime/payload size together against the previous reviewed compiler, over the
union of all three handoff corpora and audit probes. Profitability is evaluated
on checked executable work, not IR counts alone. Native selection, allocation,
ELF/debug encoding and self-hosting are owned by later stages; the supplied
backend is only an explicitly invoked test boundary. Existing ABI facts and
source locations remain attached to the operations that execute.

## Reference proof, validation and disposition

The only accumulated oracle change is
`300-function-template-result-first-lookup.ref.exit_status`. The
[reducer and proof](reference-correction79.md) are sound: the nondependent
`select(0)` expressions bind at their respective declarations, yield different
return types, and define distinct equally viable function templates. The call
is ambiguous. The dependent-name first-declaration rule cannot merge them.
The retained positive prefix and dependent-name controls preserve those obligations.
Fresh reference/student observations reproduce acceptance/rejection respectively;
compiler agreement is not the proof. Bundle
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, its payload, other sidecars and all
comparison rules are unchanged. No new reference correction is made in audit 82.
Earlier proofs 65, 67 and 69 remain preserved.

| Required check / evidence | Reviewed code result |
|---|---|
| `make test-pa18` | **396/420**, exit 2; identical **24** entry failure paths (one status failure, 23 LowIR comparisons). No extra passing case compensates for a new failure. |
| `n=18; if [ "$n" -le 1 ]; then echo '===== ALL TESTS PASSED SUCCESSFULLY! (0/0) ====='; else make test-report-through-pa$((n - 1)); fi` | **2609/2609**, exit 0. |
| `perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src` | Exit 0; same three inherited header-division advisories. |
| Coverage / comparison | **420** original sources; **1686** fixture/reference files unchanged from entry. Full range changes only the proved exit-status sidecar. |
| Personal controls | **1137 inherited + 27 audit = 1164**, all pass. |
| Structural / execution | Ten summary/budget/emission inspections, inherited ABI and scaling/completion checks, and all four source-to-native traces pass. |
| Performance | Frozen cumulative A/A/ABBA evidence, checked outputs, compiler latency/RSS and runtime/size; stage-scoped assessment in performance82.md. |

Historical **+15%, +16 MiB and 5.5×** targets remain diagnostic, not exit gates.
PA18/O0 specifies no numerical compiler-latency/RSS ceiling. Historical misses
and later-stage native/self-hosting constraints add no gate. The audit preserves
all measurements, correctness, coverage, mandated limits and the O0 proof/work/
growth budgets. Necessary semantic costs are disclosed separately from optional
optimization, whose repeatable runtime benefit remains required.

Remaining work is grouped in [plan.md](plan.md): array/aggregate initialization
and constant materialization; object ABI/emission and scalar representation.
All 24 failures and the inherited class-ellipsis reducer remain implementation
obligations. Splitting signatures, scalar conversions and named-result summaries
across three handoffs left shared query/lookup consumers unchecked together.
That fragmentation and repeated packaging were avoidable. Future work should
complete each owner's semantic, constant/query, lowering and emission consumers
before handoff. PA19 still requires a passing root through-PA18 report.

| Checkpoint ledger | Range / fixes | Evidence / disposition |
|---|---|---|
| 82, accumulated audit | `82fca940` → entry `85ea42c0` → code `ecc308bc`; every commit across handoffs 79–81, shared conversion/query ownership and reusable parser scratch repaired | PA18 **396/420**, identical 24 failures; earlier **2609/2609**; file/coverage pass; **1164** controls; cumulative stage-scoped performance assessed. The reviewed code tip above is the next audit baseline; full-stage remains unfinished. |
