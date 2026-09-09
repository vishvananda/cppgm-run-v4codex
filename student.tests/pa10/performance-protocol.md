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
