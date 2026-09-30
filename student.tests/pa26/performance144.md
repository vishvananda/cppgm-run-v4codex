# PA26 audit144 performance: checkpoint and final acceptance

## Preserved writer checkpoint

Frozen A is entry `0af58619`, SHA-256
`f057adc7ca5ed77791e42e388fc1eb1c9e2d07337c0cfb3637e11737571f6ce7`.
Frozen B is the ELF ownership repair in `8c541d1e`. Both binaries survive in
`/tmp/pa26-144/{entry,writer}-cppgm`; exact B hash, input hashes, flags, phase
counters, image hashes and every observation are in [evidence144](evidence144/).
These measurements evaluate this repair, not an accepted full PA26 disposition.

`writer_performance144.py OUT A B` measures a fixed 32 MiB data object and a
checked executable that changes/reads all 8192 pages over 256 iterations driven
by argc. `performance143.py OUT A B` retains the established template-heavy
memory/call/loop, floating, exception and string workloads. Each mode has four
A/A observations followed by six ABBA blocks. All 280 observations survive;
external host linking is outside compilation timing. Output equality is checked
before sampling and executable results on every timed run. There were no
concurrent compiler builds or test suites from this audit during measurement.

The strings labels both select B, as defined by the inherited harness: that row
is an additional current/current baseline, not evidence for a writer speedup.
The separate 37-object equivalence control includes the required string TU.
Compilation uses `-O0 -c --stats`; RSS uses `/usr/bin/time` directly on the
compiler. Text measurements come from `size -A .text`, excluding shared libraries.

| Workload | Compile median A/B ms | Compiler peak A/B KiB | Runtime median A/B ms | Executable text A=B bytes |
|---|---:|---:|---:|---:|
| 32 MiB storage | 223.14 / 102.94 | 138160 / 40016 | 98.78 / 100.27 | 571 |
| template + memory/calls/loops | 156.51 / 155.42 | 29040 / 29228 | 51.73 / 51.68 | 151633 |
| template + floating | 158.77 / 153.93 | 28608 / 28796 | 48.92 / 48.42 | 151474 |
| template + exceptions | 155.08 / 152.64 | 29364 / 29480 | 259.20 / 256.61 | 151778 |
| strings, B/B | 683.95 / 696.41 | 46232 / 46208 | 540.80 / 559.75 | 35963 |

Storage compiler paired B/A median is 0.432, range **0.040–1.418**; A/A is
204.2–425.9 ms, with an A outlier at 5350.9 ms. File-write timing is highly
variable. No precise repeatable latency gain is claimed. The measured peak-RSS
reduction is **71.0%** on this affected workload: the code change removes
duplicated native-section storage and the extra whole-file buffer.
Common compiler paired medians are 0.959, 0.990 and 0.976; their ranges
are respectively 0.870–1.242, 0.756–4.641 and 0.905–1.429. These also do not
establish compiler speedups. All A/A ranges and individual samples are retained.

Storage, memory, floating and exception objects and linked executables are
byte-identical across A/B. Runtime paired medians are 1.011, 0.994, 0.993 and
0.991; respective ranges are 0.988–1.045, 0.833–1.012, 0.915–1.078 and
0.880–1.044. No generated-runtime improvement is claimed from these noisy
measurements or from IR/node counts. Text growth is zero. Strings B/B paired
medians are 1.018 compile and 1.016 runtime, also within observed variation.

Legality follows exact buffer ownership transfer and unchanged section layout,
symbol ordering, relocation reindexing, zero padding and ELF bytes. Layout and
streaming remain O(section bytes + relocations + symbols), with the existing
stable symbol partition bounded by O(symbols log symbols). There is no new
analysis, semantic invalidation, iterative search, instruction transform or code
growth. Zero-sized sections, 4096-byte alignment, aliases, lifecycle arrays,
CFI/LSDA and failed output writes have explicit controls.

PA26 mandates no numerical compiler latency or RSS ceiling. The inherited 15%
and blanket zero-growth targets remain diagnostic under spec section 9; earlier
misses and all historical142/143 observations are preserved. Mandatory EH
inspection bounds, correctness, comparison and coverage remain required.
Self-hosting remains the PA34 surface, not a new PA26 performance gate. At that checkpoint, the unresolved output-name contract defect remained required
work independently of favorable memory measurements and passing course tests.
The following final comparison covers its completed repair.


## Final frozen comparison (`78410bfc`)

Frozen A remains the original audit entry above. Frozen B is
`55d08c175a29d771e96d63e00c9f166526db0d743e3ca3cdbb9ef48b610d547c`, retained at
`/tmp/pa26-144-policy/accepted-cppgm`. `final_performance144.py` verifies the fixed
source hashes against the checkpoint manifests before measuring. It records
**448 observations**: five host-object workloads and three compiler-owned
executable workloads, each with four A/A observations and six ABBA blocks for
compilation and execution separately. Every executable returns its independently
computed expected result before measurement and during each runtime sample.
The input loops depend on argc and update live memory/results. No compiler build
or correctness suite from this audit ran during these samples.

