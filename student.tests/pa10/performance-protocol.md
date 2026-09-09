# PA10 evidence protocol

Frozen before measurements. A is the first correct course implementation at
`55b33a44` (implementation `17f3deb7`), copied to `/tmp/pa10-first-correct`.
B is the audited `3af0da70` compiler, built identically by `make -C dev cppgm++`
with g++ GNU C++11/O3 and the course batch runner. Freeze both hashes before
measurement; never build/test concurrently with timing.

Compiler workloads: calls/control (3500/14000 function pairs), floating/memory
loops (3500/14000 functions), static reference chains (800/3200 bindings), and
inherited template semantics (3500/14000 demands via --emit-semantics). Use
--emit-lowir -O0 for procedural workloads. Every A/B output must compare
semantically equal using the unmodified course comparison (byte equal for
semantic dumps). Retain input hashes, all external wall/RSS measurements,
compiler text sizes, and a separate B telemetry run. Pin to one available CPU.
Run AAAA noise calibration then two ABBA blocks per input. Record an empty
input baseline; workloads should exceed startup by 20x. Shorter observations
are diagnostic only, with no speed claim.

Executable workloads: loops/calls, indexed memory, and floating calls. Local
volatile trip counts force runtime reads, and checked final results keep work
observable. This PA's supplied backend exposes a zero-argument executable
entry; command-line-dependent entry behavior is outside its execution surface.
Compile A/B LowIR separately with the same frozen lowir2native-ref -O0, verify
exit 0, record executable hashes/text, and run AAAA plus two ABBA runtime blocks.
Backend compilation is outside compiler latency. No optional generated-code
transform was introduced; identical executable hashes prove no code/runtime
tradeoff is being hidden. Runtime noise is reported without a speed claim.

Self-selected diagnostic budgets: compiler wall <=1.10x A (outside measured
noise), RSS <=1.20x A +16 MiB, compiler text growth <=128 KiB, fourfold input
wall <=5.5x and RSS <=5x +16 MiB. These are not additional exit gates; PA10 has
no mandated numeric speed budget beyond spec.md's complexity/legality rules.
Investigate avoidable regressions. Necessary corrected semantics and audit
instrumentation are disclosed; no historical/self-imposed miss overrides the
stage-scoped acceptance rule. PA9's naming-tool text budgets still apply to
that unchanged tool, not to cppgm++'s newly required lowering implementation.

The base driver emits no IR and is not a valid full-stage A/B comparison.
A/B above measures the ownership/caching audit on correct common workloads;
new volatile-discard, bool-storage and padding fixes are validated separately.
Later template lowering, MIR allocation, ELF writing, and self-hosting remain
owned by their assignments. Earlier template semantics are measured here to
check that extending the procedural frontend has not hidden a regression.

## Measurement continuation (same frozen binaries)

The first campaign completed all 96 compiler observations, then encountered
sectionless ELF at executable text-size measurement. Preserve `performance.json`
unchanged and carry those observations into `final-performance.json`. The
corrected metric counts bytes from ELF entry to the executable PT_LOAD end;
these three native inputs have no static data. Compiler .text remains the
ordinary section size. This matches PA8's established sectionless-ELF metric.

Both 800/3200-reference B timings were below 20x startup. Add one frozen
8000-reference input under the same AAAA/ABBA/ABBA protocol to obtain useful
long-workload evidence; retain all smaller observations and outliers. Other
compiler groups need no repeat. Finish the three originally planned native
workloads with the corrected size adapter. No compiler or flags changed.

The completed short executable measurements are retained in
`short-runtime-performance.json`: 21–34 ms is too short against ~5 ms process
startup. Without changing either compiler, add a second set using 16x volatile
trip counts (96M calls, 64M memory updates, 32M floating calls) and recomputed
checked results. Keep original executable artifacts and all observations;
use distinct `*-long` input/executable names. Carry the complete record into
`audited-performance.json`. This follow-up resolves timing granularity, not a
correctness or profitability failure. No executable speedup is claimed.

## Final compiler refresh

Implementation `63592ef0` adds the course's required RHS-before-computed-LHS
compound-assignment sequencing. The earlier A/B binaries and all observations
remain intact in `audited-performance.json` (formerly the in-progress final
record). Freeze the final compiler separately at `/tmp/pa10-final`, using the
same build flags. Run `benchmark.py final` with A unchanged, new B, and fresh
`/tmp/pa10-final-performance` artifacts. Use all eight original compiler inputs,
the already frozen 8000-reference input, and the three 16x runtime inputs; do
not repeat the superseded short runtime calibration. Keep the same AAAA plus
two ABBA sequence, affinity, equivalence checks, telemetry, and budgets. The
new complete record is `final-performance.json`. Record the native backend's
binary hash as well as its pinned bundle revision. Run no builds/tests during
timing. The new sequencing behavior is covered by an independent native test;
the performance inputs remain in the semantically equivalent common subset.
