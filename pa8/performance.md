# PA8 performance evidence

The first working PA8 implementation (`66167cf72`) is A; the final implementation
(`01d39a2f6`) is B. The stage-entry scaffold produces no LowIR and is not a valid
performance baseline. Both measured versions pass the 109 course cases, but A
loses signalling NaNs in an added fidelity probe. B adds local shape checks,
fixes literal preservation and counts IR pool growth. No optimizer or speedup
claim is made; this experiment bounds the cost of those correctness changes.

The [raw observations](../student.tests/pa8/performance.json) retain binary,
source and input hashes, host/CPU identity, GNU C++11/O3 compiler flags, backend
revision/flags, output hashes, wall/RSS measurements and phase/work telemetry.
Each workload has AAAA calibration and two ABBA blocks on one pinned CPU.
Every compiler output agrees within its family; all four native A/B executable
pairs are byte-identical. Reported latency deltas average the two paired block
ratios; paired spread and calibration spread are disclosed below.

Budgets fixed before measurement: compiler wall <=10% + A/A spread; RSS <=20%
+1 MiB; host text growth <=25%; 4x input <6x wall / <5x RSS +1 MiB; each main
sample >20x startup. Native runtime <=5% + A/A spread; native text growth 0%.
All gates pass. All 168 primary observations and 14 small exercise/adapter
observations remain, including the callback calibration outlier. None were
removed or superseded. Compiler startup median is 4.953 ms; native startup
median is 3.084 ms. The smallest main samples still exceed the 20x floor.

## Compiler latency and memory

| Workload | B median wall | B peak RSS median | Paired change (two blocks) | A/A spread |
| --- | ---: | ---: | ---: | ---: |
| calls-integers-1 | 236.48 ms | 41.08 MiB | +2.61% to +3.08% | 2.99% |
| calls-integers-4 | 940.55 ms | 154.02 MiB | +2.38% to +3.18% | 2.51% |
| memory-floating-1 | 296.72 ms | 40.61 MiB | +3.59% to +3.84% | 1.83% |
| memory-floating-4 | 1174.62 ms | 155.39 MiB | +3.27% to +3.50% | 1.26% |
| cfg-phi-1 | 109.22 ms | 17.37 MiB | +2.40% to +4.06% | 3.12% |
| cfg-phi-4 | 417.30 ms | 59.91 MiB | +2.39% to +2.64% | 4.08% |

The mean paired compiler regression is 2.52–3.72%; maximum median RSS increase
is 22 KiB. Host `.text` grows from 125,382 to 130,054 bytes
(+3.73%). These costs are disclosed, not presented as speedups. At 4x input,
wall scales 3.82–3.98x and RSS 3.45–3.83x. The largest case contains 696,000
instructions and 1,200,000 operands; the final validator visits each instruction
once. Its 14 IR pools grow only 160 times. Name interning and per-function
indexes are separate allocations, outside that pool-specific counter.

Separate stats-enabled observations preserve every output and report phase
and work counts. Measured B telemetry deltas range from -1.38% to +1.26%; these
small deltas include timing noise and are not evidence of negative overhead.
The allocation-growth counters themselves are always active in B and their
cost is included in the ordinary A/B comparison.

## Executable runtime and text

The fixed harness loads volatile loop state, varies helper arguments at runtime,
accumulates results, and checks the final value and exit status. Sum traverses
its complete allowed domain 20 times; the other workloads run 20 million
iterations. Swap includes identical-pointer calls, callbacks remain ordered,
and floating work includes volatile loads/stores and scalar conversions.
Generation, reference-backend compilation and execution are timed separately.
The backend is pinned to the manifest's source revision
`5ddcfe408e3d277825a6fc39a06f7f6bdeff7709` with `-O0`.

| Workload | B median runtime | Text bytes A = B | Paired change (two blocks) | A/A spread |
| --- | ---: | ---: | ---: | ---: |
| sum | 129.16 ms | 234 | -0.47% to +1.78% | 5.93% |
| swap | 183.77 ms | 250 | +0.47% to +0.48% | 0.80% |
| call | 201.83 ms | 270 | -0.13% to +0.26% | 47.39% |
| floating | 394.27 ms | 417 | -0.37% to +0.20% | 0.53% |

The supplied backend emits sectionless ELF. These harnesses contain no globals;
text size is the executable load payload from its entrypoint onward, excluding
ELF headers and including code padding. There is no static data in that span.
This is recorded explicitly instead of inventing a `.text` section.

The callback A/A calibration has a 47.39% outlier spread. Its paired deltas are
small, and identical executable hashes independently establish unchanged code;
no runtime improvement is inferred from timing differences in any family.
The experiment proves current-stage costs and equivalence, not the quality of
our future native backend. Template-heavy frontend and self-hosting costs remain
owned by their stages; the unchanged frontend's existing evidence is linked in
[PA7](../pa7/performance.md). Reproduction commands and artifact lifetimes are in
[the personal-test README](../student.tests/pa8/README.md).
