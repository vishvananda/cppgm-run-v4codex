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
