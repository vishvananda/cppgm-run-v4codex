# PA20 handoff 94: performance and architecture evidence

The completed deduction, operand construction and lvalue conversion owners add
required PA20/O0 behavior. This is an incomplete stage handoff, not a whole-stage
performance or architecture certification. No generated-code speedup is claimed.

## Frozen comparisons

[Harness](../student.tests/pa20/benchmark94.py) records sources, input hashes,
binary hashes, flags, backend hash, raw observations, checked exits and telemetry.
The compiler build uses `g++ -std=gnu++11 -Wall -O3` and `TEST_RUNNER_ENABLE`;
compilation is `--emit-lowir -O0`. Validation/telemetry run separately with
`--stats --validate-lowir`; timed compilation has neither flag. The course's
supplied backend runs with `-O0` only in the test harness. No implementation
output is delegated to another compiler.

| Binary | Commit | SHA256 | Compiler .text bytes |
|---|---|---|---:|
| Entry | `a9b24ab6` | `1a181c95ee97d646d41c1009780200049e776d67e1d715840d5526b3d10e1d40` | 2,005,958 |
| Before scratch-table removal | `df239d8e` | `d83fca6dbedd42070cb29d352b416684397d473c97326df129197e304268c9bb` | 2,012,230 |
| Final | `0dd795a1` | `29cfe4f4ced25eabdc3d652a91b275c74455f855a090029d3497207ed88bf5bc` | 2,012,294 |

Final compiler growth is 6,336 bytes (0.316%). The first candidate was frozen
again as `/tmp/pa20-before-fastpath-cppgm` before replacing the final snapshot.
Backend SHA256 is `c3bae4acf3243d5a2fd6a15ef00e2d75d11a7d82715b1e4771f55165d0542490`.
Platform: Linux x86-64, kernel 7.0.0-1005-gcp, glibc 2.43.

Each comparable compiler/runtime workload has one warmup per binary, four A/A
observations and four ABBA blocks. New behavior has one warmup and six B samples.
Each full run retains 332 observations plus 34 warmups. No concurrent build/test
was launched during measurement. All runs are retained:

- [Initial CPU 31 run](../student.tests/pa20/performance94-initial.json), entry vs `df239d8e`.
- [Noisy CPU 31 run](../student.tests/pa20/performance94-noisy.json), entry vs final.
- [Final CPU 1 run](../student.tests/pa20/performance94.json), entry vs final.
- [Direct CPU 1 comparison](../student.tests/pa20/fastpath94.json) and
  [repeat](../student.tests/pa20/fastpath94-repeat.json), `df239d8e` vs final;
  each has 40 observations plus four warmups.

The eight common inputs are correct under both binaries, with checked results
and **byte-identical native files**. The auto workload deliberately returns int,
where entry's incomplete implementation already produces the correct result;
non-int/reference deduction is covered by correctness controls. The new pack
array workload was rejected by entry and is reported without an A/B claim.

## Compiler cost

Final CPU 1 run below. Milliseconds are medians; RSS is the maximum observed
KiB in measured ABBA blocks. The four ratios are B/A per block; A/A is the
minimum–maximum calibration interval in ms. All individual timings, user/system
time and context switches remain in the JSON.

