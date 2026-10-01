# PA29 compact plan — implementation183

Target: **PA29 full-stage**. Phase: **implementation; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `667edd800e4e5eb1b1ef92a3108da3bd96c51708`.
Audit entry: `9211517d1f554f61efa7d6e2e30020dc13171f3f`.
Last reviewed commit: `52070178897f5894edaf2f35d03a734b781979d4`.

## Reviewed design and fixes

Reviewed the complete **17-commit** range across three accepted handoffs and
this audit fix, including combined changes to **79 implementation/build paths**.
[Audit182](audit.md) records every commit, ownership trace, findings and ledger;
[range evidence](../student.tests/pa29/evidence182/range.json) binds the range.
[Audit178](audit178.md) and handoffs179–181 remain preserved.

Two shared-owner defects are repaired: array structured bindings retain source
element cv-qualifiers; mandatory inline expansion has one native preparation
owner, so source LowIR and explicit adapters receive the same single budget.
Typed alias/copy/lifetime facts, canonical array identity and separate demand
states remain shared. Production uses typed LowIR, per-function MIR and direct
ELF. The integrated control combines all three handoffs through exception cleanup.
The code fixes were validated and committed before these record-only updates.

## Validation and performance disposition

PA29 **388/403**, the identical **15** entry failures; PA1–28 **4538/4538**;
through PA29 **4926/4941**. File audit passes with four inherited warnings.
All **403** inputs and **1,707** contract/harness paths are unchanged.
**141** explicit controls and **653** inspection commands pass, including
recursive/depth/growth fallback, source/adapter object and work-counter equality,
volatile accesses, zero-size copy/destruction, native symbols/unwind and telemetry.
The [manifest](../student.tests/pa29/evidence182/manifest.json) binds tested source,
compiler, checks, coverage, controls and measurements to the reviewed code tip.

[Performance182](performance182.md) retains **440** new performance observations
plus eight launchers and verifies **2,576** inherited observations. Equivalent
benchmark objects are identical; corrected cv-array cases have final-only scaling.
Compiler latency/RSS and runtime/text are reported together with all noise/spread.
No optional transform is added. Mandatory inline budgets and existing compiler
limits remain enforced. Inherited blanket 15%/zero-growth targets remain diagnostic
under spec §9; necessary semantic costs and later-stage work add no exit gate.

## Remaining groups and handoff quality

The [15-case ledger](../student.tests/pa29/evidence182/remaining.json) groups work
as extended syntax/types **11**, template demand/hosted ABI **3**, legacy trait **1**.
It retains **12 implementation cases** and **3 independent contract questions**;
none is waived. No reference, fixture, comparison rule or bundle revision changes.
Full PA29 and through-report success remain prerequisites for PA30.

Finish broad owners with their cv/reference, substitution, lifetime/exception,
ABI and adapter-budget interactions before handoff. The three groups made useful
progress (22 → 15 failures), but array deduction and serialization fallback gaps
caused avoidable follow-up fragmentation. This audit establishes one reviewed
code baseline; it does not mark the assignment complete.

## Implementation183 entry and working groups

Entry HEAD: `a2ce2670669e0ef392d61b1e1e4aeb6f2e302467`; review markers above
are preserved. Entry is clean; 388/403 with the 15 failures in evidence182.
Previous turn: completed audit evidence (progress); no live work to resume.

First owner: hosted deduction-guide declarations. Parser distinguishes the
parameter-clause/arrow form; semantics retain target template, canonical
signature, environment and explicit condition without binding a callable
function or demanding a body. Extend to templated/explicit guides, defaults,
parameter packs, same-scope/access validation and duplicate signatures. Work
tracks source nodes and parameter types; indexed guide identity avoids scans
of unrelated declarations. Validate acceptance/rejection, no symbol emission,
ordinary declaration interactions and scaling. Frozen entry compiler:
`/tmp/pa29-183/entry`; measurements use fixed inputs, A/A and ABBA compiler
latency/RSS plus checked executable runtime/text. No optional optimizer added.

Remaining owners: extended scalar/vector types, contextual coroutine syntax,
template demand/ABI and legacy traits. Independent contract questions remain
explicitly separate in the inherited ledger, with unchanged fixtures. Extend
related work after the initial fix; stopping requires a coherent owner boundary,
required checks, evidence, committed changes and a clean worktree.

Implementation183 checkpoint: declaration-only guides now have a distinct source
node and typed per-primary registry; both ordinary and complete-class parsing
preserve the class-template binding. Shared default checks reject evaluated
parameter uses and pack defaults, while retained type queries support sizeof/
noexcept before body demand. The 52 explicit controls pass, PA1–28 is 4538/4538,
and preliminary PA29 is 389/403 (guide fixture repaired; 14 unchanged failures).
File audit retains its four inherited warnings. Inspection and frozen performance
evidence remain in progress; this checkpoint is not a handoff boundary.
