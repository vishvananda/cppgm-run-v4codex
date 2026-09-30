# PA24 scalar backend evidence

Run personal validation explicitly from the repository root:

```
python3 student.tests/pa24/scalar.py
python3 student.tests/pa24/integration.py
python3 student.tests/pa24/status.py # after make test-pa24
```

The arithmetic generator uses independent Python modular-integer calculations,
including negative division/remainder and unsigned interpretation of signed
operands. The integration test checks call argument cycles/stack arguments,
atomic expected-storage updates, width-changing copies, byte swapping, native/MIR
consistency, multi-file references, helper-only debug dumps and separate RX/RW
ELF loads. Required course fixtures remain unchanged.

`benchmark.py A B OUTPUT_DIRECTORY` freezes deterministic large LowIR input and
measures a compiler workload and two runtime workloads separately, with four A/A
samples and six ABBA blocks each. Runtime loop counts come from process argc;
computed results are checked for two argument counts before timings. The script
pins all samples to one allowed CPU and retains binary/input hashes, every wall
and peak-RSS observation, telemetry/text sizes, paired ratios and spread.

The backend's new state has the following owners:

- LowIR's unit owns canonical names, types, operand IDs and external validation.
- One function selector owns placement facts, alias relationships, frame bindings,
  bounded copy/edge decisions and flat MIR. The unit keeps compact lookup indexes.
- One encoder owns code/data buffers, symbol offsets and typed fixups. Function
  labels are reused; the ELF writer resolves references after code/data layout.
- The MIR dump reads selected facts; enabling it does not change executable bytes.

No assembly or textual MIR transports implementation data between these owners.

Final measurements and acceptance rationale are in [performance.md](performance.md).

Handoff128 extends the checks to floating execution and scalar ABI state:

```
python3 student.tests/pa24/floating.py
./dev/lowir2native -o /tmp/pa24-parameter-flow student.tests/pa24/parameter-flow.lowir
/tmp/pa24-parameter-flow
```

`floating.py` checks arithmetic against Python/Decimal, exhaustive ordered/NaN
comparisons, truth and negation, signed/unsigned conversions, XMM call cycles,
stack arguments, call-crossing values, f32/f64 parallel phi transfers, and mixed
variadic register/overflow arguments. f80 phi remains outside LowIR's contract.
`parameter-flow.lowir` exercises both incoming-carrier and post-call home paths,
a deferred address across a call, high parameter pressure, and a loop backedge.

`benchmark.py A B DIRECTORY [WORKLOAD ...]` can select `floating-runtime` and
`pressure-runtime` in addition to the original two workloads. Set
`PA24_FLOAT_COMPILER=1` for 4096 floating helpers with 64 typed operations each.
The floating runtime sums input-dependent kernel results and the pressure runtime
checks an input-dependent recurrence. See [performance128.md](performance128.md)
for the current frozen experiments, including all earlier observations.

`PA24_COMPILE_ONLY=1` reruns compiler measurements alone after telemetry-only
changes. Executable hash identity establishes reuse of the frozen runtime
measurements; it never substitutes for checking both runtime input counts.

Handoff129 adds wide numeric and object ABI checks:

```
python3 student.tests/pa24/wide.py
python3 student.tests/pa24/objects.py
```

`wide.py` independently computes modular arithmetic, signed/unsigned division,
ordered comparisons, shifts and nearest-even FP results. It also covers atomic
success/failure updates, wide variadic overflow, parallel phis, direct branches,
switches, narrow/wide call boundaries and full-width decimal literals.
`objects.py` checks every byte across partial tails, direct/indirect calls,
register exhaustion and large stack objects (160 programs).

`PA24_WIDE_COMPILER=1` selects 4096 helpers with 64 wide operations each.
`wide-runtime` checks high-word call/phi traffic; `wide-numeric-runtime` checks
input-dependent division and all three FP conversion formats. Both check results
with two runtime argument counts. See [performance129.md](performance129.md).

Checkpoint audit130 adds independent controls for fixed-register reuse, implicit
wide operands (including slots), indexed spill stores, hidden byte-multiply
effects, floating sign bits and x87 rounding restoration:

```
python3 student.tests/pa24/audit130.py
dev/cppgm++ --validate-lowir --emit-lowir -o /tmp/pa24-trace130.lowir student.tests/pa24/trace130.cpp
dev/lowir2native --dump-machine-ir /tmp/pa24-trace130.mir -o /tmp/pa24-trace130 /tmp/pa24-trace130.lowir
/tmp/pa24-trace130
/tmp/pa24-trace130 input
```

