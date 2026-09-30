# PA28 handoff152 performance

Final code: `5f8483c7`. Frozen entry compiler SHA-256:
`d10d9691602a574ab7c5183c2b6ac71ab657bf693d57d9e2a30a873ff0254686`.
Final compiler: `67e6bdd42e6185b8f483f2ca3796a35aafa18155ddc28a739431ba2cc2bdf7f2`.

## Protocol and scope

[Common data](evidence152/common-performance.json) contain four fixed PA26/27
workloads: 2,400 demanded function templates plus memory/calls, floating point,
200,000 checked throws, and 1,200 unused declarations. argc controls 3,000,000
loop iterations or the throw count. Compilation and execution are separate;
host linking is outside timing. Each mode uses four A/A observations and six
ABBA blocks, CPU 0 affinity, -O0 -c --stats. No compiler build or correctness
suite ran concurrently. All outputs are checked before and during timing.

[Filter input](evidence152/filter-input.cpp) demands 400 exception-specification
templates and performs 120,000 runtime-controlled throws with ordered Guard
cleanup. Half the relevant throws require unexpected conversion; numeric
results and destruction/conversion counts are checked. The entry executable
returns 9 because it does not enforce the filter. Twelve final compile and
twelve checked runtime observations measure newly supported costs; there is no
A/B speed comparison against this incorrect baseline. [Raw data](evidence152/filter-performance.json).

[Ownership input](evidence152/ownership-input.cpp) demands 400 local static
template instances with host-owned virtual tables and performs 3,000,000
runtime-controlled virtual-base projections. Host RTTI cross-casts, arithmetic
checksum and destructor count are checked. The entry compiler rejects the input
with an unavailable semantic prerequisite. Twelve final compile and twelve
checked runtime observations measure standalone support costs, with the same
affinity and separate host linking. [Raw data](evidence152/ownership-performance.json)
include the source/header hashes and host flags/version;
[host input](evidence152/ownership-host-input.cpp) uses the checked-in personal headers.

## All four dimensions

Seconds are medians of 12 measured A/B samples; RSS is maximum KiB.

| Input | Compile s A/B | Compile KiB A/B | Runtime s A/B | Text bytes A/B |
|---|---:|---:|---:|---:|
| memory | 0.1991 / 0.2044 | 29316 / 29248 | 0.0691 / 0.0728 | 151633 / 151633 |
| floating | 0.2273 / 0.2024 | 29420 / 29340 | 0.0570 / 0.0570 | 151474 / 151474 |
| exceptions | 0.2495 / 0.2541 | 29252 / 29220 | 0.3477 / 0.4409 | 151781 / 151781 |
| pruning | 0.2095 / 0.2059 | 35252 / 34860 | 0.0557 / 0.0538 | 151633 / 151633 |

| Input | Paired compile ratio (range) | Paired runtime ratio (range) | Compile A/A s | Runtime A/A s |
|---|---:|---:|---:|---:|
| memory | 1.013 (0.887–1.120) | 1.030 (0.985–1.092) | 0.1858–0.2517 | 0.0549–0.0649 |
| floating | 0.996 (0.671–1.052) | 1.000 (0.979–1.042) | 0.2624–0.2720 | 0.0515–0.0527 |
| exceptions | 1.028 (0.992–1.413) | 1.002 (0.990–1.350) | 0.1543–0.1574 | 0.4409–0.4451 |
| pruning | 1.011 (0.893–1.061) | 0.992 (0.863–1.045) | 0.1979–0.2221 | 0.0525–0.0650 |

New filter workload: compilation **0.0917 s** (0.0906–0.1496),
**20,520 KiB** peak; runtime **0.5672 s**
(0.5611–0.6028), **3,912 KiB** peak;
**242,918 executable text bytes**, 581,216 object bytes.

New ownership workload: compilation **0.0618 s** (0.0439–0.0742),
**11,796 KiB** peak; runtime **0.0559 s** (0.0548–0.0588),
**3,608 KiB** peak; **57,298 executable text bytes**, 445,792 object bytes.

There is considerable scheduling noise and temporal drift. The exception
runtime standalone medians differ by about 27%, while its paired ratio median
is 1.002 and range .990–1.350. Memory runtime has a measured paired increase
of 3.0%. These results are disclosed, not filtered or averaged away. No precise
timing speedup or uniform slowdown is claimed.

## Work, budgets and acceptance

[Byte comparisons](evidence152/comparison.json) show every common object and
entire executable is identical between A and B: instructions, frames, spills,
calls and text size are unchanged. Common non-time/non-RSS telemetry counters
also agree; Entity remains 120 bytes. Compiler RSS differs by -32 to -392 KiB,
which is not a precise allocation measurement. There is no common code growth.

Allowed exception sets cost O(k log k) for k actual types; published slices
and native filters consume linear storage. The existing boundary rewrite is
one O(n) pass per affected function; region/action queues remain deduplicated.
Only demanded table definitions establish construction-table prerequisites.
All transient native/filter storage dies per function. New text/LSDA growth
implements mandatory semantics, not an optional optimization. No optional
transform or optimizer work/growth budget was added or relaxed.

PA28 O0 acceptance is governed by spec §9. Inherited blanket 15% latency/RSS
and zero-growth diagnostics are not mandated gates. All existing frontend
depth/evaluation, backend/frame/ELF bounds, correctness and coverage remain.
PA32/33 optimization and PA34 self-hosting remain later-stage obligations.
This evidence accepts the completed EH/ownership groups, not unfinished
covariant virtual-primary layout.

## Preservation and reproduction

All **520 observations** are preserved: 224 common + 24 filter observations
before the adapter/inspection refinements, the same counts for final code,
and 24 final ownership observations.
See [intermediate common](evidence152/intermediate-common-performance.json) and
[intermediate filters](evidence152/intermediate-filter-performance.json). The
rerun followed code changes; no sample was discarded. Earlier performance151
evidence remains unchanged.

```sh
PERF_CPU=0 python3 student.tests/pa27/performance147_common.py OUT_COMMON A B
PERF_CPU=0 python3 student.tests/pa28/performance152.py OUT_FILTERS A B
PERF_CPU=0 python3 student.tests/pa28/performance152_ownership.py OUT_OWNERSHIP A B
```
