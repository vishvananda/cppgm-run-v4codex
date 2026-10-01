# Implementation189 performance evidence

Frozen entry `54ddacb1` versus final implementation `383fae65`; the initial
assertion implementation `4ed34a87` and access refinement `e08707fc` are
preserved separately. The final source also preserves conditional-specifier
substitution failure.
O0, `-c --stats`, CPU 0 affinity, fixed hashed sources, host g++ link, wall time
and `/usr/bin/time` peak RSS. Compiler and checked executable measurements are
separate. Every equivalent workload has four A/A samples and six ABBA blocks.
The record retains **1,512 observations** (560 final, 560 intermediate, 392 preliminary) and **24**
launcher samples. No observation or outlier was discarded. Preliminary evidence
precedes the access-context extension; intermediate evidence precedes the
substitution-failure refinement. Neither substitutes for final evidence.

[Final common](../student.tests/pa29/evidence189/common-performance.json),
[final owner](../student.tests/pa29/evidence189/owner-performance.json),
[intermediate common](../student.tests/pa29/evidence189/intermediate-common-performance.json),
[intermediate owner](../student.tests/pa29/evidence189/intermediate-owner-performance.json),
[preliminary common](../student.tests/pa29/evidence189/preliminary-common-performance.json),
[preliminary owner](../student.tests/pa29/evidence189/preliminary-owner-performance.json)
contain binary/input hashes, all raw samples, A/A ranges and phase/work counters.

Times are median milliseconds, RSS is maximum KiB, and ratios are paired B/A
block medians with min–max block spread. Text is executable `.text` bytes.

| Workload | Compile A/B ms | RSS A/B KiB | Compile ratio [spread] | Runtime A/B ms | Runtime ratio [spread] | Text A=B |
|---|---:|---:|---|---:|---|---:|
| memory | 168.21/170.11 | 29672/29812 | 1.001 [0.993, 1.259] | 72.45/72.42 | 1.000 [0.986, 1.007] | 151633 |
| floating | 280.70/282.72 | 29660/29868 | 1.001 [0.901, 1.277] | 56.38/53.15 | 0.999 [0.877, 1.006] | 151474 |
| exceptions | 168.72/168.66 | 30040/30244 | 0.998 [0.989, 1.682] | 251.45/251.10 | 1.001 [0.985, 1.012] | 151781 |
| pruning | 207.46/206.26 | 35700/35904 | 0.992 [0.732, 1.000] | 52.89/52.92 | 1.000 [0.996, 1.055] | 151633 |
| demand600 | 158.13/168.76 | 26076/26276 | 1.030 [0.891, 1.057] | 137.56/137.37 | 0.986 [0.952, 1.261] | 825 |
| demand1200 | 291.13/292.51 | 44224/44920 | 1.012 [0.939, 1.134] | 137.48/137.34 | 1.000 [0.974, 1.003] | 825 |
| demand2400 | 592.42/596.55 | 81664/81660 | 1.023 [0.997, 1.086] | 137.00/137.20 | 0.999 [0.998, 1.019] | 825 |
| contexts600 | 63.21/63.96 | 14304/14552 | 1.015 [1.006, 1.033] | 137.27/137.44 | 1.001 [0.996, 1.029] | 825 |
| contexts1200 | 114.16/117.09 | 20736/21340 | 1.028 [1.009, 3.129] | 137.21/136.28 | 0.997 [0.991, 1.003] | 825 |
| contexts2400 | 222.00/228.61 | 34180/34892 | 1.039 [0.647, 1.142] | 140.12/138.19 | 0.990 [0.891, 1.022] | 825 |

All **ten** final object/executable A/B pairs are byte-identical. The seven
preliminary images and all ten intermediate images also survive the final
refinement unchanged. Every program checks runtime inputs/results; loops, calls
and memory/floating/exception
work remain executable. The common suite includes 2,400 demanded template
functions and an unused-function pruning variant. The owner families use a
checked 3,000,000-iteration character-conversion loop and 600/1200/2400 declarations.

`demand` exercises actual primary definitions, member-type demand and source-defined
false traits, with two assertions per specialization. `contexts` exercises
explicit constexpr bool conversions in assertions, noexcept and explicit
specifications. Its entry result happens to be correct for always-true operands;
the new implementation additionally performs the required conversion and access
work, which is necessary to distinguish the false/private controls.

## Work bounds and acceptance

[Verified counters](../student.tests/pa29/evidence189/scaling.json) are identical
across all 12 final-B compiler observations at each family/size. With N declarations:

| Family | Tokens | Parsed nodes | Semantic specialization identities | Candidate work | LowIR instructions / native functions |
|---|---:|---:|---:|---:|---:|
| demand | 36N+323 | 38N+502 | 5N+5 | 35 | 101 / 7 |
| contexts | 37N+340 | 40N+522 | 7 | 3N+35 | 101 / 7 |

Expanded occurrence counts are 191N+636 and 40N+764 respectively. The source is
parsed once; inherited specialization occurrences are recorded separately from
parsed nodes. There are zero function-template body transitions in these owner
families. Dormant methods emit no code. These counters establish linear added
owner work; they are not a claim that an IR count alone proves runtime profit.

The context family includes newly required conversion validation/execution.
Paired wall-time spreads and preliminary/intermediate/final absolute times show
substantial environmental variation. No compiler or runtime speedup is claimed. Runtime
variation occurs between byte-identical images. Ordinary trait-demand medians
and spreads do not establish a repeatable avoidable regression. Successful
assertions reuse their completion record; message rendering runs only on failure.

Compiler text: 4,075,358 → 4,079,702 bytes (+4,344, 0.107%).
This is required parsing, diagnostics and semantic checking. Generated program
growth is zero. No optional transform, optimization-body retention, new global
cache, work budget or growth allowance is introduced. Existing 1,000,000-step /
512-depth constant evaluation, 0x70000000 native frame/data, 4096 alignment and
course timeout limits are unchanged. Self-hosting remains PA34 scope.

Under spec §9, mandatory semantic costs are documented instead of being treated
as an optional optimization requiring runtime profit. Historical blanket 15%
latency/RSS and zero-growth targets remain self-selected diagnostics, not exit
gates. No mandated limit, correctness requirement, comparison or coverage was
weakened. All historical and preliminary measurements are retained.
