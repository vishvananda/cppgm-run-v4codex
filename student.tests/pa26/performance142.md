# PA26 performance evidence, implementation142

Code: `041cf554f499c4574b72294d23af897459e0147a`. Host objects are new required
behavior; the entry binary writes non-linkable private bytes to `.o`, so there
is no correct entry host executable for an A/B speedup comparison. No optional
optimization was introduced or performance improvement claimed.

## Frozen protocol

- Entry: `/tmp/pa26-142/entry-cppgm`, built at the stage base.
- Candidate: `/tmp/pa26-142/final-cppgm`, code `3f7bc44e`.
- Accepted: `/tmp/pa26-142/accepted-cppgm`, code `041cf554`.
- Build: `g++ -std=gnu++11 -Wall -O3`; exact binary/input/image hashes are in
  the JSON manifests. Final source generators are checked in here and in PA25.
- Each workload/mode has an AAAA calibration block and six wall-time ABBA
  blocks. All 672 candidate/final observations are preserved in
  [evidence142](evidence142/). RSS comes from `/usr/bin/time`; host phase
  counters come from `--stats`, which reads existing work. No observations
  were filtered. Compiler work and executable runs were measured separately.
- Legacy comparison reuses `student.tests/pa25/performance.py` at O0: 4800
  template specializations across eight TUs plus a main TU; memory/call and
  floating workloads each execute three million runtime-driven iterations.
  Short compiler cases use batches of 64; runtime batches contain three runs.
- Host baseline uses `student.tests/pa26/performance.py OUT FROZEN_BINARY`:
  `-O0 -c --stats`, then external `g++` linking outside the compiler timer.
  Every input demands 2400 templates; loops/calls/memory, floating point and
  200,000 throw/catch/cleanup operations have independent checked results and
  an argc input. Host A and B are the same frozen accepted binary. Compiler
  samples take 145+ ms, so they dominate launch overhead. Runtime samples
  take 47+ ms. These supplement, rather than replace, course checks.

## Final legacy A/B

All three entry/final executables are byte-identical. The table reports paired
B/A median (six-block range); changes in runtime timing cannot be caused by a
changed generated instruction stream. Compiler batches include process launch.

| Workload | Compiler B/A | Compiler peak RSS A/B KiB | Runtime B/A | Text bytes A=B |
|---|---:|---:|---:|---:|
| templates | 0.908 (0.716–1.037) | 14684/15284 | 0.999 (0.891–1.066) | 384,567 |
| memory | 0.950 (0.853–1.014) | 6784/6956 | 0.999 (0.760–1.056) | 521 |
| floating | 0.961 (0.815–1.551) | 6972/7108 | 1.000 (0.988–1.003) | 362 |

No repeatable compiler regression is established. In particular, the floating
compile range contains a 1.551 outlier; the same-binary host calibration below
also has a 1.768 outlier. The whole distribution and A/A ranges remain in JSON.
Candidate and final batches ran at visibly different machine throughput; do
not attribute their absolute timing difference to the small storage guard.

## Final host baseline

| Workload | Compile median ms (range) | Compiler peak KiB | Runtime median ms (range) | Runtime peak KiB | Object/executable text bytes |
|---|---:|---:|---:|---:|---:|
| memory | 156.7 (146.0–204.5) | 28796 | 51.0 (50.4–52.4) | 1472 | 151,393/151,633 |
| floating | 147.5 (145.0–271.8) | 28888 | 47.9 (47.3–51.1) | 1384 | 151,234/151,474 |
| exceptions | 151.5 (145.3–380.0) | 28768 | 252.6 (250.6–269.4) | 3908 | 151,538/151,778 |

Host final/final compiler paired medians are 0.973, 0.986 and 1.015;
runtime medians are 0.997, 1.005 and 0.991. They measure noise, not profit.
All candidate/final host input and image hashes agree. The exceptions object
contains 96,248 bytes of CFI, 26 bytes of LSDA and 16 bytes of indirect runtime
references. Its 151,538 text bytes include the demanded template workload.
Executable text includes 240 host startup bytes; shared host runtime libraries
are not included in that text-size total. Self-hosting remains owned by PA34.

## Stage-scoped acceptance and budgets

Correctness and PA26 inspection bounds remain mandatory. The backend does two
bounded per-function CFG visits, interns stack/selector facts in flat indexes,
and traverses each native instruction, clause and fixup a constant number of
times. Output growth is proportional to functions, handlers, clauses and call
ranges; no inlining, cloning, unrolling or optional optimizer is added. Shared
resume terminals and call-range coalescing are PA26 requirements, not an
unmeasured optional optimization. There is no numeric latency/RSS ceiling in
PA26; inherited 15% and blanket zero-growth targets remain diagnostics under
spec section 9, with all historical measurements preserved. Host ABI metadata
cost is necessary semantics, not a claim that size bounds prove runtime profit.

The avoidable private-path allocation discovered during implementation was
removed before the accepted freeze. Nine personal host/format controls and a
production demanded-template inspection pass. The unchanged course suite is
29/30; this evidence accepts the completed EH backend group, not the unfinished
header/library and driver migration or the independent whole-stage audit.
