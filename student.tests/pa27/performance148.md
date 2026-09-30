# PA27 audit148 performance

Code tip: `cbe7871e211c284ef4a1c571de12db5f26c29777`.
Final compiler SHA-256: `2dd35668617b07ddd6cb5f24c699d12bbaf02bdb92a06e2c186bbcee26e29fa5`.
Stage-base binary: `55d08c17…`; audit-entry binary: `a2abb98c…`.
Full identities, flags, input hashes, host tool version, image sizes, phase/work
counters, RSS and every observation are retained in
[common](evidence148/common-performance.json) and
[storage](evidence148/storage-performance.json).

The common comparison covers the complete accumulated stage against its base;
the storage comparison isolates this audit's repairs on identical, already
supported source. Both use frozen binaries, `-O0 -c --stats`, CPU 0, a four-run
A/A calibration and six wall-time ABBA blocks per compiler/runtime pair.
Compilation excludes host linking; executable timing runs separately and checks
the expected result on every run. Runtime `argc` bounds and checked sums keep
template/call/loop/memory/FP/EH workloads live. No concurrent build or test suite
ran during the measurements. There are **336 new observations**, plus **1960
preserved historical observations** in the [inventory](evidence148/historical-evidence.json).
No observation was dropped. Self-hosting remains PA34's boundary.

| Input / comparison | Compiler seconds A/B | Compiler peak KiB A/B | Runtime seconds A/B | Text bytes A/B | Paired compile ratio (range) | Paired runtime ratio (range) |
|---|---:|---:|---:|---:|---:|---:|
| memory (stage base → final) | 0.5507 / 0.5823 | 29032 / 29124 | 0.1274 / 0.1349 | 151633 / 151633 | 1.094 (0.827–1.262) | 1.012 (0.746–1.283) |
| floating (stage base → final) | 0.5726 / 0.5548 | 28860 / 28968 | 0.1124 / 0.1100 | 151474 / 151474 | 0.980 (0.872–1.061) | 0.978 (0.832–1.935) |
| exceptions (stage base → final) | 0.5317 / 0.5464 | 29152 / 28848 | 1.0086 / 0.9699 | 151781 / 151781 | 0.990 (0.784–1.178) | 1.110 (0.897–1.390) |
| pruning (stage base → final) | 0.4377 / 0.4440 | 35192 / 35384 | 0.1277 / 0.1253 | 182449 / 151633 | 1.038 (0.736–1.313) | 0.954 (0.903–0.998) |
| construction (audit entry → final) | 0.7728 / 0.7600 | 36148 / 36384 | 0.2948 / 0.2997 | 118696 / 118696 | 0.966 (0.755–1.037) | 0.994 (0.756–1.065) |
| constant (audit entry → final) | 0.7167 / 0.6794 | 77156 / 77340 | 0.0597 / 0.0605 | 439 / 439 | 0.965 (0.822–1.123) | 1.017 (0.980–1.184) |

A/A ranges (seconds):

| Input | Compiler min / max | Runtime min / max |
|---|---:|---:|
| memory | 0.4330 / 0.7006 | 0.1478 / 0.1637 |
| floating | 0.5015 / 0.5727 | 0.0855 / 0.1263 |
| exceptions | 0.4923 / 0.6702 | 0.5649 / 0.8688 |
| pruning | 0.3980 / 0.4529 | 0.1358 / 0.1441 |
| construction | 0.8530 / 0.9283 | 0.4988 / 0.5589 |
| constant | 0.5250 / 0.6583 | 0.0623 / 0.0890 |

The broad paired spreads and A/A variation do not support a precise compiler
speedup or slowdown claim. The audit changes add 236 KiB peak RSS on 600 nested
constructor specializations and 184 KiB on 8192 constant objects. These are
bounded TU-owned projection facts; completed layout is required, and the cache
cannot instantiate declarations or bodies. Constant receiver groups reuse
activation-owned path addresses rather than walking the same prefixes for each
default. No optional optimization pass or executable growth was added.

The audit's construction and constant objects and executable `.text` are
byte-identical A/B. Across the full stage, memory and FP have identical normalized
instruction sequences; layout/relocation addresses differ. EH changes one
`lea` of imported RTTI into the required GOT `mov`, with unchanged instruction
count and text size. Its paired runtime median is 1.110 (0.897–1.390); this cost
is disclosed, with noisy A/A and timed ranges and no asserted precise slowdown.
The instruction difference enforces imported-data identity and PIE safety.
[Text hashes](evidence148/text-comparison.json) and
[instruction comparison](evidence148/instruction-comparison.json) preserve the
comparison; normalization removes instruction addresses, symbolic branch target
addresses and RIP displacements, retaining target names and opcodes.

Required local-function pruning removes 1200 unreferenced functions and
**30,816 text bytes (16.9%)**. The memory object grows from 466,960 to 933,720
bytes because correct COMDAT definitions need body sections, groups and symbol
metadata. Executable text stays at 151,633 bytes. Prior145 already removed
avoidable empty relocation sections; this audit confirms they remain lazy.
The old compiler is a correct executable-behavior comparison for these inputs,
not a substitute for the stage's missing host-object semantics.

Budgets remain structural: one demand visit per reached symbol/body; one group
and body section per mergeable definition; relocation sections only for actual
fixups; O(symbols + operands) reachability, O(S log S) placement sorting and
O(bytes + relocations) movement/writing. Field projections use flat canonical
`(field, receiver-class)` keys with shared tails and a TU release point; constant
storage groups use one address per selected path per activation. Existing
1,000,000-step/512-depth constant-evaluation limits remain unchanged. No new
work/growth budget or speculative code transform is introduced.

Under spec §9, inherited blanket 15% latency/RSS and zero-growth diagnostics do
not add exit gates. All mandated limits, correctness, coverage and observations
remain required. Required ABI metadata and GOT costs are accepted as semantic
costs after removing identified avoidable work. O0 is the current native policy;
accepted O2 controls exercise compatibility, not a claim of PA32/PA33 optimizer
completion. The data show no repeatable avoidable regression from this audit.

Reproduce with `PERF_CPU=0 python3 student.tests/pa27/performance147_common.py
OUT BASE FINAL` and `PERF_CPU=0 python3 student.tests/pa27/performance148.py
OUT ENTRY FINAL`. Frozen executables and inspection files are retained in
`$RALPH_ARTIFACT_DIR/pa27-148/`. An initial launch of the archived base binary
failed before measurement because its executable bit was absent; a byte-identical
executable copy was used. The initial build's stale-object link failure after a
header edit is retained in artifacts; the subsequent complete rebuild and final
validation pass. Neither failed attempt supplied accepted measurements.
