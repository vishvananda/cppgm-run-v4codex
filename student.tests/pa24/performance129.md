# PA24 handoff129 performance evidence

PA24 O0 scope: required wide numerical execution and object/integer ABI transport. No optional optimization was added. The entry compiler cannot execute these wide workloads, so comparisons against entry cover the previously correct scalar/floating paths. Wide measurements use the same correct final binary for A and B; they establish a baseline and calibration, not a speedup.

Frozen artifacts: `/home/vishvananda/work/private/v4codex/artifacts/pa24-handoff129`. Earlier handoff evidence is unchanged. `scalar-performance/` retains the initial observations; `final-scalar/` remeasures compiler latency/RSS after payload representation changes. `final-runtime-identity.json` proves its measured integer/memory executables byte-identical to final output.

Protocol: GNU C++11 release build; `-O0 --stats -o OUTPUT INPUT`; CPU 0 affinity; four A/A samples and six ABBA blocks for each compiler/executable workload. Each manifest freezes binary/input hashes and flags and retains every wall time, peak RSS, telemetry record, paired ratio and spread. Compilation and execution are separate. Executables check their computed results with one and two extra arguments before timing; loop bounds depend on runtime argc. Compiler inputs have 4096 helpers with 64 typed operations each plus a checked entry.

| Binary | SHA-256 | Bytes |
|---|---|---:|
| entry-lowir2native | `71ea52397dd39c441edc2ac0db37a913b48da36bbfbc6b133ea6a22d6787cdb0` | 381752 |
| numeric-lowir2native | `c168f9ea17dfb45429840fc53fd53dd12c16d7bdb16778bf1d43ca0d7a4ac77a` | 428328 |
| final-lowir2native | `af21cb9fdad3a0c737c45b15fc2b823202b682b7d202b0fd608b658c793ed56b` | 428392 |

| Manifest | SHA-256 |
|---|---|
| scalar-performance/observations.json | `ea21976aa8cdfa7b290aa3d9d117e21be8f5f60f11515dd1bb07bd3241a8982c` |
| final-scalar/observations.json | `196c76bffb51f1df235baa5abbfb39a076d1daa2fbd6bfc4ff5222ed4b56a117` |
| final-floating/observations.json | `7fbc10745516e6431ce151d37f2d7b67fc4480bc145b07f51f50611d6bdacbcf` |
| final-wide/observations.json | `248725bc2ee7f233a6b5a59bef0f144ec59bcb145494db1376a7b7239182c98a` |

| Measurement | A median seconds | B median seconds | Paired B/A | A/B max RSS KiB |
|---|---:|---:|---:|---:|
| final-scalar: compile | 0.494 | 0.493 | 0.996 | 88108 / 87980 |
| final-floating: compile | 0.655 | 0.656 | 0.997 | 88908 / 88900 |
| scalar-performance: runtime | 0.579 | 0.580 | 1.005 | 256 / 256 |
| scalar-performance: memory-runtime | 0.611 | 0.609 | 0.990 | 256 / 256 |
| final-floating: floating-runtime | 0.538 | 0.539 | 1.000 | 256 / 256 |
| final-floating: pressure-runtime | 1.346 | 1.340 | 1.003 | 256 / 256 |
| final-wide: compile | 0.726 | 0.728 | 1.003 | 88152 / 88148 |
| final-wide: wide-runtime | 0.357 | 0.363 | 1.014 | 256 / 256 |
| final-wide: wide-numeric-runtime | 1.119 | 1.119 | 1.000 | 256 / 256 |

| Runtime workload | A text bytes | B text bytes |
|---|---:|---:|
| runtime | 161 | 161 |
| memory-runtime | 149 | 149 |
| floating-runtime | 1522 | 1522 |
| pressure-runtime | 557 | 557 |
| wide-runtime | 308 | 308 |
| wide-numeric-runtime | 2211 | 2211 |

Acceptance: final paired compiler medians are 0.996 (integer) and 0.997 (floating); peak RSS does not increase. All four existing runtime images are byte-identical, so noisy timing differences are not generated-code changes. The diagnostic budgets (at most 15% compiler latency/RSS growth, no optional text growth) are met. They remain diagnostic targets, not added exit gates; all mandated course bounds and correctness requirements remain binding. No optimization profit is claimed. The initial scalar compiler ratio 1.148 and its large spread are retained; the final frozen comparison supersedes it.

Necessary costs: wide values use two-word homes and ABI fragments. Restoring division has exactly 128 target iterations with constant emitted code. Float-to-wide conversion uses four base-2^32 digits; wide-to-float retains guard/sticky information before rounding. These operations emit bounded code per input instruction; no pass duplicates bodies or performs global retries. Atomic retry loops implement the atomic contract; they do not add compiler iteration. The new 308-byte call/phi workload and 2211-byte numeric workload provide checked runtime baselines. The latter exercises division and all f32/f64/f80 conversion formats using runtime inputs.

Compiler work remains O(N + E log E), including existing phi-edge ordering. ABI scheduling has at most fourteen register assignments and linear stack argument processing. Payload storage does not enlarge LowIR Operand. Function placement/MIR is released after encoding; existing telemetry records phase latency, peak memory, pool growth, instruction/text sizes and bounded work. Template/source-native and self-host benchmarks remain owned by later driver stages; earlier compiler behavior is checked by the 3856-case through report.

All paired ratios and spread (seconds):

- final-scalar/compile: paired 0.986, 0.994, 1.007, 0.937, 0.998, 1.023; A 0.484–0.557; B 0.485–0.510; A/A 0.485–0.520.
- final-floating/compile: paired 0.996, 0.985, 0.998, 1.014, 1.005, 0.996; A 0.649–0.674; B 0.653–0.662; A/A 0.661–0.695.
- scalar-performance/runtime: paired 0.997, 1.003, 1.007, 1.097, 0.989, 1.124; A 0.430–0.587; B 0.427–0.598; A/A 0.563–0.585.
- scalar-performance/memory-runtime: paired 0.879, 0.987, 0.994, 0.993, 1.011, 0.751; A 0.318–0.627; B 0.316–0.618; A/A 0.524–0.606.
- final-floating/floating-runtime: paired 0.999, 1.000, 1.001, 0.998, 1.001, 1.004; A 0.537–0.540; B 0.537–0.543; A/A 0.537–0.540.
- final-floating/pressure-runtime: paired 0.909, 0.921, 1.024, 1.009, 1.007, 1.000; A 1.323–1.744; B 1.324–1.511; A/A 1.327–1.352.
- final-wide/compile: paired 1.002, 1.003, 1.072, 0.987, 1.312, 1.001; A 0.719–0.749; B 0.722–1.052; A/A 0.722–0.733.
- final-wide/wide-runtime: paired 1.000, 1.017, 1.016, 1.013, 1.036, 1.003; A 0.352–0.366; B 0.350–0.377; A/A 0.357–0.368.
- final-wide/wide-numeric-runtime: paired 0.999, 1.002, 1.001, 0.984, 0.999, 1.004; A 1.114–1.155; B 1.114–1.126; A/A 1.117–1.123.
