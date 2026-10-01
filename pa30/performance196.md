# PA30 implementation196 performance and ownership

Implementation: `b7b1436a` and `2f1595b2`; entry: `c67da7c8`.
[Source binding](../student.tests/pa30/evidence196/source-binding.json) retains
all tested implementation hashes, fixed compiler hashes and personal inputs.
[Verifier](../student.tests/pa30/verify196.py) checks unchanged coverage, required
check outcomes, frozen binaries/sources, correct results, all ABBA orders,
paired ratios, linear work counters and equivalent output images: **105 checks pass**.

## Protocol and stage acceptance

Frozen A/B binaries use `-O0 -c --stats`, monotonic wall time and `/usr/bin/time`
peak RSS, with compiler and executable measurements separate. CPU affinity was
not pinned; the host is shared and unisolated. No other compiler build, test
suite or benchmark overlapped the measured runs. All observations and outliers
are retained without startup subtraction. Host g++ links student-produced
objects; it supplies no compiler implementation or output.

[Common data](../student.tests/pa30/evidence196/common-performance.json): 224
observations, four inherited fixed template/loop/memory, floating, exception/call
and unused-definition workloads. Each mode has one four-observation A/A block
and six ABBA blocks. All four object/executable pairs are byte-identical, and
all runtime-input-dependent checks pass.
[Owner data](../student.tests/pa30/evidence196/owner-performance.json): 48
observations plus 16 launchers, eight compile and eight checked runtime samples
at each of 128/512/2048 current-instantiation families. A rejects these sources;
its recorded failures are not timing baselines.
[Hosted data](../student.tests/pa30/evidence196/hosted-performance.json): 60
compiles, four per newly passing fixture. Total: **332 observations + 16 launchers**.

The mandated **45 seconds per compile** remains unchanged. No optimization,
work-search policy, code-growth allowance or new container is introduced.
Inherited blanket percentage/no-growth targets remain diagnostic under spec §9,
as recorded in PA29 and handoff195. This preserves all historical measurements,
mandated limits, correctness and coverage. No speedup is claimed. PA32/33
optimization acceptance and PA34 self-hosting remain owned by those stages.

## Equivalent fixed workloads

Times are medians in milliseconds, RSS maxima in KiB. Paired B/A ratios show
median and complete block range; raw data also retain A/A calibration.

| Workload | Compile A → B ms | RSS A → B KiB | Compile ratio [range] | Runtime A → B ms | Runtime ratio [range] | Object text bytes |
|---|---:|---:|---|---:|---|---:|
| memory | 169.22 → 169.58 | 29620 → 29776 | 0.996 [0.977, 1.478] | 51.14 → 51.12 | 0.996 [0.984, 1.029] | 151393 |
| floating | 168.00 → 167.75 | 29700 → 29996 | 0.993 [0.968, 1.428] | 47.65 → 47.67 | 1.001 [0.995, 1.064] | 151234 |
| exceptions | 168.96 → 168.90 | 29172 → 29372 | 0.983 [0.637, 1.002] | 252.35 → 251.69 | 0.996 [0.953, 1.022] | 151541 |
| pruning | 211.98 → 213.15 | 35660 → 35868 | 0.961 [0.864, 2.065] | 50.99 → 51.01 | 0.997 [0.988, 1.012] | 151393 |

All paired ranges cross unity. Sporadic compiler outliers occur on both A and B
(the pruning maximum paired ratio is 2.065); no repeatable regression or
speedup is established. Identical output images preclude a generated-code
change on these workloads. There is no optional transformation to justify.

## Current-instantiation owner and data flow

`expression_query` resolves a construction's declaration once, then obtains its
canonical injected-class-name type from `injected_template_type`. That owner
caches by class entity and source-head parameter slice; initial work scales with
the actual parameter tuple, subsequent lookups are O(1) average. Primary,
partial and nested classes retain their real dependent arguments. An explicit
specialization keeps its own type.

