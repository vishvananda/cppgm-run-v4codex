# PA24 implementation ledger

Stage base commit: bde9eb3e128e24923a1de40bb63b8e348a13553b
Last reviewed commit: bde9eb3e128e24923a1de40bb63b8e348a13553b

## Design and current group

Initial state: scaffold driver; 0/435 reported passing. No native implementation.
Implement the common integer execution path before extending other ABI classes.
Owners/data flow: PA8 typed Unit -> per-function native selection/placement ->
compact typed MIR -> shared MIR dump and x86 encoder -> label/fixup ELF layout.
No text phase transport, host assembler/compiler, or reference delegation.
Function-local state is released after encoding; program symbols and fixups survive
until image layout. Selection/allocation/encoding should be linear or near-linear;
constant-size target register sets and conservative frame homes bound placement.
Validate scalar widths, memory/addressing, branches/phi, calls and register effects
as related groups rather than recognizing individual fixtures.

## Remaining implementation

- Driver, scalar integer MIR, encoder, executable/data layout and symbol fixups.
- Control flow, scalar memory/address forms, integer call ABI and liveness.
- Floating/f80, i128, direct/indirect object ABI, variadics, atomics/bulk memory.
- Required MIR quality/layout controls and complete course validation.

## Performance and acceptance

O0 native selection only; no optional optimization pipeline in PA24. Measure
latency/RSS plus executable runtime/text bytes for fixed checked integer workloads.
Freeze binaries and inputs, retain raw ABBA/A-A observations when comparing correct
implementations. The initial scaffold has no executable baseline; do not claim a
speedup against failure. No self-selected timing limit overrides spec stage scope.

## Handoff / independent review

Implementation ongoing; no handoff yet. Independent audit must inspect the typed
boundary, register/ABI effects, complexity and performance evidence; review markers
above remain unchanged during implementation. Missing implementation is not an
independent-review question and no assignment requirement is waived.

Progress 1: implemented scalar native foundation, 199/296 root oracle fixtures
passing (baseline 0/296); focused controls reached the frame-copy shape check.
The larger Ralph inventory includes additional checks; coverage is unchanged.
First-pass bugs in copy direction and atomic operands were corrected against
PA8's explicit src/dst and expected-pointer contracts. Extended shared validation
for integer consumers, by-address scalar actuals and bounded typed slot reads;
phi types and unmarked pointer parameters retain strict checks. Remaining native
scalar failures are primarily placement/quality, not a handoff boundary yet.