| Workload | A / B ms | A / B peak KiB | Paired B/A | A/A ms |
|---|---:|---:|---|---:|
| startup | 16.96 / 17.08 | 5,720 / 5,932 | 0.976, 1.078, 0.979, 0.968 | 16.47–17.53 |
| auto-specializations-2400 | 388.92 / 396.16 | 30,764 / 31,608 | 0.968, 1.214, 1.250, 0.619 | 390.12–841.81 |
| namespace-variable-2400 | 136.22 / 133.83 | 24,624 / 24,748 | 0.991, 0.901, 0.977, 0.987 | 133.51–142.25 |
| auto-specializations-9600 | 632.01 / 664.16 | 107,016 / 106,012 | 0.926, 1.060, 0.929, 1.058 | 617.37–627.24 |
| namespace-variable-9600 | 536.35 / 531.20 | 81,648 / 81,680 | 0.999, 0.990, 1.082, 0.986 | 532.55–540.16 |
| runtime-calls | 6.36 / 6.13 | 5,892 / 6,092 | 0.997, 0.966, 0.967, 0.958 | 6.48–6.63 |
| runtime-memory | 6.55 / 6.27 | 5,984 / 6,112 | 0.980, 0.959, 0.958, 0.971 | 6.47–6.76 |
| runtime-floating | 6.54 / 6.43 | 6,152 / 6,180 | 1.004, 0.975, 0.960, 0.956 | 6.15–6.26 |
| new-array-pack | — / 6.87 | — / 6,204 | — | — |

The 9,600-specialization final median is 664.16 ms vs 632.01 ms (+5.1%),
with paired ratios spanning 0.926–1.060 because two A observations stalled.
The initial run was 759.04 vs 639.03 ms; the noisy final CPU 31 run was
1,460.83 vs 1,269.24 ms. Changing CPU did not cure all noise: the final
2,400-case calibration spans 390–842 ms. These are disclosed semantic costs
and measurement limits, not evidence of a general speedup or quadratic work.

`0dd795a1` removes two per-deduction scratch hash tables for a bare by-value
placeholder: its binding is already the adjusted canonical operand type. This
repairs avoidable allocation work in the required deduction path and adds 64
compiler text bytes; structured declarators still use general deduction.
The direct comparison was noisy (2,603/2,701 ms, paired
0.953/0.930/1.345/0.977). Its repeat measured 682.24/668.27 ms, paired
0.992/0.579/0.645/0.976, A/A 656–871 ms, peak RSS 106,452/108,228 KiB.
All four repeat blocks favored the direct path, but the large ratios were caused
by stalled A samples; **no stable percentage latency benefit is asserted**.
The retained change removes unnecessary hot allocations rather than adding an
optional optimizer or new analysis. Both direct-comparison native files match.

For n=2,400/9,600, both binaries parse 62,462/249,662 nodes and check n+1
bodies. Final records exactly n return deductions and n deduction demands,
only two placeholder-type computations, 2n+2 type-substitution work units,
and n substitution frames. Entry used eight canonical types, final ten.
Deduction does not reparse or recheck a body for the second call. Type/substitution
facts are scoped to the compilation and released with the analyzer; there is
no persistent cache. Linear counters, unlike noisy wall time, support the
bounded work claim. Namespace-variable workloads are retained from PA19 to
expose collateral frontend costs.

## Executable cost

All executions check their result. Calls use a volatile 24-million-iteration
loop, memory 16 million, floating point eight million; the new pack array
uses 12 million and checks a masked accumulated sum. These dominate process
startup and cannot be eliminated. Other rows are startup-dominated controls,
not runtime-profit evidence. Native payload size is the supplied sectionless
ELF payload proxy **including static data**, not an exact .text section.

| Workload | A / B ms | Paired B/A | A/A ms | A / B payload bytes |
|---|---:|---|---:|---:|
| startup | 8.219 / 8.584 | 1.086, 0.944, 1.052, 0.999 | 7.809–8.935 | 24 / 24 |
| auto-specializations-2400 | 3.638 / 3.591 | 0.985, 0.978, 1.030, 1.008 | 3.355–3.509 | 192,062 / 192,062 |
| namespace-variable-2400 | 3.362 / 3.456 | 1.085, 1.032, 1.031, 0.983 | 3.308–3.441 | 24 / 24 |
| auto-specializations-9600 | 3.496 / 3.521 | 0.948, 0.993, 1.003, 0.996 | 3.477–3.815 | 768,062 / 768,062 |
| namespace-variable-9600 | 3.131 / 3.121 | 0.989, 0.993, 1.014, 0.991 | 3.175–3.236 | 24 / 24 |
| runtime-calls | 122.457 / 122.367 | 1.003, 0.999, 0.993, 0.985 | 122.368–123.162 | 206 / 206 |
| runtime-memory | 73.124 / 73.027 | 0.994, 0.993, 0.996, 0.999 | 72.630–73.803 | 434 / 434 |
| runtime-floating | 86.006 / 85.720 | 0.998, 1.000, 0.999, 0.994 | 85.693–86.201 | 230 / 230 |
| new-array-pack | — / 76.973 | — | — | — / 263 |

