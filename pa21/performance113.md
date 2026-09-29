# PA21 audit 113 performance

These are **PA21/O0** measurements, not optimizer or student-native claims.
A is the clean audit entry `f3af4630`; B is implementation `30aec177`.
Frozen compiler SHA256:

- A: `3550213c5e30f9329593bec04df573e045b3ca4d74669e78f70c280eafdf114c`
- B: `8e06eea800b20675d8292fbc7346d58fac267693aa653c0569547db722e87c5d`

The [driver](../student.tests/pa21/benchmark113.py) pins one allowed CPU,
warms each binary, records four A/A observations and four ABBA blocks, and times
compiler invocations separately from checked executables. All flags, fixed
sources, hashes, raw samples, paired ratios, spreads, RSS and work counters are
retained. The [artifact archive](../student.tests/pa21/audit113-artifacts.json)
keeps the frozen executables and raw outputs. Runtime loops use volatile trip
counts and verify their results.
Startup and the template workload's tiny runtime are explicitly startup-sensitive;
the template **compilation** and loop/call/memory/floating/heap runtimes dominate
startup. The supplied object backend and host linker provide executables;
compiler measurement covers only the student source-to-LowIR command.

## Comparable correct fixed workloads

The table uses the [hosted campaign](../student.tests/pa21/performance113-hosted.json)
so size is actual ELF `.text`, including fixed CRT text and excluding shared
runtime text. Each A/B pair checks the same result. The five common workloads
produce byte-identical LowIR and ELF `.text` bytes (complete hosted ELF hashes
differ in other content). Heap A/B agrees on this fixed
nonthrowing execution; A is not used as a correctness oracle for failing
construction/destruction, where the new controls prove it wrong.

All times below are wall milliseconds; RSS is external peak KiB. Ranges in the
last two columns are the four paired B/A block ratios, not confidence intervals.

| Workload | Compiler A → B ms | Peak RSS A → B KiB | Runtime A → B ms | Native `.text` A → B bytes | Compiler B/A range | Runtime B/A range |
|---|---:|---:|---:|---:|---:|---:|
| startup | 7.70 → 7.55 | 5868 → 6044 | 4.92 → 4.94 | 248 → 248 | 0.975–1.017 | 0.982–1.003 |
| auto-specializations-9600 | 760.45 → 755.86 | 107712 → 108212 | 5.45 → 5.51 | 768286 → 768286 | 0.863–1.120 | 0.997–1.055 |
| runtime-calls | 6.81 → 6.51 | 6120 → 6176 | 124.74 → 126.56 | 430 → 430 | 0.854–0.971 | 0.822–1.027 |
| runtime-memory | 6.36 → 6.16 | 6144 → 6180 | 115.91 → 116.53 | 658 → 658 | 0.954–0.988 | 0.987–1.010 |
| runtime-floating | 8.51 → 8.33 | 6188 → 6360 | 87.15 → 87.64 | 454 → 454 | 0.977–1.004 | 0.999–1.007 |
| runtime-heap-ownership | 6.67 → 6.44 | 6172 → 6196 | 194.16 → 195.51 | 802 → 924 | 0.950–0.987 | 0.983–1.037 |

