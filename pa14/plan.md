# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement**; architecture work remains.
Original entry **84/314**; continuation entry/current **314/314**.
All **230 original failures** remain resolved. Coverage, references and comparison
rules are unchanged. PA15 has not started.

## Design/spec alignment and remaining groups

One parsed source graph and canonical template/type/argument identities feed
ordinary semantics and typed LowIR. Fixed names, queries, scalar expressions,
direct/indirect calls and constructor-argument recipes are shared. Calls record
ADL, selection and conversions at definition time; occurrences consume mapped
arguments, defaults, emission demand and object/lifetime actions. Source fixed
bases retain access edges independently of layout. See [ownership](implementation.md).

| Remaining current-PA14 owner | Data flow, complexity and validation |
| --- | --- |
| Typed template body graph | Add symbolic nonstatic objects/member signatures, constructor/operator expressions and declaration/return conversions. Then replace full-region projection with dependent-only facts. Work follows new object/operand edges; validate access, lifetime, source/context identity, unused-body legality and native/scaling cases. |
| Demand/failure graph | Separate layout/default/exception/body states, typed reasons/reverse dependencies and structured expected failures. Work follows demanded facts/edges; validate cycles, complete negative keys and unrelated-declaration scaling. Direct-call ranking now returns compact failure, but dependent candidate operations still need this owner. |

The completed group was extended through parameter adjustment, fixed-base access,
class conversion results, constructor defaults and secondary transfer recipes.
Further source-call caching reaches a concrete representation boundary:
nonstatic member calls require a pattern-owned object's identity, initialization,
lifetime and concrete member signature. These currently come from function-body
and member-expression owners. Copying those into the present source-only cache
would mix specialization-specific access/object facts. Introduce symbolic object
facts before sharing those consumers or removing their occurrence projection.
Entire regions still project (50N/53N nodes in the call corpora). This remains
current-stage work; course success and performance acceptance do not waive it.

## Performance evidence

The frozen entry/final A/A+ABBA campaign adds **476 observations**, **5,194 total**
verified, with all earlier campaigns preserved. All **27 compiler/seven native
outputs are byte-identical**. Compiler text grows 14,080 bytes (1.15%); records
remain 112/36 bytes. At 4,000 scalar-call specializations, candidate work falls
48,000→4,011, conversions 72,012→12,027 and RSS 1,498 KiB. Class-call pairs improve
8.7–11.5% at 1,000 and 8.9–9.6% at 4,000, with 1,298 KiB less RSS at 4,000.
Small scalar-call timing is mixed. Required checking of 4,000 unused definitions
adds 100.7 ms/4,876 KiB; seventeen retained reducers establish missing legality.
Ordinary calls-4 is mixed (.9977/1.0208); all outliers and prior costs remain.
No general compiler or generated-program speedup is claimed.

Work is bounded by required source candidate/argument edges plus concrete use
edges and required materializations. Source recipes are TU-owned and proportional
to source; generated-code growth budget is zero, verified by exact output.
O0 adds no optional optimizer/native backend and has no mandated numeric compiler
threshold. Historical self-imposed gates remain diagnostics; preserve mandated
bounds, correctness, coverage and measurements. See [performance.md](performance.md).

## Handoff ledger

Continuation entry `d6891c36`: the preceding call/constructor-recipe group is
**verified progress**, with 1935/1935 through-stage checks and frozen evidence.
Next owner: symbolic fixed object/member expressions and their call receiver
edges. Source facts retain pattern declaration or source-expression identity,
member selection, cv/category and base adjustment; each occurrence maps only
its receiver/object identity and lifetime use. No concrete temporary may enter
a source-owned receiver recipe. Work follows member/receiver edges once per
source and once per use. Validate fixed class parameters/locals, fields, access,
virtual and qualified dispatch, side effects and class-result lifetimes, then
course/native/sanitizer checks and frozen compiler/native scaling evidence.

Continuation entry `816c9dc0`: preceding scalar-fact work was **verified progress**.
The earlier transfer/query continuation (`c05778ed`) and scalar/evidence commits
(`b22e683f`, `d0030349`, `ced1c0d6`, `973b9928`, `d19a1ff7`, `816c9dc0`) remain
preserved with their ownership and measurements in the linked reports.

| Current increment | Commit / validation |
| --- | --- |
| Shared typed direct-call selection and structured candidate result | `ba609e57`: through 1935/1935; native/query checks |
| Fixed call facts, indirect parameters, ADL and source base access | `358a9d5b`: through 1935/1935; fifteen native programs; initial sanitizer parity |
| Constructor/default and user-result recipes; private-base conversion checks | `421aa292`: through 1935/1935; seventeen fixed-call rejections; native/default/lifetime controls |

Final checks: **314/314 PA14**, **1621/1621 earlier**, **1935/1935 through**;
fifteen native programs; release/ASan/UBSan status/output parity on **329 inputs**;
**48** explicit sanitizer rejections and two ABI controls. File audit passes with
three inherited header advisories. Evidence verification passes on all 5,194
observations. Rejection parity is not counted as additional course passes.
Frozen binaries, reduced proofs, exact commands/statuses and logs live in
`$RALPH_ARTIFACT_DIR/pa14-call-facts/`; prior artifact directories are unchanged.
The fixed call/argument group is complete. The symbolic object/body and finer
demand/failure boundary above remains incomplete. Intended changes are committed
with a clean tree at handoff.

Current object increment: fixed class/pointer objects, shared receiver views,
member calls and one member-value owner pass through **1935/1935**, sixteen
native programs, fourteen new object/member rejections and the reduced
reference/static-member reproducer. Sanitizer/performance validation is pending.
Dependent template-owned member paths remain a distinct incomplete owner.

Receiver extension: selected pointer arithmetic/subscript operations and type
queries validate complete pointees; callable class objects route to their
operator owner. Through 1935/1935 and twenty object rejections pass. A related
default-argument identity control exposed shared lowered temporary storage.
That group now gives each emitted materialization its own location and captures
cleanup addresses; classifiers follow semantic default edges. Seventeen native
programs (including destructor/branch/default/array controls), through 1935/1935
and file audit pass. Sanitizer and frozen performance validation remain pending.