The audit runner optionally accepts a compiler path and an evidence directory;
it keeps every reduced input, MIR view and executable there. The source trace
checks ordinary field access and a demanded class-template member while an
invalid unused dependent member remains dormant. These are explicit tool-boundary
checks; source/native driver integration remains a later milestone.

Handoff131 adds generic native exception and frame lifetime checks:

```
python3 student.tests/pa24/runtime131.py
dev/cppgm++ --validate-lowir --emit-lowir -o /tmp/pa24-trace131.lowir student.tests/pa24/trace131.cpp
dev/lowir2native --dump-machine-ir /tmp/pa24-trace131.mir -o /tmp/pa24-trace131 /tmp/pa24-trace131.lowir
/tmp/pa24-trace131
/tmp/pa24-trace131 input
```

The 75 runtime/frame cases cover scalar payload widths, nested cleanup/resume,
cross-function transfer, normal and early returns, bounded handler loops,
dynamic-allocation lifetimes, high-pressure parameter homes, over-aligned
storage and stack-passed objects. The runner accepts a compiler and evidence
directory and retains inputs, MIR, executables and a JSON result list. The source
trace checks the layout of a demanded aligned class template; dynamic allocation
is tested at the explicit LowIR boundary.

`PA24_RUNTIME_COMPILER=1` selects the fixed 4096-function compiler workload with
handler regions, dynamic allocation and aligned storage. `exception-runtime`
checks an input-dependent sum through eight million mixed normal/throwing calls
at the timed argument count. Entry binaries cannot execute this new surface;
use the same frozen completed binary as A and B to measure its absolute costs
and noise, without claiming a speedup. See [performance131.md](performance131.md).

Handoff132 adds initial-thread TLS storage, accessors and address consumers:

```
python3 student.tests/pa24/tls132.py dev/lowir2native /tmp/pa24-tls132
python3 student.tests/pa24/tls-integration132.py dev/lowir2native /tmp/pa24-tls-integration132
dev/cppgm++ --validate-lowir --emit-lowir -o /tmp/pa24-trace132.lowir student.tests/pa24/trace132.cpp
dev/lowir2native --dump-machine-ir /tmp/pa24-trace132.mir -o /tmp/pa24-trace132 /tmp/pa24-trace132.lowir
/tmp/pa24-trace132
/tmp/pa24-trace132 input
```

The 236 TLS controls preserve the existing outcomes while changing eligible
personal copies of course globals to thread storage, then add independent scalar,
conversion, derived-address, wrapper, pointer-argument, function-table and bulk-copy
cases. Course fixtures and comparisons are untouched. The integration controls
inspect real TLS instruction/debug facts, initial-thread setup before initializer
functions, wrapper fixups across files, helper-only dumps and view/native identity.
The source trace follows ordinary TLS access through a demanded class-template
member; its unused invalid member remains undemanded.

`PA24_TLS_COMPILER=1` selects 4096 distinct TLS globals and helpers with load,
arithmetic and store work. `tls-runtime` checks forty million accessor calls
and increments at the timed argument count, with a second runtime input checked
before timing. The entry binary rejects TLS; measure the final binary against
itself for absolute cost/noise. See [performance132.md](performance132.md).

Handoff133 completes the canonical scalar placement/frame protocol:

```
python3 student.tests/pa24/placement133.py dev/lowir2native /tmp/pa24-placement133
dev/cppgm++ --stats --validate-lowir --emit-lowir -o /tmp/pa24-trace133.lowir student.tests/pa24/trace133.cpp
dev/lowir2native --stats --dump-machine-ir /tmp/pa24-trace133.mir -o /tmp/pa24-trace133 /tmp/pa24-trace133.lowir
/tmp/pa24-trace133
/tmp/pa24-trace133 input
```

The 205 controls independently check weighted argument permutations, indirect
targets in register/stack positions, GPR/XMM classification, stack and bulk-copy
dependencies, Boolean conversions across calls/edges, fixed effects, adjacent
conversion debug locations and view/native byte identity. The source trace
follows a field and demanded template member through the mixed numeric ABI while
an unused dependent member remains dormant.

`PA24_ABI_COMPILER=1` selects 4096 callers with 64 mixed calls each. `call-runtime`
checks input-dependent parity through an indirect target; `mixed-abi-runtime`
checks a varying mixed-domain sum. Both run at two argument counts before timing.
See [performance133](performance133.md) for the frozen A/A + ABBA evidence,
including the initial overbroad frame policy, its correction, the measured
forwarding benefit and the mandatory canonical mixed-ABI cost.
