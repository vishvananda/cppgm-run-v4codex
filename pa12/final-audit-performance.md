# Final PA12 performance acceptance

This review covers compiler latency/peak RSS, executable runtime and text size
for the complete stage and final correctness repairs. It applies `spec.md`'s
PA12 O0 acceptance, including to inherited plans. The
[architecture audit](audit.md) supplies legality, ownership and pipeline bounds.

## Frozen protocol and reproducibility

Two final campaigns retain all measurements. The initial audit campaigns at
`b7a15e3e` are also preserved as
[initial audit](../student.tests/pa12/final-initial-audit-performance.json) and
[initial stage](../student.tests/pa12/final-initial-stage-performance.json).
A final operand-form review added reversed subscript and data-member-pointer
reference lifetime coverage in `0768b868`; the fresh campaigns below freeze
that revised compiler without overwriting the earlier artifacts:

- [Audit harness](../student.tests/pa12/audit_benchmark.py) and
  [24-workload observations](../student.tests/pa12/final-audit-performance.json):
  completed PA12 entry A versus repaired B, including inherited compiler corpora,
  common class values and three corrected paths.
- [Fixed inherited harness](../student.tests/pa10/benchmark.py) and
  [full-stage observations](../student.tests/pa12/final-stage-performance.json):
  PA11/stage-base A versus final B on nine equivalent compiler corpora and three
  checked long-running executable workloads, plus startup measurements.

| Frozen binary | SHA-256 | `.text` bytes |
| --- | --- | ---: |
| Stage base `91e5dbe0` | `fe662c96e7ed834276169929d4b99e4076331e048867f0c3ceeefb9ea0367280` | 785798 |
| Audit entry `18ef3757` | `7f19ddb9099151715f3a967c6fae5cb9139d9bfbca9d7a6308e9fd08fcc34ed8` | 980166 |
| Initial audit `b7a15e3e` | `99f73e5dda370708fb9de7801ae1274acf601c0cf7282a37d5b36ac9b4f7e848` | 980678 |
| Final implementation `0768b868` | `46aa205bfdc4f9a11fd28aec8e0cef7714ea110c61e280c4e7b03d9ab91e0ccb` | 980742 |

Build flags are the course `g++ -std=gnu++11 -Wall -O3` configuration;
generated output uses `--emit-lowir -O0` (the template signature corpus uses
`--emit-semantics`). The supplied native backend uses `-O0`, SHA-256
`c3bae4acf3243d5a2fd6a15ef00e2d75d11a7d82715b1e4771f55165d0542490`.
JSON records exact binary/input paths, hashes, platform, CPU affinity and flags.
Each common workload has warmup, four A/A noise observations, then two ABBA
blocks pinned to CPU 0. Validation, telemetry and backend compilation are
separate from compiler timings; execution is measured separately. All checked
common outputs, including native images, are byte-identical between A and B.

Tables use medians of the four observations per binary in the paired blocks;
RSS is the maximum across all timed observations, including A/A. Ratios are
block mean B / mean A; JSON retains raw spread and outliers. The shared harness's
console summary also includes A/A in its A median; it is not the table metric.
Compiler text is ELF `.text`. The supplied native images are sectionless, so
their size metric is executable payload after entry, including support/data,
not a claimed isolated native `.text` section.

The three repaired paths are measured B-only with warmup and six observations.
A violates their semantics, so its timing cannot establish optimization profit.
Their absolute measurements characterize current cost and scaling. Runtime
loops use volatile bounds, varying inputs and checked sums/object counts or
initialized representations; class/reference/volatile loops run 12 million
iterations, heap allocation 40,000. Inherited calls/memory/floating loops use
96/64/32 million iterations. Tiny compile/startup timings make no speed claim.

## Final fixes versus completed PA12

| Compiler workload | Seconds A / B | Peak RSS A / B, KiB | B/A pairs | A/A range, seconds |
| --- | ---: | ---: | --- | --- |
| calls-4 | 1.740198 / 1.707856 | 306164 / 306204 | 1.009 / 0.952 | 1.670642–1.763837 |
| memory-float-4 | 1.468198 / 1.512926 | 249380 / 249360 | 0.968 / 1.035 | 1.417309–1.632299 |
| references-4 | 0.055515 / 0.053389 | 14448 / 14424 | 0.908 / 1.090 | 0.050326–0.052315 |
| template-semantics-4 | 0.262199 / 0.359403 | 37600 / 37468 | 1.042 / 1.585 | 0.261459–0.290564 |
| class-1000 | 0.230796 / 0.231167 | 47420 / 47308 | 1.005 / 0.998 | 0.229684–0.232451 |
| class-4000 | 0.949514 / 0.953517 | 175380 / 176324 | 1.004 / 1.004 | 0.941472–1.024804 |

