# PA26 independent audit144 — final architecture review

Stage base: `369c57f19fa13c0394d4fb0345cf2bb79708fc67`.
Entry: clean `0af58619`. Reviewed implementation span: `66ed6469` through
`a624085e`, plus repairs `8c541d1e`, `53ac444b` and `78410bfc`.
Final code marker: **`78410bfc`**. The uniform-output defect is closed across
its compiler, ELF, EH, TLS and linker ownership paths. The final acceptance
record below uses fresh checks on that code. No fixture, reference, comparison
rule or coverage changed.

The entry handoffs were treated as claims to verify. No live job from a previous
turn was found; fresh source inspection, a fresh root report, independent object
comparisons and new measurements supplied the evidence for this checkpoint.
The whole-stage reconstruction was followed through the final ownership repair;
checkpoint conclusions were not substituted for source review or validation.

## Reconstructed architecture and representative data flow

1. `toolchain/driver.cpp` builds a `lowir_model::Program` through
   `lowering::build_program`. `host_config.cpp` supplies build-time target roots
   and predefined metadata; it runs no host compiler on user input. Immutable
   source storage feeds `PPTokenSource`, post tokens and `syntax::Cursor`'s
   geometric lookahead ring. Tokens borrow spelling or use interned identifiers.
   The parser calls semantic construction as declarations complete; locations
   and minimal source topology belong to the TU graph.
2. `syntax/occurrence.cpp` retains parsed source regions and projects compact
   `(context, source)` occurrences. Bodies/defaults/nested definitions are
   separately demanded. `template_instantiation.cpp` substitutes raw parameter
   facts and checks the body without invoking grammar. `template_type_facts.cpp`
   interns frames by specialization, parameter slice, parent and arguments.
   Non-dependent semantic facts remain inherited; changed declarations publish
   `(frame, pattern)` identities. These are source/semantic IDs, not rendered
   type or mangling keys.
3. `semantic/declaration.cpp::finish` advances per-owner demand cursors; body,
   storage, member, vtable and exception work share the scheduler without
   restarting all pending entities. Exception specifications use distinct
   Active/Success/Failure states; evaluated body facts precede deferred exception
   queries. Header traits use typed query facts and completed class property
   caches; template integer packs have linear output and an explicit 1048576
   resource ceiling. Local mutation does not clear a process-global cache.
4. `lowering/source_exception.cpp`, `exception_handlers.cpp`,
   `full_expression.cpp` and `destruction.cpp` consume selected declarations,
   conversions, destruction effects and lifetime states. They construct typed
   LowIR calls, catch clauses, selectors and cleanup/resume paths directly.
   `finish_exception_boundary` inspects the function's published call boundaries
   once, and rebuilds only its instruction slice. One local terminate adapter
   owns begin-catch followed by terminate. Host and private runtime signatures
   are explicitly different facts, which now remain explicit format/ABI choices.
5. `native/driver.cpp` creates one `Selector` and MIR `Function` at a time.
   Clause records retain TypeInfo symbol IDs; placement reserves typed host
   exception/selector slots and dynamic-stack floor when needed. The region
   walk interns parent/landing stacks and requires equal state at joins; its
   reachability walk follows throwing-call edges rather than mere registration.
   Liveness/call epochs, parameter flow and private home identity drive placement.
6. `native/layout.cpp` encodes native instructions and records final call ranges.
   Host landings save RAX/RDX state, translate LSDA-local selectors, restore the
   stack floor and branch to source cleanup/catch blocks. All semantic resumes
   branch to one physical terminal which reloads the active exception slot.
   `host_tables.cpp` merges only equal continuations; unprotected throwing sites
   remain explicit null-landing barriers. It writes sparse ULEB call sites and
   SLEB action chains. CFI follows actual prologue/save/restore byte offsets.
7. `toolchain/host_elf.cpp` maps output names once, emits typed relocations and
   lifecycle arrays, and `host_unwind.cpp` emits CIE/FDE and indirect personality
   and type-info cells. Calls use PLT32; externally bound function addresses use
   GOTPCREL. Names here are the external ELF adapter's symbol spellings, not
   keys used to redo semantic lookup. Repair `8c541d1e` moves native buffers
   into the writer and streams final sections after layout. No serialized IR,
   assembly parser, external assembler or compiler implements these phases.
