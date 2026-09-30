# PA26 audit144 performance checkpoint

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
Self-hosting remains the PA34 surface, not a new PA26 performance gate. The
unresolved output-name contract defect remains required work independently of
these favorable memory measurements and passing course tests.
