# PA29 compact plan — implementation184 handoff

Target: **PA29 full-stage**. Phase: **implementation handoff; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `667edd800e4e5eb1b1ef92a3108da3bd96c51708`.
Audit entry: `9211517d1f554f61efa7d6e2e30020dc13171f3f`.
Last reviewed commit: `52070178897f5894edaf2f35d03a734b781979d4`.
Implementation184 entry HEAD: `68ee6f8d12349e4ba9576963c200178cf4ae629e`.
Implementation code commit: `52c00f4d`.

## Design and spec alignment

[Handoff184](handoff184.md) records owner, data flow, complexity, validation and
boundary. Shared explicit conversion selection now chooses cv-only pointer/member
casts before reinterpretation, preserves composed base adjustments and enforces
nested qualifier and reference-category rules. Constant evaluation retains
object/member identity and declared storage restrictions. Typed conversions feed
ordinary LowIR, per-function MIR and direct ELF; lowering does not reselect casts.
Qualification work is O(type depth), with O(1) new temporary storage and no new
cache, parsing pass, global retry, textual transport or allocation family.

The original Darwin constant-expression repair was extended through pointer,
reference, array, member-pointer, null, dependent/template, private/virtual-base
and native-offset controls. All **42** explicit controls and **194** inspection
commands pass. Inspection follows source and demanded template facts to native
code, validates LowIR adapters and checks recorded adjustment/constant fields.

## Required validation and performance

PA29 **390/403**, improving **14 → 13** entry failures; PA1–28 **4538/4538**;
through PA29 **4928/4941**. File audit passes with four inherited warnings.
All **403** inputs and **1,707** contract/harness paths are unchanged. See the
[validation](../student.tests/pa29/evidence184/validation.json),
[stage delta](../student.tests/pa29/evidence184/stage-delta.json) and
[manifest](../student.tests/pa29/evidence184/manifest.json).

[Performance184](performance184.md) retains **328** observations and eight
launchers. A/A and six ABBA blocks compare frozen correct implementations on
four inherited inputs and the runtime-cast owner; all five object pairs are
byte-identical. Compiler paired medians are **0.9889–1.0033**, with every compiler
range crossing unity. Latency/RSS and runtime/text, noise and outliers are all
reported. Newly accepted constexpr casts have final-only 600/1200/2400 scaling:
one body transition and one cache hit per specialization, fixed address/member
facts, and fixed **661-byte** text. No optional transform is added; mandated
limits remain unchanged. Inherited blanket 15%/zero-growth targets remain
diagnostic under spec §9, with historical evidence preserved.

## Handoff ledger and remaining groups

Completed: cv-only explicit cast selection and constant identity, composed base
adjustments, nested qualifier validation and reference-category correction.
The [13-case ledger](../student.tests/pa29/evidence184/remaining.json) distinguishes
**10 unfinished implementation cases** from **3 independent contract questions**.
No reference correction, reduced comparison rule or waived failure is claimed.

Remaining implementation requires extended scalar/complex representation and
ABI, vector expression/deduction/lowering, contextual coroutine syntax and
hosted template behavior. Current diagnostics do not enter the repaired cv-only
conversion path; these need distinct type/parser/template/native owners.
Independent contract questions remain nothrow trait shorthand, invocable-cache
expectation and nested-template ABI-tag policy. They still count as failures.

Independent review must examine cumulative committed changes and resolve those
questions. [Audit182](audit.md), [handoff183](handoff183.md), all earlier evidence
and the review markers above remain preserved. This handoff returns implementation
control to Ralph. Full PA29/root-through success and whole-stage audit are still
required before PA30.
