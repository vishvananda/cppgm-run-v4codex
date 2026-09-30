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
