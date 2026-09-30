# PA26 independent audit144 — in progress

Stage base: `369c57f19fa13c0394d4fb0345cf2bb79708fc67`.
Entry: clean `0af58619`. Reviewed implementation span: `66ed6469` through
`a624085e`, plus repair `8c541d1e`. **Not accepted for advancement.**
The output-name defect below prevents final Spec Alignment, even though both
required checks pass. No fixture, reference, comparison rule or coverage changed.

The entry handoffs were treated as claims to verify. No live job from a previous
turn was found; fresh source inspection, a fresh root report, independent object
comparisons and new measurements supplied the evidence for this checkpoint.
This turn makes implementation and evidence progress, not a verified wait.

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
   are explicitly different facts, which matters at the remaining driver gap.
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

The nontrivial trace is `student.tests/pa26/trace.cpp`: `Guard::~Guard` mutates a
global, while demanded `calculate<11>(argc)` constructs a Guard, throws an int,
cleans up and catches it. The fresh trace records 86 tokens, maximum 10 pending,
136 parsed nodes, 58 compact occurrences, one specialization, one body transition,
one substitution frame and five inherited expression facts. It lowers 71 LowIR
instructions into three native functions/82 MIR instructions and 460 text bytes.
Production LowIR/MIR, symbol, relocation and CFI views verify the same facts;
the host-linked program returns zero. With `PATH=/nonexistent`, compilation
produces the identical object. [Inspection and counters](../student.tests/pa26/evidence144/validation.json)
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
| Spec 8: object-buffer ownership | **Repaired**: native code/data/LSDA copied into HostElf, then copied into final file storage. They now move once and stream once. 37 object hashes unchanged. |
| README command line/output format | **Open defect**: suffix `.obj` changes default output to `CPPGMOBJ`; arbitrary requested names must receive host-compatible ELF. |
| Prior stages and file audit | Required root report 4283/4283 across 26 stages; file audit passes with the four inherited header-division warnings. |

`student.tests/pa26/output_policy144.py OUT COMPILER` is an independently reduced
contract test. One demanded template, destructor and typed catch compile to `.o`,
`.obj`, extensionless and `.bin` outputs. Three outputs host-link and run; `.obj`
is private, so the test intentionally fails on current code. The source and
structured failure evidence are retained. This is not a diagnostic target that
can be reclassified under performance acceptance.

A suffix-only change is insufficient: `source` passes `o.host` into both lowering
and native compilation. Private begin-catch has an extra temporary-storage
parameter, private EH uses handler-stack runtime entities, and TLS uses private
storage/startup. The private linker also needs producer-owned definition/role
facts absent from the current foreign ELF adapter. A correct repair must follow
these ownership paths while preserving PA25 direct/separate/mixed linking; it
must not select by fixture names, change the harness, add a serialized semantic
fallback or delegate compiler implementation to a host compiler. General private
host-runtime integration remains outside PA26, but that scope rule cannot make
the demonstrated output-format violation disappear. Final design is pending.

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
five workloads, including a targeted large-object case. The measured memory
benefit has an ownership explanation; noisy timing is disclosed. Historical
142/143 evidence and measurements remain unchanged.

The inherited 15% latency/RSS and blanket zero-growth targets are diagnostics,
not PA26 exit gates. Required semantics, coverage, native property limits and
inspection contracts remain mandatory. Hosted completeness/self-hosting are
later-stage constraints, not additional PA26 benchmarks to implement now.

## Validation and handoff ledger

- Entry and repaired `make test-report-through-pa26`: **4283/4283**, 26 stages.
  The supplied 4446 count differs from both fresh logs. Contract hashes and the
  protected-tree diff against the stage base establish unchanged coverage.
- `make test-pa26`: **30/30**, all object inspections included.
- Required file audit: pass, four inherited nonfatal header warnings.
- Explicit 9 host ABI/format, 10 header and 22 intrinsic/trait controls pass;
  imported-function identity and production template inspection pass.
- 37 ELF objects are byte-identical A/B; both writers report `/dev/full` failure.
- The independent arbitrary-output-name reducer fails for `.obj`; **final
  acceptance is withheld**. [Validation144](../student.tests/pa26/evidence144/validation.json)
  pins code, binary, contract, command and evidence hashes.

| Handoff | Review result |
|---|---|
| implementation142, `3f7bc44e` / `041cf554` | Reconstructed typed EH region/encoding/CFI/LSDA/ELF paths and private separation; repeated explicit controls. Writer ownership defect repaired. |
| implementation143, `e4df536c` through `a624085e` | Reviewed host configuration, typed queries/intrinsics, demand ordering and PIE function addresses; repeated header/intrinsic/DSO checks. Broader extension completeness remains a later hosted surface. |
| audit144, `8c541d1e` | Ownership repair validated and measured. Output-name gap independently reproduced, ownership implications recorded. Full audit and its final acceptance remain active; next work is the driver/object compatibility repair. |
