# Scalar native handoff performance

Stage scope is PA24 integer/pointer execution and native selection at O0. Floating
execution, source-driven native compilation and self-hosting remain unfinished;
these measurements do not claim their acceptance. The original scaffold could
produce no executable, so the comparison uses two correct intermediate/final
implementations, not failing output.

Frozen artifacts: `/home/vishvananda/work/private/v4codex/artifacts/pa24-handoff127/`.
The accepted experiment is `accepted/observations.json`, SHA-256
`b6174714536b85245935edd14c1e380322eb1144e94dae7498ef426a4bd4253a`.
It retains every sample, phase counter, hash and paired result. Earlier observations
in the root, `final/`, `handoff/` and `reviewed/` are retained, including the
regression that motivated the final correction. No timing observations were deleted.

- A: `A`, SHA-256 `ccde07718e06e35ce117d975960b80787787067c8025cb4463e34d50fa381468`.
- B: `B-accepted`, SHA-256 `9e7181256b23c885e2b4c07f2be196285ed97a6797c3f424258217f238bdb3c3`.
- Compiler flags: `-O0 --stats -o OUTPUT INPUT`; host build: GNU C++11, `-O3`.
- Fixed compiler input: 4,096 helpers, 64 arithmetic operations each, plus checked
  entry; SHA-256 `8a6e5de91783f08bdd73eddf76e68cf4c533c2018bf18b3b0a97667497724f5c`.
- Runtime inputs are frozen alongside it and in this directory. With one extra
  process argument both execute 100,000,000 iterations; two argument counts are
  checked before measuring. Each result is compared against the runtime count.
- CPU affinity: logical CPU 0. Four A/A observations and six wall-time ABBA
  blocks per workload. Compilation and execution are measured separately.

| Measurement | A | B | Paired B/A median |
|---|---:|---:|---:|
| Compiler wall median | 0.845 s | 0.804 s | 0.959 |
| Compiler maximum RSS | 88,092 KiB | 88,100 KiB | essentially unchanged |
| Forward-edge/call runtime median | 0.721 s | 0.424 s | 0.592 |
| Forward-edge executable text | 176 bytes | 161 bytes | 0.915 |
| Memory/call runtime median | 0.518 s | 0.494 s | 1.127 (noisy identical code) |
| Memory/call executable text | 149 bytes | 149 bytes | identical bytes |

Compiler wall ranges were A 0.650–0.930 s and B 0.706–0.839 s; A/A was
0.823–0.827 s. Compiler paired ratios ranged 0.893–1.036. There is no compiler
speedup claim: scheduling variation exceeds the small median difference. Maximum
RSS meets the diagnostic 15% budget; parser/input storage dominates process peak
RSS even though placement state now dies per function. Large-input native text
fell from 1,167,413 to 1,105,973 bytes with identical checked results.

Forward-edge runtime ratios were 0.584, 0.604, 0.601, 0.580, 0.557 and 0.717:
all six blocks improved. A/A range was 0.812–0.835 s; measured ranges were
A 0.689–0.822 s and B 0.410–0.589 s. Retaining a scalar over the forward edge
removes its frame store/load and frame allocation. This is a repeatable runtime
benefit with smaller code and bounded compiler work, meeting the local budget.

The earlier `reviewed/` experiment revealed an avoidable 17.9% paired memory-loop
regression: an observed global address acquired a spill home. B now rematerializes
constant symbol addresses when no retained register covers their interval. The
accepted A and B memory executables have the same SHA-256:
`fb9a683e39bdc31c2c11fcb68674fdb746acdeafd6ae37436e013185e88b2e3f`.
Their text/frame sizes are restored, not merely their results. Accepted memory
paired ratios ranged 0.673–1.513, with A/A 0.314–0.354 s and measured A
0.317–0.790 s, B 0.357–1.050 s. Those wall differences cannot establish a
compiler-induced runtime change for byte-identical executables. No memory-runtime
speedup is claimed and no noisy ratio creates an additional stage exit gate.

Runtime peak RSS was 256 KiB throughout. ELF inspection confirms separate RX and
RW load segments. The integration suite checks native/MIR byte consistency and
parallel phi transfer growth; all successfully compiled course programs are also
checked independently of any earlier MIR comparison failure.
