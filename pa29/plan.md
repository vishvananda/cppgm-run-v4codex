# PA29 compact plan — implementation189

Target: **PA29 full-stage**. Phase: **implementation handoff; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `52070178897f5894edaf2f35d03a734b781979d4`.
Last reviewed commit: `2df00585bd10d4e2e068934394dffc8adb0a47ed`.
Implementation189 entry: `54ddacb1ced906e692ea94bde5110b0757e1b2af`, clean,
**395/403**, eight failures. Review markers are unchanged. The preceding goal
turn made verified progress; entry inspection found no live compiler/test work.

## Design/spec alignment

[Handoff189](handoff189.md) completes the template-definition/declared-value group
and extends it through constant boolean conditions. Three incorrect success
oracles are corrected with [reducers, C++11/contract proof and bundle revision](reference-corrections189.md).
No primary definitions or trait values are fabricated from library names.

Static assertions now select/check and execute the actual contextual conversion,
including explicit bool, access and deletion. Template dependence and immutable
source identities remain intact. Messages validate during parsing and render only
on failure. Concrete noexcept and conditional explicit/guide conditions use the
existing typed validator with their lexical access scope; friend access survives.
Constructor-template condition failures publish a
failed specialization result and allow fallback candidates; demanded-definition
errors remain hard errors.

Source nodes → canonical declarations/query facts → recorded conversions →
constant execution or typed LowIR → ELF. Existing complete-fact identity caches,
parent-linked frames, TU arenas and function release boundaries remain. There is
no new cache, grammar replay, global retry, textual phase transport, optimizer,
ABI representation or per-node allocation. Work follows actual lookup candidates,
demanded facts and message code units; ready assertions are not rechecked.

## Validation and performance

`make test-pa29`: **398/403**, five failures, exit 2. PA1–28: **4538/4538**.
Root through PA29: **4936/4941**, exactly those five failures. File audit passes
with four inherited header-division warnings. **221 explicit commands**, seven
validated LowIR outputs and symbol inspections pass. All **403 original inputs**
remain; of **1,707** contract/harness paths, only three proven exit-status
corrections differ. The entry compiler also accepted eleven invalid assertion
controls and rejected the valid assertion composite; those defects are fixed.

[Performance189](performance189.md) retains **1,512 observations and 24 launchers**:
frozen preliminary/final comparisons, A/A calibration, six ABBA blocks, compiler
latency/RSS and runtime/text. Final measurements cover four common workloads and
template/assertion/specifier demand at 600/1200/2400 declarations. Generated images
are unchanged across the ten final equivalent pairs; semantic work counters scale
linearly. No speedup is claimed. Mandatory evaluator/native/time limits remain;
inherited blanket 15%/zero-growth targets remain diagnostic under spec §9.

## Remaining groups and handoff ledger

The [five-case ledger](../student.tests/pa29/evidence189/remaining.json) separates
unfinished implementation from independent review:

| Owner | Cases | Disposition |
|---|---:|---|
| Binary128/half representation and ABI | 3 | Unfinished; genuine formats, operations, typed IR and ABI needed. |
| Executable vectors and type-operand intrinsics | 1 | Unfinished; vector reduction/comparison, conversion and lowering needed. |
| Nested-template ABI-tag policy | 1 | Independent contract question from implementation176; unchanged and counted. |

`d0df835d` records entry ownership; `32e2430a` records reference proof/corrections;
`4ed34a87` fixes contextual assertions/messages; `e08707fc` completes specifier
access contexts; `383fae65` preserves conditional-specifier SFINAE. The evidence
commit binds final artifacts in the
[manifest](../student.tests/pa29/evidence189/manifest.json).
The group was extended through definitions, specialization, deferred demand,
constant/runtime values, assertion conversion/message rules and all adjacent
constant boolean access consumers found defective. Remaining numeric/vector cases
require new representations/operations and cannot be supplied by another demand
or contextual-conversion correction. This is the concrete handoff boundary.

Ralph independently reviews these changes and reference corrections. Full
PA29/root-through success and whole-stage audit remain required before PA30.
[Audit186](audit.md), all review markers, historical handoffs and measurements
remain retained; no outstanding requirement is waived.