The template-4 paired rows include A pauses of .426 seconds and B pauses of
.373, .346 and .559 seconds, against an A/A minimum of .261 seconds. The
apparent 37.1% median increase is retained and investigated by the focused
repeat below; it is not silently replaced by a more favorable sample. Calls
and memory also retain slow A observations (.619 and .629 seconds at scale 1).
Compiler text grows **576 bytes (.059%)**; common native output stays identical.

| Common executable | Runtime seconds A / B | B/A pairs | A/A range, seconds | Payload A / B, bytes |
| --- | ---: | --- | --- | ---: |
| class | 0.256775 / 0.256663 | 0.998 / 1.023 | 0.255173–0.267422 | 315 / 315 |
| calls | 0.479164 / 0.477014 | 1.002 / 0.989 | 0.476118–0.476808 | 206 / 206 |
| memory | 0.279100 / 0.279587 | 1.007 / 0.999 | 0.278400–0.281212 | 434 / 434 |
| floating | 0.331524 / 0.331560 | 1.000 / 1.009 | 0.329988–0.331622 | 230 / 230 |

All these executables peak at 256 KiB. Compilation of each runtime source is
.0055–.0058 seconds and approximately 5 MiB RSS; individual rows remain in JSON.
Native byte identity establishes that apparent common-path timing differences
are execution noise, not changed generated code. Empty startup takes
0.003203 / 0.003156 seconds; its payload stays 24 bytes. No native speed claim is made.

| Corrected B-only compiler path | Small / large seconds | Small / large peak RSS, KiB | Measured work small / large |
| --- | ---: | ---: | --- |
| Conditional subobject references, 100 / 400 copies | 0.035286 / 0.126470 | 10360 / 28116 | binding visits 500 / 2000; alternatives 200 / 800; IR instructions 8200 / 32800 |
| Qualified lists, 1000 / 4000 copies | 0.091414 / 0.368145 | 21428 / 72868 | binding visits 1000 / 4000; IR instructions 18000 / 72000 |
| Typed heap zero, 100 / 400 copies | 0.034325 / 0.122208 | 10092 / 25900 | zero plans 200 / 800; IR instructions 7900 / 31600 |

| Corrected executable | Compile seconds / peak KiB | Runtime median [range], seconds | Runtime peak KiB | Payload bytes |
| --- | ---: | --- | ---: | ---: |
| Conditional reference | 0.006029 / 5080 | 0.161229 [0.160842, 0.162581] | 256 | 521 |
| Volatile list | 0.005765 / 5048 | 0.090970 [0.090522, 0.091337] | 256 | 232 |
| Typed heap zero | 0.005899 / 5068 | 0.237348 [0.235374, 0.238482] | 160000 | 575 |

The heap runtime repeatedly allocates through the supplied backend's allocation
support; its 160000 KiB peak is disclosed, not attributed to compiler memory or
hidden by reporting only IR size. The corrected lowering adds no allocation to
the source program. Required typed initialization and reference destruction
have no valid zero-work alternative. All three fourfold source expansions have
linear work/IR counts and sub-fourfold peak RSS; dynamic heap extent and nested
conditional properties independently check the pipeline growth bounds.

## Whole-stage comparison

| Compiler workload | Seconds base / final | Peak RSS base / final, KiB | B/A pairs | Base A/A range, seconds |
| --- | ---: | ---: | --- | --- |
| calls-4 | 1.650700 / 1.686478 | 302980 / 305436 | 1.051 / 1.009 | 1.640966–1.649742 |
| memory-float-4 | 1.455484 / 1.417259 | 245848 / 249224 | 0.976 / 0.964 | 1.365959–1.377233 |
| references-4 | 0.050167 / 0.050942 | 13608 / 14352 | 1.017 / 1.017 | 0.049948–0.050371 |
| template-semantics-4 | 0.258646 / 0.263482 | 36928 / 37640 | 1.023 / 1.010 | 0.257858–0.262220 |
| references-8000 | 0.115312 / 0.118400 | 25560 / 25904 | 1.026 / 1.027 | 0.115397–0.116809 |

The large call corpus costs **2.17%** more compile time and **0.81%** more peak
RSS by these measures across the whole stage. The initial campaign at `b7a15e3e`
measured +4.35% time and +6.00% peak RSS; those observations remain linked above.
Required class/lifetime/initialization state and work are bounded, without an
identified avoidable retry or duplicated graph. Fourfold source scaling and the
larger reference corpus remain consistent with linear work. The stage-base
comparison's template-4 pairs are 1.023/1.010, unlike the noisy adjacent-compiler
campaign. No claim that the complete stage speeds up compilation is made.

