# PA16 compact plan — implementation 43

Stage base commit: `438d56b164600f4fa19d25dcb5f09a76e2a79776`
Last reviewed commit: `7c39a6edbfa43c226036b8a92fe236722ac85dcc`

Target: **PA16 full-stage**. Entry HEAD `c9c4ddb75c65c8de2849b49ecaf42ea83d75fdab`,
**153/154** passing. Previous goal turn: progress (initializer storage/lifecycle
implementation and evidence). Both review markers remain unchanged.

## Design/spec alignment

Semantic class facts own the result ABI. A user-provided empty destructor stays
nontrivial, with caller-owned result storage. Full-expression lowering now records
its final temporary identity and avoids reopening an EH region once that result
is complete. Potentially throwing destruction still uses the remaining live
suffix. One qualified [oracle correction](result-reference-correction.md) changes
only the two factory result boundaries and their uses, with a reduced source,
N3485/ABI/LowIR proof and pinned bundle/before-after hashes. Coverage, sources,
exit sidecars and full comparison rules are preserved.

The constant evaluator now represents the inherited member-pointer value family
by typed member identity: null, same-owner qualification, bool/equality,
conditionals, zero/value initialization, object projection, constexpr member
calls and reference/aggregate/array storage. [Ownership and data flow](member-constants.md)
include persistent scalar-reference temporary identities and demanded template
initializer relocations. Lowering consumes selected identities and typed data;
no string key, grammar replay or lookup reconstruction was introduced.

Member operations use constant work or the required receiver path. Initializer
semantics stay sparse; data writing follows emitted items. A demanded static
initializer visits distinct values/edges once and uses the existing function
worklist. Final-result cleanup removes five IR instructions and one EH region
per affected expression. The 512-call / 1,000,000-step constexpr limits remain.
No new implementation source file or optional optimizer was needed.

## Implementation boundary and independent review

Both entry implementation groups are closed: result ABI/cleanup and the recorded
member-pointer constant-value reducer. No known failure remains in the completed
PA16 behavior groups or the course suite. The supported member-pointer surface
is formation, same-owner values and applications to compatible/inherited objects.

Cross-owner member-pointer casts and virtual member-pointer dispatch remain
unimplemented in the inherited runtime subset. Extending those requires new
selected conversion adjustments and a virtual member-pointer ABI, before the
constant evaluator can consume such facts; they are distinct source-language
owners, not unfinished cases in this represented value algebra. Their whole-stage
scope must be resolved by independent audit; this records the implementation
boundary and does not waive any requirement.

Independent review remains due for all accumulated changes since the preserved
review marker: constant cache/lifetime completeness, template query adapters,
ABI/lifetime policy, static relocation demand and reference proofs. Green course
checks are evidence for this implementation handoff, not assignment certification
or authorization to advance past that review.

## Performance evidence

[Result/member evidence](result-performance.md) retains **302** frozen A/A+ABBA
and final-only observations, all input/output hashes, compiler latency/RSS,
native runtime/code/data sizes, telemetry, outliers and producer continuations.
The 24-million-result loop improves 319.68→192.01 ms and member-array runtime
2,622.20→685.47 ms; common executables are identical. Compiler text grows 0.66%.
The focused 4,000-result repeat improves in all four pairs and saves 2,780 KiB
peak RSS. Template-storage demand scales 3.97x for 4x input, with two member
body demands at both sizes. No general speedup is claimed.

[Initializer](initialization-performance.md), [storage](storage-performance.md),
[objects](object-performance.md) and [audit](audit-performance.md) preserve earlier
measurements. PA16/O0 has no mandated numerical latency/RSS/text ceiling;
historical percentage/scaling targets are diagnostics under stage-scoped
acceptance, not inherited exit gates. Required semantic costs and all growth
remain disclosed; correctness, existing resource limits and coverage are intact.

## Handoff ledger

| Increment / evidence | Result |
| --- | --- |
| Implementations 41–42 | Typed object/address execution, constructor/conversion facts, scalar-array readonly copies and program lifecycle ownership; through entry 153/154. Earlier proofs/evidence remain in their linked documents. |
| `ee190270` result boundary | Final temporary cleanup correction; one narrow result-ABI oracle revision; seven native lifetime controls, including structural potentially-throwing cleanup. |
| `048014e8` member values | Typed member constants through scalar/object/template paths; scalar-reference temporary identity and demanded relocation targets. 34 native / 6 rejection controls; the formerly pending reducer now passes. |
| Required final checks | `make test-pa16`: **154/154**; `make test-report-through-pa15`: **2112/2112**; through PA16: **2266/2266**. File audit passes with three inherited header warnings. |
| Explicit personal validation | All eight prior suites pass (**205 native / 77 rejection**), plus **41 native / 6 rejection** new controls. Total **246 native / 83 rejection**. |
| Performance / evidence binding | [Checkpoint](../student.tests/pa16/result-checkpoint.json) and [verifier](../student.tests/pa16/verify_result.py) bind current code, unchanged coverage, oracle revision, checks and frozen measurements. |

Handoff: implementation work for the completed behavior groups is coherent and
validated; the independent whole-stage review above remains outstanding.