8. Final `ElfModule` owns typed symbols, section buffers, relocations and FDE
   identities. Compile output serializes this model once; direct source linking
   consumes it directly through `link_elf`, without emitting and rereading ELF.
   Explicit ELF file input uses the bounded reader into the same model. Symbol
   sizes, TLS classification, FDE references and lifecycle arrays are retained
   facts, not recovered from source or instruction bytes.
9. The cumulative linker merges producer definition identities and retains the
   existing once-per-definition/edge demand worklist. Purely resolved programs
   use the compiler's static ELF writer. Host runtime demands use its new
   executable writer: configured DSO symbol tables resolve only live imports,
   function veneers and eager GOT cells preserve canonical addresses, COPY
   relocations support direct imported-data access, and final FDE identities
   form a sorted unwind search table. TLS retains a separate byte/fixup lane,
   STT_TLS/TPOFF32 and a correctly aligned PT_TLS image. ELF program headers,
   dynamic metadata and the SysV libc startup call are emitted here; no external
   linker or compiler constructs these executables.

The nontrivial trace is `student.tests/pa26/trace.cpp`: `Guard::~Guard` mutates a
global, while demanded `calculate<11>(argc)` constructs a Guard, throws an int,
cleans up and catches it. The original independent trace recorded 86 tokens, maximum 10 pending,
136 parsed nodes, 58 compact occurrences, one specialization, one body transition,
one substitution frame and five inherited expression facts. It lowers 71 LowIR
instructions into three native functions/82 MIR instructions and 460 text bytes. The final
trace retains those work counts and emits 463 text bytes for the repaired EH state.
Fresh final production LowIR/MIR, symbol, relocation and CFI views verify the same facts and expose the saved raw selector/outer region;
the host-linked program returns zero. With `PATH=/nonexistent`, compilation
produces the identical object. [Final inspection and counters](../student.tests/pa26/evidence144/final-ledger.json)
record the hashes and command, rather than treating a checkpoint conclusion as
source-to-ELF evidence.

Allocation boundaries are explicit: source, AST, semantic slabs, interning and
facts die after TU lowering; the typed Program survives native emission;
selection/placement/MIR state dies after each function; only required image,
relocation and unwind records survive to object output. The ELF writer now owns
the transferred code/data/LSDA buffers and releases them on return, without a
second whole-object representation. Fixed output tables are bounded metadata.

