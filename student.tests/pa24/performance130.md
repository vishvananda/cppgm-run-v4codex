# PA24 checkpoint audit130 performance

Reviewed code tip: `9f3f9b5caaec9677af0e8431b51cacebcf823dce`. PA24/O0 acceptance covers the implemented native
selection paths and shared LowIR boundary; it does not certify the unfinished
runtime/layout and canonical MIR groups in [the plan](../../pa24/plan.md).

Frozen evidence: `/home/vishvananda/work/private/v4codex/artifacts/pa24-audit130`. The entry compiler A has SHA-256
`af21cb9fdad3a0c737c45b15fc2b823202b682b7d202b0fd608b658c793ed56b`; reviewed compiler B has SHA-256
`a87d22b8e2a3997eee5130001d2f23c8c228ec292edb2b75012011f3272b3a13`. GNU C++11 release build (`-O3`),
fixed `-O0 --stats -o OUTPUT INPUT` flags, CPU 0 affinity. Each compiler workload
has 4096 functions with 64 integer, floating or wide operations and a checked
entry. Each phase has four A/A observations followed by six ABBA blocks.
Compilation and execution are measured separately; all observations are retained.
Runtime loop bounds use argc and each executable checks its result with one and
two extra arguments before timing. These workloads are valid on both binaries;
broken reducers are never used as faster baselines.

| Workload | A wall median (s) | B wall median (s) | Paired B/A | A/B max RSS (KiB) |
|---|---:|---:|---:|---:|
| scalar/compile | 0.974 | 0.947 | 0.952 | 88104 / 88044 |
| scalar/runtime | 0.721 | 0.709 | 0.998 | 256 / 256 |
| scalar/memory-runtime | 0.548 | 0.522 | 0.947 | 256 / 256 |
| floating/compile | 1.279 | 1.256 | 0.964 | 88932 / 88932 |
| floating/floating-runtime | 0.916 | 0.912 | 0.970 | 256 / 256 |
| floating/pressure-runtime | 2.416 | 2.346 | 0.991 | 256 / 256 |
| wide/compile | 0.754 | 0.726 | 0.982 | 88348 / 88264 |
| wide/wide-runtime | 0.353 | 0.354 | 1.008 | 256 / 256 |
| wide/wide-numeric-runtime | 1.120 | 1.122 | 1.001 | 256 / 256 |

All six runtime images are byte-identical, with code sizes 161 (integer/call),
149 (memory/call), 1522 (floating), 557 (pressure), 308 (wide call/phi), and
2211 (wide numeric) bytes. `runtime-identity.json` records their hashes.
This directly preserves the historical forward-edge and XMM reuse improvements.
Timing variation on identical programs is not a code-speed change. No new speedup
is claimed. Compiler paired medians are below 1.0 with unchanged or lower peak
RSS; overlapping spread does not establish a compiler speedup either.

The inherited <=15% compiler latency/RSS and zero optional text-growth targets
remain diagnostics, not additional stage exit gates. These measurements meet
them; the spec's stage-scoped rule also prevents old noisy misses from becoming
permanent failures. All course behavior, comparisons, controls and MIR envelopes
remain binding. No required limit, test, reference or comparator changed.
Nine inherited observation manifests were rehashed against performance127/128/129
records (`inherited-performance-verified.json`); all matched and remain preserved.

The fixes add no optimization or iteration: fixed-effect reuse checks use an
existing O(1) fact; operand preparation and address stabilization do bounded work;
unknown byte-multiply effects retain valid frame traffic. Existing pipeline
limits remain nine GPR/fourteen XMM candidates, at most fourteen ABI carriers,
three 64-instruction carry probes, six-bit monotonic parameter flow and
O(E log E) phi-edge ordering. Wide division emits constant code with 128 target
iterations; FP-to-wide uses four digits. These required semantic costs are not
optional transform profit. Source/native driver integration and self-hosting
remain later-stage work; the source-template trace checks today's explicit tool
boundaries and the earlier-stage report checks inherited frontend correctness.

Paired observations and spread:

- scalar/compile: paired 0.961, 0.909, 0.906, 1.091, 0.994, 0.943; A 0.917–1.218 s, B 0.896–1.085 s; A/A 0.911–1.007 s.
- scalar/runtime: paired 0.914, 1.021, 1.033, 0.974, 1.047, 0.972; A 0.684–0.807 s, B 0.681–0.801 s; A/A 0.721–0.757 s.
- scalar/memory-runtime: paired 0.936, 1.001, 0.955, 1.043, 0.939, 0.812; A 0.481–0.654 s, B 0.473–0.604 s; A/A 0.512–0.548 s.
- floating/compile: paired 1.024, 1.003, 0.922, 0.928, 0.967, 0.960; A 1.142–1.716 s, B 1.103–1.585 s; A/A 1.397–1.492 s.
- floating/floating-runtime: paired 0.972, 1.071, 0.906, 0.969, 0.968, 1.020; A 0.841–1.040 s, B 0.802–1.014 s; A/A 0.836–0.909 s.
- floating/pressure-runtime: paired 1.022, 0.985, 0.997, 0.971, 0.783, 1.007; A 1.367–2.628 s, B 1.360–2.662 s; A/A 2.236–2.814 s.
- wide/compile: paired 0.768, 0.975, 1.125, 0.990, 0.756, 1.007; A 0.719–1.179 s, B 0.717–0.948 s; A/A 0.752–1.029 s.
- wide/wide-runtime: paired 1.009, 1.014, 1.008, 1.010, 0.986, 0.949; A 0.342–0.371 s, B 0.347–0.360 s; A/A 0.360–0.368 s.
- wide/wide-numeric-runtime: paired 1.002, 0.999, 1.004, 1.001, 1.000, 1.003; A 1.116–1.125 s, B 1.118–1.124 s; A/A 1.118–1.128 s.

Frozen manifests (all sample data, input hashes, flags and telemetry):

- `/home/vishvananda/work/private/v4codex/artifacts/pa24-audit130/perf-scalar/observations.json`: `3d8a0abcb36a8dd86bfc007efee490e2006dea952487d0fbb0855f6a8d37dc6c`.
- `/home/vishvananda/work/private/v4codex/artifacts/pa24-audit130/perf-floating/observations.json`: `b7fc4ef61a46c1c2d6ca202eb283a417bc4ef459aa89c5f8587d3184236302de`.
- `/home/vishvananda/work/private/v4codex/artifacts/pa24-audit130/perf-wide/observations.json`: `be321fe4474fdafe0a29d0fa51201545dcfc59f31d761ce6707ddff594e7d847`.
