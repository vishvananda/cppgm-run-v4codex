# PA25 implementation136 performance evidence

O0 scope: source wide-integer semantics, constant/ABI payloads and floating builtins. No optional optimization was added; no speedup is claimed. Compiler work/growth budgets remain linear source traversal and constant work per fixed-width scalar operation, plus average O(1) complete-key interning. Each distinct 128-bit payload has one TU-owned pool entry; common Constant and ABI Node sizes are unchanged. Native wide-operation bounds are inherited from PA24. Optional transform work and optional generated-code growth are both zero.

Frozen A is entry `cf26b48b`; B is production `ba608c4f`. SHA256:

- A: `b92e151177f2dc85c19b6d2a3306fd825aee066ccaf68677f64e94912af78609`
- B: `a30591cca86ea42f2a65ba3cc00e2f5714ff11055913739f4e30c9e0bb4ee417`

[performance.py](performance.py) preserves binary/input hashes, flags and all wall/RSS samples in the artifact directory. All runs used `-O0`, `/usr/bin/time`, one A/A calibration block followed by six ABBA blocks, with compiler and executable timing separate. No build/test work ran concurrently with timing. Nothing was discarded. The equivalent template, memory and floating executable pairs are byte-identical and return checked results. Inputs cover 4800 demanded template specializations across nine TUs, and runtime-driven loops, calls, loads/stores and floating arithmetic. PA34 owns self-hosting; inherited fixed benchmarks remain available.

| Workload | B compiler batch s | Peak KiB A / B | Paired B/A median [range] | A/A batch range s |
|---|---:|---:|---|---|
| templates | 0.23659 | 15604 / 15680 | 1.010 [0.993, 1.028] | 0.22908–0.24823 |
| memory | 0.41334 | 6688 / 6760 | 0.955 [0.616, 1.008] | 0.38534–0.39166 |
| floating | 0.35672 | 6784 / 6848 | 0.930 [0.899, 0.984] | 0.35532–0.37939 |

Template batches contain one compile/link; memory and floating batches contain 64. Paired ratios use within-block means; batch medians need not have the same ratio. The memory compiler ratio has a large low outlier (0.616), which is retained and prevents attributing timing improvements to this change. Compiler RSS growth is under 1.1% on the equivalent workloads.

| Workload | B runtime batch s (3 runs) | Paired B/A median [range] | Text bytes A / B | A/A runtime range s |
|---|---:|---|---:|---|
| templates | 0.11762 | 1.003 [1.001, 1.019] | 384567 / 384567 | 0.11689–0.11875 |
| memory | 0.14987 | 1.002 [0.992, 1.043] | 521 / 521 | 0.14998–0.15299 |
| floating | 0.13883 | 1.001 [0.991, 1.040] | 340 / 340 | 0.13823–0.13996 |

The entry compiler rejects the new wide source surface, so it cannot be a correct equivalent baseline. A separate final/final calibration executes 200000 runtime-dependent 128-bit arithmetic/modulo iterations through a call, checked against Python arbitrary-precision arithmetic. The 64-compile batch median is 0.40750 s, peak compiler RSS 6764 KiB, runtime batch median 0.29249 s (three runs), and text size 977 bytes. Compiler paired final/final median 1.014, range [0.910, 1.051]; runtime 1.001, range [0.996, 1.007]. A/A ranges are 0.41383–0.44454 s compilation and 0.29177–0.29276 s execution. This establishes a baseline, not a before/after benefit.

Executable RSS observations are 256 KiB for all workloads. All timings include startup and measurement-tool overhead; compiler batches make that limitation explicit. The empty-PATH source-to-ELF trace is separate from timing: 20 unique wide constants, 1024 pool capacity bytes, two specializations (one primary body transition and one explicit specialization selection), five native functions and 2538 text bytes.

Stage-scoped acceptance: these are necessary semantics, with no optional optimization cost to justify. The equivalent generated code does not grow or change; timing shows no repeatable avoidable regression. Inherited 15% latency/RSS and zero optional text-growth targets remain diagnostics, not additional gates. No mandated correctness, coverage or finite-work requirement is relaxed. The historical measurements and all present samples remain intact.

Artifacts: `/home/vishvananda/work/private/v4codex/artifacts/pa25-136/`.

| Evidence | SHA256 |
|---|---|
| `performance/performance.json` | `7eb6dab5d43d84e242be70b32860b8951b4fc41ff25d52d97f48fe5b9bc64292` |
| `wide-performance/performance.json` | `6a39e340677bc2a7d04792848dbc52e76c8d6e6896b7a82a6388e44fae6146b3` |
| `wide-trace.json` | `8632ddb61309ff63afd63afc0287fa0230ef8062f8ffb3d8d61c47d8e871ef13` |
| `validated-checks.json` | `a7cdcdb890b1521a15556bbed3fb2ad72cd96b455f69e1429aa47b9d969654d6` |