ABI review used the [Itanium EH ABI](https://itanium-cxx-abi.github.io/cxx-abi/abi-eh.html)
sections 1.3/1.6/2.4/2.5 and the [LSB exception-frame format](https://refspecs.linuxfoundation.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/ehframechpt.html),
alongside the checked-in LowIR contract and PA26 README. The host unwinder owns
stack traversal; the compiler supplies throw/catch calls and final-layout tables.
No reference correction or bundle revision is proposed.

## Requirements and findings

| Requirement | Current evidence and disposition |
|---|---|
| README 1–5: throw, catch, personality, LSDA, cleanup/resume | Fresh 30/30 suite; incoming/outgoing typed EH, large frames and 140-entry actions in explicit controls; source/MIR/ELF trace. |
| README 6: host symbols and PIE-safe personality | Object-fact inspections, indirect personality cells, imported function pointer/DSO identity control pass. |
| README 7: region joins | Typed region-state equality; full-expression/conditional/short-circuit/loop fixtures pass. |
| README 8: local generated functions | Lowering's semantic linkage facts and ELF local binding; unchanged local-lambda inspections pass. |
| README 9/12: coalescing barriers and sparse coverage | Address-ordered typed call ranges; null entries separate distinct protected continuations; all sparse/barrier inspectors pass. |
| README 10/11: shared terminate and physical resume | Dedicated typed identities, one TU adapter and one per-function resume label; object inspections including outgoing stack arguments pass. |
| Spec 8: object-buffer ownership | **Repaired**: native code/data/LSDA copied into HostElf, then copied into final file storage. They now move once and stream once. 37 object hashes unchanged in the isolated writer repair; final ABI metadata changes are measured separately. |
| README command line/output format | **Repaired** in `53ac444b`: default output is ELF for every name; four names host-link and also pass the compiler-owned link path. |
| Prior stages and file audit | Required root report 4283/4283 across 26 stages; file audit passes with the four inherited header-division warnings. |

`output_policy144.py OUT COMPILER` retains the original reduced source: a demanded
template, destructor and typed catch. `.o`, `.obj`, extensionless and `.bin`
outputs now all host-link and run, and the compiler also links all four objects
with the same result. The original failure evidence is preserved separately.
Default source, separate-object and mixed links share host ABI facts. The
explicit `--object-format=private` option retains the previous ABI and format-4
reader/writer; linking that format requires the same explicit option. Mixing a
private ABI object into host mode diagnoses the incompatible boundary.

The repair exposed and fixed two deeper defects:

- Raw partial-construction cleanup regions hid same-frame outer catches from
  host personality search. Typed region parents now form shared LSDA action
  suffixes; the raw selector survives local cleanup/resume and transfers to the
  outer landing before any frame exit. Selector identity includes **both**
  source selector and type identity, so a generated catch-all cannot alias an
  ordinary typed catch. One physical `_Unwind_Resume` terminal per function is
  retained. The constructor function-try fixture and an independent reducer
  verify destruction precedes its catch and rethrow reaches the caller.
- Unbraced `extern "C"` object declarations were emitted as definitions. The
  parser preserves the linkage form; semantic declaration facts carry its
  implicit extern status into storage, initialization and destruction decisions.
  The reducer imports `stdout`; a separate control distinguishes braced
  definitions and unbraced declarations for C and C++ linkage. This follows
  [C++11 draft N3337, 7.5/7](https://timsong-cpp.github.io/cppwp/n3337/dcl.link#7),
  whose example distinguishes these forms. The resulting ELF undefined symbol
  reaches a real COPY relocation when required; compiler agreement is not the
  proof. No reference correction was needed.

The typed link adapter preserves weak definition/alias ownership and lazy GOT
retention. Named malformed `.o`/`.obj` inputs still enter the bounded reader;
valid extensionless objects are detected by magic. This avoids treating a
truncated binary as C++ source. The inherited corruption controls still exercise
private format 4 explicitly; no negative expectation was removed.

Dynamic output follows the [System V ELF dynamic contract](https://refspecs.linuxfoundation.org/elf/gabi4+/ch5.dynamic.html),
[LSB libc startup ABI](https://refspecs.linuxfoundation.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/baselib---libc-start-main-.html)
and [glibc x86-64 entry register/stack contract](https://github.com/bminor/glibc/blob/master/sysdeps/x86_64/start.S).
DSO files are read as ABI metadata, never loaded into the compiler or invoked.
The resolver owns only requested name IDs and releases each library buffer;
there is no process-global symbol cache or external tool subprocess.

## Optimization and performance review

PA26 uses the existing O0 native policy; accepting `-O1` does not introduce an
unbounded or speculative pass. Parameter-home promotion requires one entry
store and only nonvolatile, same-type nonescaping uses. Load/address folding
requires an adjacent consuming instruction and extends carrier liveness first.
Boolean reuse follows the proven 0/1 range. `carry_reloads` limits each candidate
to 64 instructions and three scratch registers, rejects unknown/call/alias
effects, and preserves debug records. Its passes are linear with a fixed window
factor; it changes no CFG or frame ownership. Per-function analysis storage is
released, so no cross-function invalidation is needed. Unknown proofs retain
the memory form. Deferred edges use their predecessor's clobber state.

Host-EH sharing and sparse coverage are required representation choices. Their
work and output are linear in MIR, calls, clauses and relocations; they do not
justify new runtime-speedup claims. There is no inlining, unrolling, code cloning
or fixed-point search in this repair. Existing PA24 native property checks stay
required. [Performance144](../student.tests/pa26/performance144.md) reports frozen
A/A + ABBA compiler latency/RSS, checked executable runtime and text size for
five workloads, including a targeted large-object case. The final record adds host-object and
compiler-owned executable measurements on frozen correct/correct workloads.
The measured memory benefit has an ownership explanation; timing variation and
host ABI costs are disclosed. Historical
142/143 evidence and measurements remain unchanged.

The inherited 15% latency/RSS and blanket zero-growth targets are diagnostics,
not PA26 exit gates. Required semantics, coverage, native property limits and
inspection contracts remain mandatory. Hosted completeness/self-hosting are
later-stage constraints, not additional PA26 benchmarks to implement now.

The new link/CFI machinery is also bounded: section merging and relocation
work are linear in bytes, symbols and live edges; sorting definitions and final
FDE search entries is O(n log n). Persistent region nodes, reachability queues,
shared action suffixes and selector identities are indexed and visited once.
The region join check rejects incompatible state instead of retrying a global
fixed point. There is no code cloning or speculative optimization. Import work
is bounded by the finite configured runtime symbol tables plus requested names;
it neither interns every export nor caches whole libraries across links.

A raw selector adds four bytes beside the translated selector in an eight-byte
frame home; only landings with an outer region save it. This is necessary
exception-state ownership, not a runtime optimization. Frame, unwind and MIR
inspection facts agree. Code/RSS/runtime costs are evaluated under the stage
policy; later hosted-library, richer RTTI/rethrow, arbitrary DSO and self-host
completeness remain outside PA26. The old private path remains explicitly
available and its richer inherited controls are preserved there.

## Validation and handoff ledger

Final code `78410bfc` passes the serial validation manifest:

- `make test-pa26`: **30/30**, including all mandated object/EH inspections.
- `make test-report-through-pa26`: **4283/4283**, **26/26 stages**. The supplied
  4446 count disagrees with initial and final authoritative reports; protected
  fixture and harness inventories prove coverage unchanged.
- Required file audit: **pass**, the same four inherited nonfatal header warnings.
- **87** explicit link/EH/TLS commands: direct, separate, mixed and host links,
  constructor cleanup/catch ordering, selector identity, independent thread TLS,
  imported function/data identities, lifecycle arrays and foreign FDEs.
- **62** driver controls retain weak/GOT/alignment/corrupt-object coverage.
  **178/178** binding/layout and **74/74** exception compatibility controls pass
  using the explicit private mode; their original checks and expectations stay.
- **9** host ABI/format, **10** header and **22** intrinsic/trait controls pass,
  alongside the imported-function DSO/PIE check and the fresh typed template
  trace with raw-selector and outer-region MIR inspection.
- All four arbitrary output names compile to host ELF, host-link, compiler-link
  and execute correctly. PATH-empty commands establish self-contained emission
  and linking. Explicit reference tools remain observation-only.

[Final validation](../student.tests/pa26/evidence144/final-validation.json) pins the
binary, implementation and protected contract hashes and every command status.
An intermediate root run reported a PA3 `300-triple` timeout; the subsequent
serial validation passed the unchanged test
and the entire report. Its failed log hash is retained, not treated as a waived
limit. A separate overlapping two-target build raced on `.compile_config.tmp`;
serial final validation avoids that build-infrastructure collision.

| Handoff | Final disposition |
|---|---|
| implementation142, `3f7bc44e` / `041cf554` | Typed EH, CFI/LSDA/ELF and private separation independently reconstructed and tested; writer ownership repaired. |
| implementation143, `e4df536c` through `a624085e` | Header configuration, typed queries/intrinsics, demand ordering and GOT addresses reviewed; header, intrinsic and DSO checks repeated. |
| checkpoint `8c541d1e` / `7ba3d541` | Writer repair and 280 observations preserved; open output-name diagnosis followed through all consumers. |
| final repair `53ac444b` | Uniform ELF, typed linking, dynamic startup/unwind, TLS and linkage declaration facts reviewed end to end; complete ownership-path controls and root report pass. |
| inspection cleanup `78410bfc` | MIR exposes actual raw-selector and outer-region facts; final inspection and required checks pass on this marker. |

No unaudited implementation handoff remains. No reference, fixture, course
comparison or mandated limit was changed. The compact plan and final performance
record carry the stage acceptance; historical evidence remains separately named.
