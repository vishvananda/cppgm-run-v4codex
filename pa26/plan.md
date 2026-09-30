# PA26 implementation handoff

Stage base commit: 369c57f19fa13c0394d4fb0345cf2bb79708fc67
Last reviewed commit: 369c57f19fa13c0394d4fb0345cf2bb79708fc67

Target: **PA26 full-stage**. Phase: **implementation142, awaiting independent
review**. Code boundary: `041cf554f499c4574b72294d23af897459e0147a`.
Entry **0/30** -> **29/30**; PA1–25 **4253/4253**; through PA26 **4282/4283**.
No contract fixture, reference, harness, comparison rule or coverage was changed.

## Design and completed behavior group

| Owner | Data flow / complexity | Evidence |
|---|---|---|
| Lowering/runtime identities | Typed host policy chooses RTTI imports, catch signature and runtime ABI spellings; ELF aliases share one output symbol. Existing local binding and terminate helper retained. Effect-free implicit destruction uses the same completed semantic fact as declaration emission. | Runtime declaration/builtin alias, local lambda, terminate and class-condition controls. |
| Native EH | LowIR clauses/regions -> per-function MIR frame/selector facts -> exact encoded call ranges. Interned persistent region stacks, one visit per reachable block per analysis, identity-checked joins; inactive cleanup frames permit shared suffixes. Separate executable reachability prevents dead cleanup coverage. | Same/cross-TU catches, callee saves, stack-call cleanup, conditional lifetimes, 140-type actions and selector remapping. |
| Host ELF writer | Native bytes/fixups -> ELF symbols/relocations, sparse LSDA and prologue/epilogue CFI. Indirect personality/type references support PIE. One physical resume terminal per function; cleanup paths remain distinct in LowIR/MIR. | Course inspection facts, host linking/runtime, production template LowIR/MIR/ELF trace. |

Work/storage is linear in emitted instructions, edges, clauses, bytes and
relocations, using flat identity indexes and final linear symbol partitioning.
Selection state dies per function; only encoded bytes, fixups and compact unwind
records survive to object writing. No text phase transport, host code-generation
subprocess, optional optimization, global retry or new semantic name lookup.
Private emission allocates no host landing-table storage.

## Unfinished implementation and concrete boundary

- Required `300-shared-conditional-cleanup-resume` still fails: preprocessing
  cannot find `<string>`. The checkout has no hosted C++ header/library path.
  This fixture remains required despite the README's general hosted-library
  exclusion. Supplying a fixture-shaped string shim would bypass the missing
  general header/library integration; that is a separate implementation group.
- Driver migration remains unfinished: `.obj` preserves PA25 private objects;
  other output names select host ELF. `--object-format=elf|private` overrides
  this choice. Host objects use PA26's external host linker. Uniform host output
  for arbitrary names and integration with the private compile/direct/mixed link
  path require reconciling its private EH/TLS/runtime ABI with host objects.
  Do not infer full command-line or host-link closure from the `.o` tests.

These boundaries need new header/library and link-driver work; they are not
remaining local fixes to the completed host EH encoding group. The target has
not been narrowed and PA26 is not certified complete.

## Performance and ledger

[Performance142](../student.tests/pa26/performance142.md) preserves candidate and
final frozen A/A + ABBA observations, all four dimensions, exact flags/hashes and
stage-scoped acceptance. No speedup is claimed and no optional transform added.
Inherited 15% latency/RSS and blanket zero-growth diagnostics are not new gates;
mandated facts, correctness and coverage remain required.
[Validation142](../student.tests/pa26/evidence142/validation.json) records commands,
checks, inventories and hashes. Personal controls and inspection are run explicitly.

| Handoff | Disposition |
|---|---|
| implementation142 | Host EH/ELF group committed; 29 original failures removed. Header/library and unified driver migration remain unfinished. |
| Independent audit | **Pending**, not waived: review whole-stage source-to-ELF identity/lifetime flow, CFI/LSDA legality, exception spill ownership, format-policy compatibility and performance evidence. Review markers above are unchanged. |