Identical common executables mean the observed native timing differences do
not result from changed code. New-array-pack ranges 75.84–78.15 ms; its compile
time ranges 6.63–7.11 ms. Its 263-byte payload is new required behavior, with
no valid entry implementation to use as a performance comparator.

## Work budgets and stage acceptance

Required limits are the spec's O(n)/O(n log n) work bounds, canonical complete
keys and stage output contracts. PA20 sets no wall-time/RSS percentage gate.
The explicit owner budgets used here are one body computation per complete
function/specialization key; one cached placeholder predicate per canonical
type; deduction proportional to checked returns and declarator shape; list
completion proportional to consumed clauses/fields; scalar helpers at most
fields+1 prefix variants per target, with the complete transfer plan in transfer
helper keys. The inherited array expansion cap of eight remains unchanged.
No optional generated-code transform, expansion or profitability policy is added.

Necessary return deduction and array/lifetime facts are current-stage costs,
not grounds for weakening correctness or inventing an extra exit gate. The
inherited PA18/19 plans already classify old microbenchmark percentage targets
as diagnostics; preserve their measurements and that stage-scoped treatment.
Native encoding/debug and optimized policies belong to PA24/32–34; self-hosting
cannot run at PA20. Those later owners remain required at their stages, not
additional PA20 gates. No mandated performance bound or coverage was waived.

## Source-to-output trace and correctness

[Trace source](../student.tests/pa20/trace94.cpp),
[reproducer](../student.tests/pa20/trace94.py) and
[full evidence](../student.tests/pa20/trace94.json) connect a nontrivial declaration
and demanded templates to native execution:

1. `make_pair` records a placeholder result and deduces canonical `Pair` from
   typed functional aggregate initialization; its helper receives only the
   explicit first member and initializes the omitted second member to zero.
2. `alias<long>` substitutes its parameter through the existing specialization
   facts; return deduction collapses `auto&&` to `long&`. LowIR passes and returns
   a pointer, and the native check verifies object identity.
3. `Counter<long>::operator long&` is selected once for postfix increment;
   LowIR has one conversion call, one i64 load/add/store, and returns the old
   value. Condition `auto pointer` uses the same canonical object deduction.
4. `sum<int,int>` expands the two pack lanes into a typed three-element array
   plan; LowIR has an `obj<12x4>` temporary and the two source additions. Its
   deduced result is i32, without synthesized syntax or textual roundtrips.

The trace has five body checks, three return deductions, three demanded source
regions and 111 LowIR instructions. Stats-on/validated and stats-off output are
byte-identical. Supplied native execution returns zero. The selected facts and
helper identities flow directly into LowIR; the backend is only the owning
handout's validation boundary. This inspection is evidence for the completed
owners, not an independent audit of every stage requirement.

[Validation ledger](../student.tests/pa20/validation94.json) records `make test-pa20`
91/144 (53 existing failures), PA1–19 3452/3452, through-PA20 3543/3596, and
file audit exit zero with three inherited header warnings. Explicit personal
controls pass 83/83 (59 native, 24 required rejections), plus the trace.
All 144 original tests, references and exit statuses, and 637 tracked contract/
harness files match stage base. Eighteen original failures are fixed; none are
new. No reference correction or comparison change was used. Remaining behavior
and the concrete implementation boundary are in [the plan](plan.md).
