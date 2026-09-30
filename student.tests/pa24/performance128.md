# PA24 handoff128 performance evidence

PA24 O0 scope: typed LowIR native execution and bounded selection. The entry
compiler cannot execute floating input, so it is used only for the existing
integer/memory comparison. The first correct floating implementation is the
baseline for XMM selection and pressure handling. Source/template-native and
self-hosting comparisons remain owned by later driver stages.

Frozen artifacts: `/home/vishvananda/work/private/v4codex/artifacts/pa24-handoff128`. No earlier measurements were removed:
`regression/`, `selection/`, `final-regression/`, `final-selection/`,
`accepted-integer/` and `accepted-floating/` retain every observation.

| Compiler | SHA-256 |
|---|---|
| A | `9e7181256b23c885e2b4c07f2be196285ed97a6797c3f424258217f238bdb3c3` |
| float-baseline | `9b84ce1d2aa78d3d7166592a64106ea0d9af231032f5432239c0c6ccae21c8ac` |
| B-final | `7baf86e32c374774ed9cc2230cd97ca97a5f009c200ae601b474f25f6091443c` |
| B-accepted | `71ea52397dd39c441edc2ac0db37a913b48da36bbfbc6b133ea6a22d6787cdb0` |

`B-accepted` is the committed implementation including visit counters. The last
counter-only change leaves all four executable images byte-identical to measured
`B-final` output. `accepted-runtime-identity.json` records hashes, counters and
checks with both argument counts. Compiler latency/RSS was remeasured with the
accepted binary; runtime measurements refer to the identical frozen executables.

Protocol: GNU C++11 release build; `-O0 --stats -o OUTPUT INPUT`; CPU 0 affinity;
four A/A samples followed by six ABBA blocks per workload, separate compilation
and execution. Inputs and flags are frozen in each manifest. Compiler inputs have
4096 helpers with 64 operations each plus a checked entry. Runtime iterations
depend on process argc; all results are checked with one and two extra arguments.
Floating sums and pressure recurrences remain live and input dependent.

| Observation manifest | SHA-256 |
|---|---|
| accepted-integer/observations.json | `7656d265632142bc1e515539010a85072cc702f1310c92c6288dad41af50d145` |
| accepted-floating/observations.json | `dd24c28f4cac84c0b381db866a2f5971c95985b74a1cb2bc3bfbf53abb7bf698` |
| final-regression/observations.json | `cbfce3b9b033b834b6042e5df4b8f62774c1a20d9e6f9788350c5bd75c737145` |
| final-selection/observations.json | `3c1d69c0a9dad83fbb279084060d38f2d91376bdc814e5d6e4142814c8fb3ae1` |

| Measurement | A | B | Median paired B/A |
|---|---:|---:|---:|
| Integer compiler wall (s) | 1.485 | 1.421 | 0.971 |
| Floating compiler wall (s) | 1.674 | 1.369 | 0.917 |
| Integer/call runtime (s) | 0.594 | 0.563 | 0.955 |
| Memory/call runtime (s) | 0.634 | 0.626 | 0.990 |
| Floating runtime (s) | 1.193 | 0.546 | 0.458 |
| Pressure runtime (s) | 1.969 | 1.938 | 0.935 |
| Integer compiler max RSS (KiB) | 88,100 | 88,108 | — |
| Floating compiler max RSS (KiB) | 88,904 | 88,900 | — |
| runtime text bytes | 161 | 161 | — |
| memory-runtime text bytes | 149 | 149 | — |
| floating-runtime text bytes | 2,169 | 1,522 | — |
| pressure-runtime text bytes | 567 | 557 | — |

All compiler paired medians meet the diagnostic <=15% latency/RSS budget.
The compiler results do not establish a speedup: scheduling variation is large.
Selection does not duplicate IR. MIR transformations replace instructions in
place; generated text is unchanged or smaller. Integer and memory executables
are byte-identical to entry, so their noisy wall differences are not code changes.
Floating runtime improves in every ABBA block and shrinks 2169 -> 1522 bytes.
Pressure text shrinks 567 -> 557 bytes, but timing overlaps noise; no pressure
runtime speedup is claimed. Runtime maximum RSS is 256 KiB except a 308 KiB
pressure observation, retained in the manifest.

The positive carried-reload relationship is required by the focused course
control. Its bounded work and code reduction are established; a repeatable
isolated pressure timing gain is diagnostic, not an extra exit gate under
stage-scoped acceptance. Correctness, native relationships and all
course size bounds remain binding. The optional XMM reuse has independently
repeatable runtime profit, no code growth and no measured compiler regression.

All paired ratios and spread (seconds):

- accepted-integer/compile: paired 0.933, 0.928, 1.018, 0.852, 1.080, 1.009; A range 0.883–1.732, B 0.889–1.792; A/A 1.436–1.742.
- accepted-floating/compile: paired 1.157, 0.975, 0.974, 0.624, 0.861, 0.841; A range 1.101–1.935, B 1.084–1.962; A/A 1.108–1.262.
- final-regression/runtime: paired 1.018, 0.945, 0.819, 0.965, 0.797, 0.972; A range 0.537–0.643, B 0.416–0.609; A/A 0.580–0.642.
- final-regression/memory-runtime: paired 1.069, 1.008, 0.977, 0.974, 1.002, 0.975; A range 0.609–0.738, B 0.612–0.772; A/A 0.626–0.633.
- final-selection/floating-runtime: paired 0.458, 0.459, 0.460, 0.458, 0.457, 0.457; A range 1.176–1.202, B 0.544–0.549; A/A 1.185–1.189.
- final-selection/pressure-runtime: paired 1.119, 0.837, 0.999, 0.937, 0.933, 0.682; A range 1.697–3.920, B 1.616–2.431; A/A 2.100–2.686.

Work/effect evidence: `--stats` now exposes parameter worklist visits, carry
window visits, XMM reuse, carried reloads, input-pool growth/capacity and phase
latency/peak RSS. These count existing work without extra analyses. The large
floating input records 262144 XMM reuses; scalar inputs with fewer than five
parameters perform zero parameter-flow work. Carry probes are capped at three
64-instruction windows per private store. Parameter flow has six monotonic bits
and deduplicated enqueues; each edge can contribute at most six changes. No
candidate causes an unbounded whole-function retry.
