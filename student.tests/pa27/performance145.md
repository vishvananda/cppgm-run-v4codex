# PA27 implementation145 performance evidence

Frozen entry `f833cf1f` and final implementation `96e1cb72` binaries, exact flags,
source hashes, host linker version, every observation and phase/work counters
are in [final-performance.json](evidence145/final-performance.json).
[performance145.py](performance145.py) regenerates the fixed inputs and checks
the three inherited input hashes against PA26's retained manifest. All produced
programs return the checked result before timing and on every timed execution.
`argc` drives the loops; checked sums prevent dead/constant workloads.

Each compiler/runtime measurement uses A/A noise calibration and six wall-time
ABBA blocks: **224 final observations**. Compiles are 150–250 ms and programs
48–300 ms, comfortably above process startup. Linking is excluded from compiler
timing. Compiler flags are `-O0 -c --stats`. Runtime rows measure the separately
host-linked executable, not the compiler. Ordinary calls, loops, memory, floating
point, EH and 2,400 demanded template specializations are exercised. Self-hosting
remains PA34's boundary; no new claim is made about it.

| Workload | Compiler median ms A/B | Peak compiler KiB A/B | Runtime median ms A/B | Executable text bytes A/B |
|---|---:|---:|---:|---:|
| Templates + memory/calls/loops | 176.25 / 179.29 | 29056 / 29196 | 53.21 / 52.91 | 151633 / 151633 |
| Templates + floating point | 162.25 / 166.87 | 29064 / 29308 | 48.25 / 48.52 | 151474 / 151474 |
| Templates + exception lifetimes | 161.88 / 172.87 | 29360 / 29520 | 272.46 / 271.61 | 151781 / 151781 |
| Above memory loop + 1,200 unused local functions | 208.54 / 208.39 | 35120 / 35256 | 65.81 / 65.08 | 182449 / 151633 |

Paired compiler B/A medians (range): memory **1.010 (0.857–1.216)**,
floating **1.036 (0.826–1.055)**, exceptions **1.085 (0.969–1.116)**,
pruning **0.973 (0.784–1.052)**. Runtime paired medians are respectively
**0.968, 0.999, 1.022, 0.999**; full ranges and A/A noise samples remain in the
JSON. Do not interpret the noisy changes as compiler or runtime speedups.
Disclosed costs include the EH workload's +8.5% paired compiler median and its
+2.2% runtime median ratio, both with spread crossing 1. Common executable text
is unchanged. Required pruning removes **30,816 text bytes (16.9%)** and reduces
the executable from 499,696 to 375,944 bytes; the workload remains live and equal.

## Measured correction and stage acceptance

The [first 224 observations](evidence145/first-performance.json), at `c36f3501`,
exposed empty relocation sections for leaf COMDATs. `96e1cb72` makes creation
lazy while preserving groups containing actual body relocations. On the memory
input this reduces the object from **1,168,328 to 933,720 bytes**, with identical
executable text. This avoids unnecessary structural growth. The entry object
was 466,960 bytes because it lacked the required groups; the remaining increase
is ELF section/group/symbol metadata, not added executable instructions.

The explicit budgets are structural: at most one body section and group per
mergeable definition; one relocation section only per section with actual
fixups; one placement per symbol; one demand transition/body visit per reached
symbol. Section ownership sorts at O(S log S), placement/fixups are O(bytes+R),
and demand is O(symbols+operands). No search/fixed-point optimizer or code growth
transform was introduced. Unsplit byte lanes move without copying; split lanes
compact ordinary bytes in place, transferring each grouped body once. All
temporary indexes and buffers belong to the object/TU and die after writing.

COMDAT, section and GOT costs are necessary host semantics. The common correct
executables are the A/B comparison boundary; the old compiler's missing ELF
contract support is not treated as a correctness alternative. No optional
transform is retained on an unsupported profit claim. Inherited blanket 15%
latency/RSS and zero-growth diagnostics remain diagnostics under spec §9, not
new exit gates. All historical measurements, mandated limits and correctness
coverage remain intact. These observations support this implementation handoff;
independent performance and architecture review is still required.

Frozen binaries and raw command outputs are also retained in
`$RALPH_ARTIFACT_DIR/pa27-145/`; no binaries, object files or raw logs are committed.