Compiler `.text` grows 2,224,646 → 2,240,710 bytes (+16,064, 0.72%).
The heap workload adds 122 bytes (+15.2% of this small executable's `.text`).
Its median runtime changes 194.16 → 195.51 ms (+0.7%), within the observed paired
variation. Neither this nor the common workload medians establish a general
speedup or a stable slowdown. Template compilation peaks at 108,212 KiB versus
107,712 KiB. Common executable instruction bytes and checked output are unchanged.

[Native inspection](../student.tests/pa21/audit113-native.json) shows why the heap
cost exists: the frame reservation grows 0x98 → 0xb8 bytes; the normal destructor
loop saves the allocation/data addresses and retains a decrement-before-call
remaining count. The O0 layout adds a continuation jump around exceptional
blocks. The failure path destroys the remaining prefix and releases storage.
These are bounded per delete-expression, independent of the runtime extent.
No destructor, allocation call, lifetime operation or ABI obligation is skipped
to obtain these timings. The declared-nonthrowing path retains its small form;
unknown destructor effects conservatively retain cleanup. This is required
ownership work, not an optional runtime optimization licensed by fewer IR nodes.

## Noise and preserved observations

The [first campaign](../student.tests/pa21/performance113-initial.json) is retained
in full. It had large wall-time disturbances: template A/B medians were 1.227 /
5.767 seconds despite similar CPU/phase work, and identical native bytes showed
large apparent runtime changes. No performance claim is based on those ratios.
The [repeat](../student.tests/pa21/performance113.json) keeps the same frozen
binaries/inputs and adds command-level elapsed time while the measurement session
is kept active. It still shows drift: heap A/A ranges 321–326 ms while later A
samples reach 185 ms. Its heap medians are 187.0 / 193.8 ms. The hosted campaign
above supplies exact text sizes and another complete calibration; it does not
erase either earlier observation set. Raw spreads and warmups remain available.
The freestanding records label their size honestly as a sectionless ELF payload
proxy, which includes data/EH tables; that proxy is not substituted for `.text`
in the final table.

## Required new-path cost and pipeline growth

The [growth driver](../student.tests/pa21/benchmark113_growth.py) and
[all observations](../student.tests/pa21/performance113-growth.json) measure the
final compiler twice (A/A), with checked native execution and actual `.text`.
Entry fails each source (`function has no blocks` or missing live ancestor), so
these are **final-only cost/growth** observations, never speedup comparisons.

| Input | Final compiler median ms (first lane) | Peak KiB (both lanes) | LowIR instructions | IR capacity bytes | Native `.text` bytes |
|---|---:|---:|---:|---:|---:|
| nested-helper-32 | 11.60 | 6756 | 704 | 153712 | 4079 |
| nested-helper-128 | 14.58 | 7996 | 2720 | 614512 | 15023 |
| contextual-new-512 | 108.41 | 23236 | 59954 | 10716576 | 307784 |
| contextual-new-2048 | 763.31 | 70768 | 239666 | 42861984 | 1229384 |

The grouped 512→2048 timing is about 7× and is preserved, not made an exit gate.
A separate [interleaved scaling diagnostic](../student.tests/pa21/performance113-scale.json)
keeps both source hashes, compiler and flags fixed: medians 117.32 / 410.25 ms;
block ratios 3.380, 3.665, 3.484, 1.096. The last small sample has 644 ms outer
wall time but 100 ms command elapsed time; it remains in the record. Thus timing
drift cannot establish a superlinear algorithm. The independently traced work
and output do establish the budget: four times as many handler functions give
59,954 → 239,666 instructions, 18,967 → 75,799 full-expression visits,
2,562 → 10,242 candidates and 16,943 → 67,631 lookup visits. No pass rescans all
functions per allocation. Nested helpers similarly emit each requested body
once; 32→128 nesting produces 704→2,720 instructions. The cap of eight for small
array/list expansion is unchanged; larger arrays use one counter loop and a
constant number of cleanup owners. Native text growth is proportional to emitted
functions/actions. Runtime profit is not inferred from these counts.

## Stage-scoped acceptance

The old +15% latency, +16 MiB RSS and 5.5× growth numbers are self-selected
**diagnostics**, not PA21/`spec.md` exit limits. That classification, already
recorded in 110–112, remains valid for inherited plans. This audit preserves
every old and new measurement, including misses. Current results satisfy the
actual PA21/O0 bounds: correct required LowIR, passing mandated checks/timeouts,
bounded local proof/ownership work and output, and measured executable costs.
No unprofitable optional transform was added. The invalid dynamic-typeid
hoisting proof was narrowed; stateless helper sharing and completed scalar-body
proofs retain their conservative fallbacks. Required ownership costs cannot be
compared against an incorrect entry path as an optimization opportunity.

The complete evidence chain remains [102](performance102.md),
[103](performance103.md), [104](performance104.md), [105](performance105.md),
[106](performance106.md), [107](performance107.md), [108](performance108.md),
[109](performance109.md), [110](performance110.md), [111](performance111.md) and
[112](performance112.md). Native selection/allocation/debug and self-hosting are
later-stage responsibilities; PA21 does not claim those implementations or run
inception as a new gate.
