# PA10 performance and architecture evidence

The completed O0 compiler passes PA10's complete output contract. The ownership
audit removes repeated static-reference traversal; it adds no generated-code
optimization pass. Its memory and compiler-code growth fit the predeclared
diagnostic budgets. Ordinary procedural/template workloads show no material
measured regression.

## Frozen comparison

[Protocol](../student.tests/pa10/performance-protocol.md) and the
[final raw record](../student.tests/pa10/final-performance.json) retain 108
compiler observations, eight startup observations, separate telemetry runs and
36 executable observations. A is implementation `17f3deb7`; B is `63592ef0`
(snapshot commit `27dc7dab`). Both use g++ GNU C++11/O3 and the course runner.
Binary/backend SHA256, flags, platform/CPU affinity, input/source/output hashes
and executable hashes are in the record. The unchanged course comparator
checks LowIR equivalence; inherited semantic dumps are byte-identical.

A/A calibration precedes two ABBA blocks. The original
[compiler campaign](../student.tests/pa10/performance.json),
[short runtime campaign](../student.tests/pa10/short-runtime-performance.json)
and [complete ownership-audit campaign](../student.tests/pa10/audited-performance.json)
remain intact, including outliers and 72 earlier executable observations.
Those continuations fixed the sectionless-ELF size adapter and enlarged short
workloads without changing the compiler. After the final sequencing correction,
the complete final campaign uses separately frozen binaries/artifacts and the
same inputs; it does not replace historical measurements.

## Compiler results

Wall times are median seconds; RSS is maximum observed KiB. Both paired ABBA
B/A ratios and the initial A/A relative range are reported for every workload.

| Workload | A wall | B wall | A RSS | B RSS | Paired B/A | A/A spread |
|---|---:|---:|---:|---:|---|---:|
| Calls/control, 3500 pairs | 0.3838 | 0.3853 | 71204 | 71356 | 0.9967, 1.0020 | 0.98% |
| Calls/control, 14000 pairs | 1.5779 | 1.5933 | 289732 | 289860 | 1.0096, 1.0055 | 1.54% |
| Memory/floating, 3500 functions | 0.3411 | 0.3418 | 65428 | 65376 | 1.0002, 1.0036 | 1.30% |
| Memory/floating, 14000 functions | 1.3832 | 1.3679 | 248608 | 248528 | 1.0070, 0.9472 | 4.79% |
| Template semantics, 3500 demands | 0.0673 | 0.0675 | 12196 | 12004 | 1.0052, 0.9991 | 184.73% |
| Template semantics, 14000 demands | 0.2527 | 0.2544 | 35656 | 35672 | 0.9961, 1.0068 | 0.93% |
| Static references, 800 | 0.0304 | 0.0165 | 6320 | 6356 | 0.5401, 0.5480 | 0.33% |
| Static references, 3200 | 0.3134 | 0.0481 | 12396 | 12604 | 0.1491, 0.2686 | 5.63% |
| Static references, 8000 | 2.8190 | 0.1133 | 21860 | 23924 | 0.0408, 0.0406 | 3.99% |

The small reference samples and template-3500 case are below the 20x-startup
diagnostic threshold (A startup 5.72 ms; B 5.28 ms). Their short durations and
outliers remain diagnostic: template-3500's A/A observations include 295 ms
among otherwise 66–67 ms samples. No speedup is claimed for ordinary workloads
or these short samples. The frozen 8000-reference case exceeds that threshold
for B, has 3.99% A/A spread and repeats roughly a 25x compiler speedup in both
ABBA blocks. Its peak RSS grows 2064 KiB.

B's fourfold-input latency ratios are 4.14 (calls), 4.00 (memory/floating),
3.77 (template semantics) and 2.92 (references, including startup). Fourfold
RSS ratios are 4.06, 3.80, 2.97 and 1.98 respectively. Compiler `.text` grows
from 532422 to 555718 bytes: +23296 bytes (+4.38%), including the explicit IR
audit boundary and required semantic corrections.