For a qualified member query, `current_instantiation_scope` follows lexical
parents and compares canonical type IDs. It obtains the retained source scope
for the current instantiation instead of querying an empty symbolic
specialization scope. Work is O(lexical depth), with cached injected types and
indexed member lookup; unrelated scopes/declarations are not visited. The
query stores the resulting qualifier type and complete source context.

Query identities, substitution frames, completion states and ownership remain
in the translation-unit Analyzer. Existing source/query and frame/query caches
share completed facts; the two changes add no invalidation, retry or grammar
replay. The retained graph feeds existing semantic checks and direct typed
LowIR, per-function native lowering and ELF. Evaluated and explicit LowIR
adapter controls verify the same construction/member behavior through both
paths. Existing declaration and access checks remain active.

| Families | Compile median [range] ms | RSS KiB | Runtime median [range] ms | Query work | Class completions | Object text bytes |
|---|---:|---:|---:|---:|---:|---:|
| 128 | 258.92 [173.98, 271.08] | 25392 | 104.60 [97.77, 111.37] | 3716 | 256 | 44240 |
| 512 | 958.44 [705.96, 1058.32] | 78660 | 83.86 [82.15, 93.17] | 14852 | 1024 | 175952 |
| 2048 | 2796.28 [2684.93, 2933.10] | 293208 | 83.23 [82.19, 83.62] | 59396 | 4096 | 702800 |

At all scales and on every compile, query work equals unique queries = 29N+4;
class completions = 2N. Runtime receives seed `7` through argv and checks three
million state transitions through construction, swap and qualified member calls
against independently computed checksums. Each namespace also owns an emitted
probe, so the measured text growth corresponds to additional demanded code.
The A implementation rejects these inputs; correctness cost is reported without
claiming a benefit against failed compilations.

Compile launcher: median 4.66 ms [4.41, 7.20].

Runtime launcher: median 5.30 ms [3.11, 13.99].

## Newly passing hosted inputs

PA30 requires objects for these fixtures. Executable performance is measured
by the checked controls and common/owner benchmarks above. Every fixture's
four emitted objects have identical hashes.

| Fixture | Compile median [range] ms | RSS KiB | Object text bytes |
|---|---:|---:|---:|
| 500-compressed-pair-padding-instantiation | 535.92 [527.96, 544.86] | 35636 | 2430 |
| 600-allocator-deallocate-included-class-layout | 788.01 [783.85, 957.96] | 46092 | 2558 |
| 600-anonymous-allocator-traits-pointer | 803.60 [798.70, 805.51] | 45776 | 2430 |
| 600-const-unordered-map-find | 969.41 [966.03, 999.85] | 62672 | 18847 |
| 600-forward-list-dependent-size-type-compile | 875.17 [855.50, 897.02] | 50564 | 2430 |
| 600-hosted-pointer-traits-pair-pointer-to | 1140.06 [1128.89, 1169.71] | 66612 | 23734 |
| 600-hosted-unordered-set-key-compile | 583.98 [581.32, 591.59] | 40612 | 86 |
| 600-unnamed-nested-enum-allocator-pointer | 1247.93 [1239.04, 1265.79] | 74424 | 21163 |
| 700-hosted-allocator-destroy-body-compile | 1092.76 [1063.96, 1111.21] | 64052 | 21326 |
| 700-hosted-inherited-member-template-unevaluated-lookup | 556.84 [545.49, 561.95] | 38416 | 102 |
| 700-hosted-map-iterator-operator-lookup-compile | 1360.70 [1346.29, 1379.14] | 83096 | 27127 |
| 700-hosted-shared-ptr-rvalue-assignment-compile | 816.32 [808.03, 822.95] | 51452 | 8314 |
| 700-hosted-unevaluated-member-probe-named-demand | 780.49 [779.45, 786.18] | 46148 | 2430 |
| 700-unordered-map-reference-wrapper-hash-invocable | 680.10 [664.53, 780.74] | 46568 | 1810 |
| 700-unordered-set-const-range-insert-compile | 989.88 [960.74, 1033.47] | 65332 | 28127 |

Largest hosted observation: **1.379 s**, peak RSS **83,096 KiB**. No timeout or coverage relaxation.