| Whole-stage executable | Seconds base / final | B/A pairs | Base A/A range, seconds | Payload bytes, unchanged |
| --- | ---: | --- | --- | ---: |
| calls-long | 0.551312 / 0.496963 | 0.938 / 0.970 | 0.476235–0.751132 | 206 |
| memory-long | 0.281170 / 0.280983 | 0.968 / 0.999 | 0.279278–0.373336 | 434 |
| floating-long | 0.330837 / 0.330695 | 0.999 / 0.999 | 0.330405–0.330827 | 230 |

All native image hashes match, with 256 KiB peak RSS. The initial stage campaign
had calls .714533/.778037 seconds in paired medians, ratios 1.088/1.292 and
34.48% A/A spread; this repeat reverses the apparent timing direction. Both
campaigns remain. Identical images and wide A/A spread preclude attributing
these fluctuations to generated-code changes or claiming a runtime improvement.

Full-stage compiler text grows **194944 bytes (24.81%)** for the required PA12
functionality. The inherited shared harness records a 128 KiB text-growth
target plus latency/RSS/scaling targets. Those self-selected diagnostic targets
are not mandated PA12 limits. The text target is exceeded and is explicitly
reclassified as a review signal: PA12 adds 196 required passing cases and the
class-value machinery, with no corresponding common native code growth or
unbounded transform. Removing required semantics to meet the target would
violate the handout. The target and its miss remain in the original JSON.

## Focused noise follow-up

The [focused harness](../student.tests/pa12/audit_noise.py) and
[raw observations](../student.tests/pa12/final-noise-performance.json) repeat the
same frozen A/B, input hashes, flags and CPU with fresh warmups, four A/A rows
and two ABBA blocks. It additionally records user/system CPU time and context
switches; all output hashes match the original campaign.

| Compiler workload | Seconds A / B | Peak RSS A / B, KiB | B/A pairs | A/A range, seconds |
| --- | ---: | ---: | --- | --- |
| template-semantics-1 | .069059 / .069695 | 12696 / 12680 | 1.018 / .915 | .068168–.069346 |
| template-semantics-4 | .263168 / .278050 | 37640 / 37556 | 1.054 / 1.447 | .262811–.265031 |
| calls-4 | 1.712952 / 1.685376 | 306260 / 306272 | .975 / .997 | 1.666039–1.762846 |

Template-4 still has a B wall-time outlier: .496160 seconds, but its child CPU
time is .25 seconds, the same as a .265412-second B observation and ordinary
A observations (CPU accounting has .01-second resolution). A separate B row
uses .28 CPU seconds and .286604 wall seconds. Thus the large apparent wall loss
is not repeatable extra compiler CPU work; the exact cause of time outside
that accounting is not established. The elapsed observations remain part of
the result. The repeat's template median increases 5.65%, versus 37.1% in the
broader run, while the independent full-stage template pairs are 1.023/1.010.
Peak memory and semantic output remain stable. No template speed benefit,
precise zero-overhead claim or microarchitectural diagnosis is asserted.

The unchanged template source has no local reference declaration, so the final
projection walk does not execute on that path. Together with unchanged work
ownership, the recorded CPU times and the stage-base comparison, the evidence
identifies no avoidable algorithmic regression from the final repair. The noisy
wall measurements remain diagnostic evidence, not an invented numeric PA12 gate.

## Earlier optimization and acceptance ledger

All 15 checkpoint reports and 39 historical JSON campaigns were reviewed,
including discarded policies and their losses. They supplement these fresh
comparisons; no smaller-IR result alone establishes profitability.

