# PA29 compact plan — implementation187

Target: **PA29 full-stage**. Phase: **implementation handoff; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `52070178897f5894edaf2f35d03a734b781979d4`.
Last reviewed commit: `2df00585bd10d4e2e068934394dffc8adb0a47ed`.
Implementation187 entry: `5ef7f89a3bd6be203c6a845432b417bcf518eb07`, clean,
392/403 (11 failures). The stage-base and review markers remain unchanged.
The prior reviewed range was `52070178..2df00585`; see [audit186](audit.md).

## Design/spec alignment and completed owner

[Handoff187](handoff187.md) covers GNU **floating complex** types together:
syntax → canonical component type → typed expression/query/constant facts →
initializer/storage plans → LowIR signatures → native SysV fixed/variadic ABI.
Construction, cv/value-category component access, conversions, arithmetic,
constexpr execution/addresses, storage, template use and runtime calls share
that representation. `c32/c64/c80` preserve ABI identity through serialization.
The intrinsic registry owns probe answers and typed signature selection.

Work is fixed per pair, with at most three builtin signatures/TU and six runtime
helper declarations/program. Existing interned component constants fit the
ordinary value payload. Shared fixes retain `va_arg` result types and address
object-valued SSA bases correctly. MIR reports actual complex return carriers.
No source-name recovery, hosted-only lowering, optional optimizer or new budget
is introduced. Inherited guide/default, explicit-cast and bit-integer design
from audit186 remains intact.

## Validation and performance

`make test-pa29`: **394/403**, nine failures, exit 2. PA1–28: **4538/4538**.
Root through PA29: **4932/4941**, exactly those nine failures. File audit passes
with four inherited header-division warnings. **99** explicit commands include
18 rejection inputs, GCC/Clang interop, typed LowIR roundtrips and native MIR.
All **403** inputs and **1,707** contract/harness paths remain unchanged.
The original complex failures are fixed; no previous pass or coverage is lost.

[Performance187](performance187.md) retains **544 observations plus 16 launchers**
across the preliminary and final frozen comparisons. Four common object and
executable pairs are byte-identical; the final inspection-only repair preserves
all measured program bytes. Complex demand scaling checks all 24 final compiler
samples; emitted code/text stay fixed while demanded frontend work grows with N.
Compiler latency/RSS, runtime/text, paired spread and noise are reported together.
No speedup is claimed. Existing mandatory constexpr/inline/storage limits remain;
inherited blanket 15%/zero-growth targets remain diagnostic under spec §9.
All historical performance evidence is preserved.

## Remaining groups and handoff ledger

The [nine-case ledger](../student.tests/pa29/evidence187/remaining.json) distinguishes
six unfinished implementation cases from three independent contract questions:

| Owner | Cases | Disposition |
|---|---:|---|
| Binary128/half representation and ABI | 3 | Unfinished implementation; need genuine new formats and arithmetic. |
| Extended-vector deduction/contextual operators | 2 | Unfinished parser/template semantics. |
| Hosted primary `char_traits` demand | 1 | Unfinished implementation; inherited contract discussion retained. |
| Nothrow shorthand/invocability/nested ABI tags | 3 | Independent contract review; all requirements and failures still counted. |

`5c93ddef` implements the complex owner; `f714397c` repairs its MIR reporting.
The records commit binds final source, results and measurements in the
[evidence manifest](../student.tests/pa29/evidence187/manifest.json).
The owner was extended through every discovered constant/storage/ABI/adapter
consumer, including variadic calls. Further numerical work needs real precision
and arithmetic primitives absent from the existing component store; the completed
pair model cannot supply them. The other groups have different semantic owners.
This is the concrete handoff boundary, not assignment completion. GNU dependent
component mangling differs between GCC and Clang; the typed GCC vendor encoding
is retained and recorded for independent audit. Nothing is waived.

Ralph resumes implementation or schedules review. Full PA29/root-through success
and the whole-stage audit remain required before PA30. [Audit182](audit182.md),
audit186 and every historical handoff/measurement remain retained.