The self-selected 1.10x wall, 1.20x RSS +16 MiB, +128 KiB compiler text,
5.5x fourfold wall and 5x fourfold RSS +16 MiB diagnostic budgets are satisfied.
They are not additional course exit gates. The required stage base was a driver
stub and cannot provide equivalent full-stage measurements. PA9's naming-tool
budgets remain scoped to the unchanged naming tool. The A/B inputs share correct
semantics; newly fixed volatile discard, bool storage, padding and assignment
sequencing are validated separately, without comparing speed against wrong code.

## Generated programs

All paired executables are byte-identical and return the checked result 0.
The final samples use 96 million loop/call iterations, 64 million indexed
memory updates and 32 million floating calls. Local volatile bounds force
runtime reads; the checksum/result is observable. The supplied native backend
is used only by the evidence harness, after our compiler emits its own LowIR.

| Workload | A runtime s | B runtime s | Executable payload bytes, A/B | Paired B/A | A/A spread |
|---|---:|---:|---:|---|---:|
| Loops/calls | 0.47734 | 0.47733 | 206 / 206 | 0.9977, 0.9975 | 1.73% |
| Indexed memory | 0.28036 | 0.27894 | 434 / 434 | 0.9987, 0.9927 | 1.07% |
| Floating calls | 0.33191 | 0.33234 | 230 / 230 | 0.9909, 1.0027 | 0.99% |

The backend emits sectionless ELF. As in PA8, executable payload counts bytes
from ELF entry to executable PT_LOAD end; these workloads have no static
data. It includes code/padding rather than claiming a nonexistent `.text`
section. The original 21–34 ms observations remain diagnostic only. Identical
executable hashes establish that timing variation is not generated-code
improvement or regression. No executable speedup is claimed.

## Ownership and complexity audit

- The streaming frontend retains one source-faithful graph. Semantic EntityId,
  TypeId and NodeId facts select calls, defaults, conversions and addresses.
  Lowering consumes those facts; neither a text parser nor overload resolution
  transports/reconstructs them. PA9's typed graph encodes ABI entities.
- Static initializer facts are owned by the sealed semantic TU. The complete
  key is `(NodeId, target TypeId)` after declaration completion; active,
  successful and failed facts prevent recursive/repeated computation. For
  800/3200/8000 references, requests are 3200/12800/32000 and hits are
  1599/6399/15999: linear work with one computation per complete key.
- Function-local builders allocate numeric values, slots and blocks. Source
  names cannot redirect a generated temporary. Calls share a TU-owned operand
  scratch vector: each call releases its live range immediately, while capacity
  is recycled up to the maximum concurrently pending operands and released with
  the TU lowering adapter. No completed call body is retained in that scratch.
  Indirect signatures are cached by canonical TypeId.
  Dense mappings/pools and geometric growth replace per-operand strings and
  per-instruction owning vectors. The calls corpus produces 84000/336000
  instructions with 171/193 IR-pool growths, respectively.
- LowIR `SymbolId` owns top-level identity; spelling and ABI export spelling
  are stored once and kept distinct. Global constant addresses become typed
  relocation records. Field offsets are computed at their semantic layout
  owner. Volatility stays on the access; builtin memcpy's noalias boundary
  does not leak to memmove.
- Ordinary construction checks local shapes. `--validate-lowir` performs one
  explicit whole-program audit before writing; normal O0 emission does not
  repeatedly revalidate. All 123 successful course/control inputs pass it,
  also under ASan/UBSan with leak detection and halt-on-UB enabled.
- Per-TU frontend/ABI/static-fact arenas die after lowering. Function builders
  die after their body; the typed Program remains until the explicit LowIR
  output finishes. No duplicate semantic tree or serialized IR is retained.
  Whole-program LowIR retention belongs to this explicit output tool; later
  native function scheduling/release remains a backend-stage responsibility.

No optional fixed-point, inlining or code-growth transform was added. The
required power-of-two pointer difference uses an O(1) exact-divisibility proof;
other element sizes retain signed division. Unknown effects remain conservative.
Template lowering, allocator/runtime parity, ELF output and self-hosting
remain owned by their later assignments. The existing template-semantic
workload checks inherited frontend processing here.
