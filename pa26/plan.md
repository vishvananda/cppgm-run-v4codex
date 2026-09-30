# PA26 final disposition

Stage base commit: `369c57f19fa13c0394d4fb0345cf2bb79708fc67`
Last reviewed implementation: **`78410bfc`** (including `53ac444b` and `8c541d1e`).
Target: **PA26 full-stage**. Phase: **audit complete**.

Required checks pass: **4283/4283 tests**, **26/26 stages**; PA26 **30/30**.
File audit passes with four inherited nonfatal header warnings. No identified
PA26 blocker or unaudited implementation handoff remains. The supplied 4446
count disagrees with the authoritative entry and final reports; protected
fixture inventories prove coverage unchanged.

## Final Spec Alignment

Immutable sources feed the streaming interned cursor and integrated canonical
semantic graph. Retained template patterns and typed demand/substitution facts
feed direct LowIR, bounded per-function selection/allocation and direct ELF
emission. The independent [audit144](audit144.md) reconstructs this architecture,
traces `Guard::~Guard` and demanded `calculate<11>` through final native layout,
and reviews fact identity, worklists, invalidation and allocation lifetimes.
Textual token/IR/MIR/assembly views do not transport production phases.

Default `-c` produces host ELF for every output name. A typed `ElfModule` carries
symbols, sections, relocations, FDEs, lifecycle arrays and TLS into either the
writer or the cumulative linker; explicit file input alone uses the ELF reader.
Separate, direct and mixed source/object links preserve required behavior.
The compiler emits static or dynamically linked executables itself. Configured
runtime DSOs provide ABI symbol metadata and runtime implementations; no host
compiler, assembler or linker implements user-source compilation or own-link
output. Host final linking remains the separate PA26 test-harness boundary.

The original private ABI remains available explicitly with
`--object-format=private` in both compile and link modes. It is never selected
by an output suffix. Richer host RTTI/rethrow/library compatibility, general DSO
linking and self-hosting retain their handout stage boundaries.

## Findings and completed repairs

| Owner | Final result |
|---|---|
| Native image / ELF writer | Buffers transfer into the writer and final sections stream once; eliminated duplicate whole-object storage. |
| Driver / typed ELF / linker | Uniform default format; symbol extents, alias/weak definition identities, lazy GOT demand, lifecycle and FDE facts survive every path. Binary magic recognizes extensionless inputs; malformed named objects still use bounded binary validation. |
| Host EH regions / layout / LSDA | Outer catches remain visible through partial-construction cleanup. Shared action suffixes, complete selector keys and saved raw selectors preserve same-frame dispatch; one physical resume terminal remains. MIR inspection exposes the consumed facts. |
| TLS / ELF / executable | Separate TLS bytes/fixups, STT_TLS/TPOFF32 and aligned PT_TLS preserve values and independent threads, including separate/mixed TUs. |
| Language linkage / storage | Unbraced linkage declarations carry implicit extern facts; braced definitions remain definitions. C++11 `[dcl.link]/7` proof and reduced imported-data control retained. |

## Performance acceptance

[Performance144](../student.tests/pa26/performance144.md) preserves the original
280 observations and adds **448** final frozen A/A + ABBA samples on unchanged,
checked template, memory/call/loop, floating, EH, string and large-storage inputs.
Compiler latency/RSS, executable runtime and text are reported together.

The final 32 MiB object reduces peak compiler RSS **138188 -> 39996 KiB (71.1%)**.
No precise compiler speedup is inferred from noisy timing. Plain workload text
is byte-identical; the repaired host EH workload adds three text bytes. Own-link
EH runs faster with the host runtime (paired ratio 0.115, range 0.107–0.145), with
higher process RSS and bounded dynamic/unwind metadata explicitly disclosed.
Shared-library text is excluded from executable `.text` and no total-footprint
reduction is claimed. No speculative optimization or new growth/search budget
was introduced.

Inherited 15% latency/RSS and blanket zero-growth targets remain diagnostics
under spec section 9; all historical measurements survive. Mandatory native/EH
limits, correctness, comparison rules and coverage remain required and pass.

## Validation and ledger

[Final validation](../student.tests/pa26/evidence144/final-validation.json) pins
exact code/binary/contract hashes and serial command results. Both required exit
commands pass, as do the owning stage's object inspections. Additional evidence:
87 link/EH/TLS commands, 62 driver controls, four output-name cases, 9 host ABI
controls, 10 header prerequisites, 22 intrinsic/trait controls, DSO function
identity and the fresh typed template trace. Explicit private-mode compatibility
retains 178 binding/layout and 74 exception checks.

[Final ledger](../student.tests/pa26/evidence144/final-ledger.json) includes text
hashes, phase/work counters and intermediate failure provenance. One intermediate
PA3 timeout and a parallel build-configuration race are preserved; final isolated
serial checks pass with unchanged limits. No reference, fixture, harness or
comparison-rule correction was made.

| Handoff | Disposition |
|---|---|
| implementation142 | Host EH/ELF architecture independently reviewed; controls repeated; writer buffer ownership repaired. |
| implementation143 | Header prerequisites, typed traits/intrinsics, exception demand and GOT function addresses reviewed; controls repeated. |
| audit144 checkpoint | `8c541d1e` / `7ba3d541` findings and measurements preserved; output-name defect followed through all consumers. |
| audit144 final | `53ac444b` closes the ownership-path defects; `78410bfc` exposes final MIR facts. Architecture, performance and exact final checks accepted. |
