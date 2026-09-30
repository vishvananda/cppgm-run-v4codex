# PA26 performance evidence, implementation143

Entry code: `9063d5260c65bf6b1984ebae6028fef44251910a`.
Final implementation: `a624085e1f3823c590da657c2d17f596a301215e`.
Frozen binaries: `/tmp/pa26-143/entry-cppgm` and
`/tmp/pa26-143/accepted-cppgm`. Exact hashes, inputs, flags, phase counters,
images and every observation are in [evidence143](evidence143/).
Build flags remain `g++ -std=gnu++11 -Wall -O3`.

## Protocol

`performance143.py OUT ENTRY FINAL` performs an AAAA calibration and six ABBA
blocks for each compiler/runtime pair. Compilation uses `-O0 -c --stats`;
external `g++` linking is outside its timer. Every executable result is checked
before and during sampling. RSS comes from `/usr/bin/time`. All 224 observations
are retained, including outliers. The machine was not running our build/test
jobs concurrently. Timing uses wall time, not counter-derived estimates.

The common inputs each demand 2400 template bodies. Their runtime portions
exercise three million loop/call/memory or floating iterations, or 200,000
throw/catch/cleanup operations, driven by argc and checked against independent
results. The new header case executes 400,000 short/heap string constructions,
concatenations and cleanups with a checked character/length sum. Both labels
use the final binary for that case: the entry cannot compile `<string>`, so
there is no equivalent correct entry executable to compare.

`startup143.py OUT ENTRY FINAL` separately measures 28 batches of 64 short
translation units: four A/A batches and six ABBA blocks. It retains all 1792
individual latencies and the batch wall times. `startup_rss143.py OUT` adds
28 direct `/usr/bin/time` observations for compiler RSS. The Python child-tree
RSS observations are preserved but explicitly excluded from compiler RSS:
they include the measuring process's fork footprint. This avoids hiding fixed
host-environment setup costs behind a large template input.

## Common correct inputs

Paired B/A ratios are six-block medians (range). All three common objects and
executables are byte-identical between entry and final; runtime differences
therefore describe measurement noise, not generated-code changes.

| Workload | Compile median A/B ms | Compile B/A | Peak compiler RSS A/B KiB | Runtime median A/B ms | Runtime B/A | Executable text A=B bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 147.5/149.3 | 1.012 (0.984–1.151) | 28624/29092 | 51.29/51.18 | 0.996 (0.933–1.006) | 151633 |
| floating | 147.8/150.2 | 1.012 (0.998–1.302) | 28700/29080 | 47.59/47.73 | 1.003 (0.998–1.010) | 151474 |
| exceptions | 146.2/148.1 | 1.008 (0.979–1.053) | 28572/29036 | 252.10/251.75 | 1.000 (0.928–1.021) | 151778 |

Compiler A/A ranges were 146.8–169.1, 147.7–152.2 and 145.9–347.4 ms.
There is a small median compiler cost, about 1%, and under 2% peak RSS growth;
no speedup is claimed. Each common input still records exactly 2400 demanded
specializations and 2400 body transitions. Executable text includes 240 bytes
of host startup code; shared runtime-library text is not included.

The short-input compiler batches take 298.4/365.0 ms, paired ratio **1.223**
(1.201–1.275), versus a 297.9–312.2 ms A/A range. This is a repeatable fixed
cost, approximately **1.04 ms per translation unit**. Direct peak compiler RSS
is 6752/6932 KiB. The short-input objects/executables are also byte-identical.
Configured host predefines must now be tokenized and installed even when an
input contains no includes; a program can query them directly. Target header
search directories are normalized once per translation unit. There is no
process-global cache, external user-source preprocessing, speculative optimizer
or input-dependent bypass to conceal that required setup cost.

## New string baseline

Final/final compiler medians are 666.2/665.8 ms, peak RSS 46000/46092 KiB,
and paired ratio 1.006 (0.962–1.037). Runtime medians are 533.4/536.2 ms,
peak RSS 3564/3568 KiB, paired ratio 1.003 (0.833–1.224). Runtime outliers
reach 1.32 seconds and remain in the observations. Object text is 35723 bytes;
executable text is 35963 bytes. Both object hashes match. Full executable
hashes differ, so this baseline claims only checked behavior and measured
sizes, not byte identity of the linked images.

The source/header path records 127122 parsed nodes, 42414 template occurrences,
2050 specializations, 74 template body transitions, 201 class completions,
283 member-demand steps, 6104 LowIR instructions and 359 native functions.
These are measurements of demanded/source work, not universal complexity proof.
The production template-to-LowIR/MIR/ELF inspection is recorded separately.

## Acceptance and budgets

Correctness, unchanged coverage and PA26's EH inspection limits remain mandatory.
No optional optimization is added and no runtime-profit claim is made. The
new work closes required fixture semantics: bounded builtin descriptors,
canonical dependent queries, once-per-owner exception demand, and typed ABI
relocations. Header work scales with bytes/tokens and semantic demand; include
root normalization is O(d log d), integer-pack output is linear with an explicit
1048576-element resource ceiling, and GOT classification is one function pass.
Nonlocal zero-offset function addresses retain one seven-byte instruction;
private emission allocates no GOT-classification table. No inlining, cloning,
unrolling or new fixed-point optimization budget is introduced.

PA26 specifies no numeric compiler latency/RSS ceiling. The inherited 15%
and zero-growth diagnostic targets cannot become exit gates under spec section
9. In particular, the measured short-input increase is disclosed rather than
hidden by the large-input results. It pays for required target preprocessing
facts; no avoidable optional transform was retained. Historical observations in
[evidence142](evidence142/) and [performance142.md](performance142.md) remain
unchanged. Broader hosted compatibility and self-hosting belong to later stages.
The current checks are PA26 30/30, through PA26 4283/4283, and file audit pass;
whole-stage independent audit and the known output-format issue remain recorded
in the plan, without claiming assignment certification.
