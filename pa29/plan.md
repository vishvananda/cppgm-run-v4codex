# PA29 compact plan — implementation184 in progress

Target: **PA29 full-stage**. Phase: **implementation; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `667edd800e4e5eb1b1ef92a3108da3bd96c51708`.
Audit entry: `9211517d1f554f61efa7d6e2e30020dc13171f3f`.
Last reviewed commit: `52070178897f5894edaf2f35d03a734b781979d4`.
Implementation183 entry HEAD: `a2ce2670669e0ef392d61b1e1e4aeb6f2e302467`.
Implementation code commit: `054d162c`.
Implementation184 entry HEAD: `68ee6f8d` (389/403; 14 failures).

## Current implementation group

Own explicit cv-only cast selection in `semantic/explicit_conversion.cpp` and
its constant-address consumers. The Darwin wait-status fixture currently sends
a same-object C-style cv cast through the reinterpretation path. Retain the
selected conversion on the typed expression, preserve address/storage identity,
and test pointer/reference/member-pointer and template forms plus composed base
adjustments. Selection follows C++11 [expr.cast]/4; constant evaluation follows
the chosen operation, not source spelling. Work is bounded by the reachable
type qualification chain and required base edges, with no new global cache or
parser/lowering representation. Validate runtime, constexpr, SFINAE and negative
controls explicitly, then full earlier/stage gates and file audit. Freeze entry
and final binaries for A/A and ABBA compiler latency/RSS and runtime/text evidence;
newly accepted cases get final-only owner scaling. Preserve all existing review
markers, remaining implementation and independent contract questions below.

## Design and spec alignment

[Handoff183](handoff183.md) records the owner, data flow, complexity, validation
and boundary. Hosted deduction guides now have a distinct source declaration,
canonical typed signature, class identity, template environment and conditional
explicit/noexcept queries. Indexed guide records neither hide the class name nor
create callable functions or demand result-class bodies. Complete-class parsing
uses the same category rule. Related shared fixes validate pack/default usage
and retain prototype inquiry queries for template default substitution before
body demand. All 53 explicit controls and 291 inspection commands pass.

The graph is parsed once; identities and source facts remain shared. New maps
use compact keys and have TU lifetimes. Deducibility visits reachable typed
nodes; default validation visits its source expression. Ordinary typed LowIR,
per-function MIR and direct ELF remain the production path. The inspection
covers source/adapter code and symbol equality, no emitted guides, nested member
specialization, default query substitution, incomplete classes and telemetry.

## Required validation and performance

PA29 **389/403**, improving **15 → 14** entry failures; PA1–28 **4538/4538**;
through PA29 **4927/4941**. File audit passes with four inherited warnings.
All **403** inputs and **1,707** contract/harness paths are unchanged. See the
[validation](../student.tests/pa29/evidence183/validation.json),
[stage delta](../student.tests/pa29/evidence183/stage-delta.json) and
[manifest](../student.tests/pa29/evidence183/manifest.json).

[Performance183](performance183.md) retains **544** performance observations plus
eight launchers: 224 first-run common, 224 pinned confirmation and 96 newly
accepted owner observations. A/A and six ABBA blocks compare frozen correct
implementations; newly accepted inputs have final-only scaling. Common objects
are byte-identical. Confirmation compiler paired medians are 0.9766–1.0109 with
ranges crossing unity. Compiler latency/RSS and runtime/text are reported with
all spread. Guide work/storage grows linearly and emits no code; default facts
and demanded bodies grow once per specialization. No optional transform is added.
Mandatory limits remain enforced. Inherited blanket 15%/zero-growth targets
remain diagnostic under spec §9; all historical measurements are preserved.

## Handoff ledger and remaining groups

Completed: guide declaration formation and related template/prototype default
validation/substitution. The initial crash repair was extended through templates,
member guides, canonical duplicates, packs, conditions, access and runtime
positive controls. The [14-case ledger](../student.tests/pa29/evidence183/remaining.json)
retains **11 unfinished implementation cases** and **3 independent contract
questions**, with no waived requirement or reference correction.

Remaining implementation needs extended scalar/complex representations and ABI,
vector expression/deduction/lowering, constant-expression compatibility,
contextual coroutine syntax and hosted template behavior. Those failures do not
enter the repaired guide/default path; they require separate semantic and native
owners. General CTAD selection is a future consumer of the retained declarations,
not a claimed feature of this declaration-acceptance handoff.

Independent review must examine the cumulative committed range and retain the
three contract questions. [Audit182](audit.md), [audit178](audit178.md), earlier
handoffs and performance evidence remain preserved. Review markers above are
unchanged. This handoff returns implementation control to Ralph; full PA29 and
the root through report must pass, followed by whole-stage audit, before PA30.