[Raw final measurements](evidence144/final-performance.json) retain all samples,
spread, calibration, phase/work counters, flags, binary/input/image hashes and
host-tool identity. Compilation uses `-O0 --stats`; `-c` is added for the host
object surface, and host final linking is excluded from that timing. Own-link
compilation includes this compiler's linker. RSS comes from `/usr/bin/time`.
The strings row retains the inherited **B/B** calibration rather than an A/B
improvement claim. The original 280 writer/common observations and earlier
142/143 measurements remain unchanged.

| Surface / workload | Compile A/B median ms | Compiler peak A/B KiB | Runtime A/B median ms | Executable text A/B bytes |
|---|---:|---:|---:|---:|
| host_object/memory | 247.88 / 253.95 | 29084 / 29196 | 66.01 / 69.85 | 151633 / 151633 |
| host_object/floating | 180.29 / 181.74 | 28580 / 28800 | 47.72 / 48.86 | 151474 / 151474 |
| host_object/exceptions | 170.11 / 165.75 | 29404 / 29572 | 375.37 / 422.48 | 151778 / 151781 |
| host_object/strings | 917.54 / 751.37 | 46168 / 46048 | 592.16 / 825.37 | 36009 / 36009 |
| host_object/storage | 195.62 / 98.12 | 138188 / 39996 | 107.41 / 102.81 | 571 / 571 |
| own_link/memory | 233.21 / 203.08 | 28724 / 29244 | 61.39 / 67.95 | 151425 / 151425 |
| own_link/floating | 234.84 / 225.27 | 29088 / 28776 | 51.41 / 54.26 | 151266 / 151266 |
| own_link/exceptions | 159.29 / 167.38 | 28760 / 29656 | 2329.48 / 273.08 | 156071 / 151640 |

The large-object peak reduction persists: **138188 -> 39996 KiB (71.1%)**.
Its paired compile median is 0.517 B/A, but the range is **0.062–8.238** with
multi-second file-write outliers on both binaries; no precise latency speedup is
claimed. Common compiler medians also sit within broad calibration/paired
variation. The host-object EH runtime paired median is 1.161, range
0.709–1.401; the identical-binary strings runtime baseline itself has median
1.107, range 1.023–1.512. These observations do not establish a host-object
runtime improvement or a repeatable slowdown attributable to the repair.
The host-object EH text change is **+3 bytes**, required to retain exception
state through the repaired outer-handler path; plain memory/floating/storage
text is byte-identical. Final source-to-ELF inspection records 463 bytes versus
460 at the writer-only checkpoint, with the same 71 LowIR/82 MIR instructions.

The own-link EH workload has a clear runtime change: paired B/A median **0.115**,
range **0.107–0.145**; medians are 2329.48 -> 273.08 ms. This is the switch from
our private exception runtime to the host ABI/runtime, not an IR optimization.
The result is correct on both fixed inputs and exceeds the observed A/A noise.
The cost is disclosed: compiler peak RSS 28760 -> 29656 KiB; generated-process
peak RSS 256 -> 4060 KiB; executable file size 160208 -> 405376 bytes. Text
falls 156071 -> 151640 bytes, but these figures **exclude shared-library text**
and must not be described as a whole-program footprint reduction.

Own-link memory/floating text is byte-identical. Their file sizes grow by 96160
bytes on the 2400-specialization input because the shared host-object contract
retains final-layout CFI data (about 40 bytes per function); static executable
section collection is not added as an O0 optimization in this PA. Dynamic EH
output additionally carries symbol, relocation and unwind-index tables. This
is bounded linear ABI metadata, disclosed separately from `.text`; it does not
justify a zero-file-growth claim. Runtime ratios on the plain workloads remain
within the noisy sampling regime. No code cloning, inlining, speculative fact,
new search pass or larger optimization work budget was introduced.

The final policy is accepted for PA26: mandated native/EH inspection limits,
all correctness checks and unchanged coverage pass. The memory ownership
repair has a repeatable affected-workload benefit without text/runtime changes.
The uniform ELF repair carries necessary ABI facts and bounded metadata costs;
it is required correctness work, not an optional transform kept for a presumed
speedup. The inherited 15% latency/RSS and blanket zero-growth targets remain
diagnostics under spec section 9, with historical observations preserved.
Neither hosted completeness nor PA34 self-hosting is an additional PA26 exit
gate. [Final ledger](evidence144/final-ledger.json) records exact text hashes,
current counters, compatibility checks and failed-attempt provenance.
