# PA29 compact plan — implementation189 (in progress)

Target: **PA29 full-stage**. Phase: **implementation; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `52070178897f5894edaf2f35d03a734b781979d4`.
Last reviewed commit: `2df00585bd10d4e2e068934394dffc8adb0a47ed`.
Implementation188 entry: `c1caa0fc6b2985a633c4db587097ac5c08c30c75`, clean,
394/403 (nine failures). These stage-base and review markers are unchanged.
The preceding goal turn made verified progress: its committed complex owner and
validation fixed two original failures. Entry inspection found no live work.

## Implementation189 entry and working groups

Entry HEAD: `54ddacb1ced906e692ea94bde5110b0757e1b2af`, clean, **395/403**
(eight failures). Stage base and Last reviewed commit above remain unchanged.
Previous goal turn: **progress**, verified by implementation188 source, evidence
and failure reduction; no compiler/test process remains live at entry.

Current owner: ordinary template definition/member demand and source-defined
trait results. Resolve three apparent erroneous success references using reduced
inputs and cited C++11/contract proof; check real primary definitions, explicit
specializations, unused member bodies, substitution and constant/runtime results.
Data flow: source declaration identity → specialization environment → member
lookup/initializer fact → typed LowIR → ELF. Work must follow demanded facts and
indexed dependencies, with TU-owned identity/cache lifetimes; no library-name
recognition, synthesized definitions or global retry is acceptable.

Validation: explicit positive/negative controls and source-to-object inspections;
frozen entry/final compiler evidence under spec §9 if source changes; all PA29
fixtures, earlier stages and file audit. Float/half and vector representation
remain separate unfinished owners; nested ABI tag remains independent review.
This working scope is not a handoff boundary. Extend related fixes as evidence
requires; preserve all prior review/evidence markers and coverage.

## Design/spec alignment

[Handoff188](handoff188.md) completes contextual coroutine **syntax retention**,
ordinary C++11 identifier preservation, complete-class name indexing, and shared
throw/yield comma boundaries. Streaming tokens → indexed lexical categories →
distinct source nodes → once-bound template operands. Actual coroutine demand
rejects before lowering; there are no fabricated calls/results or ordinary return
conversions. Defaults and unevaluated operands cannot borrow a body context.

Late comma/parenthesized declarators and unscoped enumerators share the existing
class-region index. Attributes, constructor initializers, using names and operator
spellings retain their own boundaries; variable templates keep their category.
Contextual spellings use interned IDs and each indexed declarator binds once.
Work follows source prefixes/nodes and lexical lookup edges; no grammar replay,
textual phase transport, new cache, global retry or per-node allocation is added.
Existing typed exception/lifetime lowering handles throw tails and cleanups.

## Validation and performance

`make test-pa29`: **395/403**, eight failures, exit 2. PA1–28: **4538/4538**.
Root through PA29: **4933/4941**, exactly those eight failures. File audit passes
with four inherited header-division warnings. **61** explicit control commands,
five AST grouping assertions, typed LowIR validation, emitted-symbol checks,
GCC/Clang runtime agreement and 22 rejection checks pass. All **403** inputs and
**1,707** contract/harness paths are unchanged. One original failure is fixed;
no previous pass or coverage is lost.

[Performance188](performance188.md) retains **880 observations plus 16 launchers**
across preliminary/final frozen comparisons: A/A plus six ABBA blocks for four
common and three affected index workloads; corrected-only coroutine scaling.
Compiler latency/RSS and runtime/text are reported together. Seven equivalent
object/executable pairs and all measured program bytes survive the refinement
unchanged. Corrected-only work counters scale linearly; no frame/body is emitted
for an unused coroutine. No runtime speedup is claimed. Mandatory limits remain;
inherited blanket 15%/zero-growth targets remain diagnostic under spec §9.
All historical measurements and preliminary observations are preserved.

## Remaining groups and handoff ledger

The [eight-case ledger](../student.tests/pa29/evidence188/remaining.json) keeps five
unfinished implementation cases separate from three independent contract questions:

| Owner | Cases | Disposition |
|---|---:|---|
| Binary128/half representation and ABI | 3 | Unfinished; genuine formats, arithmetic and ABI needed. |
| Extended-vector builtins and executable operations | 1 | Unfinished; type-operand intrinsic, reduction/comparison and vector lowering owners. |
| Hosted primary `char_traits` demand | 1 | Unfinished; inherited contract discussion retained. |
| Nothrow shorthand/invocability/nested ABI tags | 3 | Independent contract review; every failure remains counted. |

`e52f4586` implements the group; `bf8bc00f` removes duplicate indexing and uses
interned contextual IDs. The records commit binds final source/results/measurements
in the [manifest](../student.tests/pa29/evidence188/manifest.json).
The group was extended through declaration boundaries, fixed operand validation,
nested bodies, ordinary identifiers, throw runtime effects and all discovered
prior-stage regressions. Further coroutine work requires promise/frame/lifetime
semantics; the separate vector failure needs absent builtin and executable vector
representations. Neither is another contextual-parser correction. This is the
concrete implementation handoff boundary, not full-stage completion.

Ralph resumes implementation or review. Full PA29/root-through success and the
whole-stage audit remain required before PA30. [Audit186](audit.md), earlier audit
markers, every historical handoff and all independent questions remain retained.