| Handoff evidence | Audited disposition |
| --- | --- |
| [Members](performance.md), [transfers](transfer-performance.md) | Empty destructor effect omission has a repeatable native gain. Optional scalar-only copying prefixes lost 26–112% runtime and were removed; final outputs match fieldwise A. Required union/storage prefix remains about 90% slower under the supplied backend and stays a required O0 representation, without a speed claim. |
| [Values](value-performance.md), [conversion/reference](conversion-performance.md) | Nested conditional records share selected facts instead of rebuilding subtrees (1060880 to 8208 records in the deep case); compiler scaling improves with unchanged executable output. Static references and conversions retain their selected typed facts and independently checked executable outcomes. |
| [Allocation](allocation-performance.md), [lists](list-performance.md), [boundaries](boundary-performance.md) | Selected layout/list/extent facts drive bounded typed lowering. Required large-class transport retains a measured 44.5% runtime cost and 88-byte growth; correctness-only comparisons are not optimization claims. The final audit closes reference-subobject, cv-list and typed-heap handoffs these earlier campaigns missed. |
| [Cleanup](cleanup-performance.md), [destruction](destruction-performance.md) | Proved no-throw/empty effects and bounded suffix sharing retain repeatable native wins where measured. A redundant reference-initializer guard was removed (condition loss 37% to 7.6%); required branch cleanup remains about 20% slower. Inline suffix work is capped at eight actions, followed by sharing. |
| [Member pointers](member-pointer-performance.md), [zeroing](zero-performance.md) | Required typed null/copy representation and zero boundaries remain, including measured 3.92% zero-path cost. No byte-zero shortcut is accepted for data member pointers. |
| [Consumption](consumption-performance.md) | The 25x loop loss was isolated by the frozen native data-relocation control: original B 4.85777 seconds, separated data .19147, unchanged instruction positions/opcodes except four data-address fixups and segment sizes. A remains about .188. This diagnoses supplied-backend code/data cache-line sharing; the diagnostic images are not compiler output policy. Placement belongs to PA24. |
| [Storage/conversion](storage-performance.md) | Intermediate explicit conversion boundaries cost about 2.5x; fewer bytes via direct storage did not help. Selected nonempty trivial-result elision restores runtime (.25780 to .10575, paired .412/.400) with 311 to 277 bytes. Empty/nontrivial explicit boundaries and constructor allocation-unit ordering remain. |
| [Parameter queries](parameter-performance.md) | Independent body/ABI queries retain required object transport: focused runtime +59%, payload +24 bytes. One class fact and an incremental entity cursor bound query work; this necessary ABI cost is not an optional optimization profit claim. |
| [Scalar consumption](scalar-performance.md) | Optional dynamic/modified cleanup consumption lost 8–20% and was removed; final nonconstant native bytes equal A. The required constant-condition O0 ordering retains +12.1% runtime despite 1600 to 1424 bytes. Its microarchitectural cause is not established; no invented backend diagnosis excuses it. |

The 4/6/8 KiB feature text targets and 5% common compile targets in later
checkpoint reports are likewise diagnostic, not new exit criteria. Historical
misses, outliers and required costs remain visible. Stage-scoped acceptance
preserves correctness, exact course comparison and mandated expansion limits;
it removes unprofitable optional transforms and does not impose positive-runtime
or self-hosting/native-backend gates on PA12.

## Evidence integrity and commands

`audit_verify.py` checks **6488 historical wall observations**, **934 historical
IR/native output hashes**, all recorded frozen compiler/input path hashes, both
final campaign binaries, ABBA order/recorded ratios and final native outcomes.
Two A output pairs in `storage-elision-performance.json` had been overwritten
by reuse of scratch (`explicit-400` and `explicit-runtime`). They reproduce
exactly from the original frozen A and input in a separate evidence directory,
including original IR/native hashes and checked exit. Observations and old
files are untouched. The final audit's 13 executable images and both stage campaigns' six images
execute successfully again. The focused noise repeat verifies the same frozen
inputs/compilers/outputs and retains its CPU/context-switch observations.

Artifacts live in `$RALPH_ARTIFACT_DIR/pa12-final-audit/`. Reproduction uses new
output directories; do not reuse a historical campaign's scratch:

```sh
python3 student.tests/pa12/audit_benchmark.py "$RALPH_ARTIFACT_DIR/pa12-final-audit/baseline-cppgm++" "$RALPH_ARTIFACT_DIR/pa12-final-audit/candidate-complete-cppgm++" "$RALPH_ARTIFACT_DIR/pa12-audit-repeat" "$RALPH_ARTIFACT_DIR/pa12-audit-repeat.json"
python3 student.tests/pa10/benchmark.py delta /tmp/pa12-stage-base-cppgm "$RALPH_ARTIFACT_DIR/pa12-final-audit/candidate-complete-cppgm++" "$RALPH_ARTIFACT_DIR/pa12-stage-repeat.json" "$RALPH_ARTIFACT_DIR/pa12-stage-repeat" 91e5dbe0
python3 student.tests/pa12/audit_noise.py student.tests/pa12/final-audit-performance.json "$RALPH_ARTIFACT_DIR/pa12-noise-repeat" "$RALPH_ARTIFACT_DIR/pa12-noise-repeat.json"
python3 student.tests/pa12/audit_verify.py
python3 student.tests/pa12/audit_check.py
```

The final compiler matches both frozen B hashes. No performance finding remains
as an unresolved PA12 correctness, architecture, mandated-limit or optional
regression blocker. Later native/self-hosting ownership remains explicit.
