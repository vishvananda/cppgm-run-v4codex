# PA29 assembly ownership — implementation163

The seven resolved fixtures share GNU assembly statements, including an assembly
statement inside the unused attribute/template fixture. References and discovery
remain unchanged. This is a completed behavior-group handoff, not PA29 completion.

## Ownership and data flow

- `syntax/assembly.cpp` owns GNU statement grammar, qualifiers, named operands,
  constraints and clobbers. `syntax/assembly_template.cpp` parses the instruction
  template once into source-owned typed recipes. Compact colons (`:::`), numeric
  and named matching inputs, early-clobber outputs and concatenated literals use
  the ordinary token cursor. Operand expressions are children in the sole graph.
- An assembly recipe has a stable TU-local index and a flat instruction range.
  Substitution projects operand expression edges; it shares the source recipe.
  Semantic binding checks nondependent lookup at definition and dependent value
  types on demand. Read/write category, constness, machine width, bit-fields,
  immediate constants and matching widths are checked before lowering.
- Lowering evaluates each operand expression once, captures register inputs and
  output addresses, executes the recipe, then commits register outputs. Memory
  operands read/write their locations during the recipe. Full-expression and
  lexical lifetime records preserve temporary destruction and throwing operands.
  The instructions themselves do not unwind. No fake calls, semantic lookup,
  name reconstruction, source replay or assembly subprocess occurs in lowering.
- Moves, integer arithmetic, bit operations and byte swaps use existing typed
  LowIR. Exchange and additive locked operations use the existing atomic
  primitives. Other locked updates use one bounded-size CAS loop; contention is
  runtime work. All 1/2/4/8-byte integer storage is supported, with x86 locked
  memory operations independent of C++ atomic type identity. Compiler fences
  conservatively bracket every recipe. `mfence` uses the thread-fence primitive.
- The zero-operand, void `nop` and `pause` LowIR instructions are explicit tool
  extensions. Their reader, writer, shape/whole-program checks, MIR view and
  direct native encodings agree. They retain debug metadata and remain effects,
  never terminators or SSA values. Existing LowIR fixtures are unchanged.

Supported instruction recipes are `nop`, `pause`/`rep; nop`, `mfence`, `mov`,
`add/sub/and/or/xor`, `inc/dec/not/neg`, `bswap`, `xchg` and `xadd`, including
valid `lock` prefixes and AT&T byte/word/long/quad suffixes. Constraints cover
integer `r/q/m/i/n`, `=/+/&`, and register matches by number or name. Clobbers
are `cc` and `memory`. Unsupported machine instructions, explicit physical
registers, operand modifiers, asm-goto and other clobber/constraint forms reject;
there is no silent instruction discard. The PA29 required assembly fixtures use
this subset. This does not promise a general GNU assembler.

## Complexity and limits

Parsing is linear in template bytes, expressions and emitted recipes. Name and
matching lookup inspect at most 30 operands (the supported GNU operand bound).
Per occurrence, semantic/lowering work is O(operands + instructions), with no
whole-TU scans, cache invalidation, fixed-point compiler retries or additional
member-body demand. Per-statement scratch vectors are released on return;
source recipes live with the TU. Each instruction expands to a fixed number of
LowIR operations/blocks, independent of operand values. Existing phase telemetry
is supplemented with source statement/instruction counts, without extra walks.

No optional optimization is added: optional work and growth budgets are zero.
Additive atomics use the existing direct primitive; no CAS search is needed for
them. Required semantic work is measured in [performance163](performance163.md),
including compilation/RSS and checked runtime/text size. Existing constexpr,
native frame/data/alignment limits and course timeouts are unchanged.

## Validation and handoff boundary

Explicit controls cover register/memory aliasing, matching inputs, each supported
scalar width, both exchange operand orders, same-register xadd, templates,
evaluate-once operands, exception cleanup, noexcept boundaries, concurrent
locked updates, malformed operands/constraints and constant-evaluation rejection.
Inspection covers serialized LowIR to host objects, standalone execution,
native opcodes, MIR debug locations and malformed processor hints.

An initial inspection asserted host ELF DWARF line tables. This repository's
inherited host writer does not emit them. PA8 `lowir.md`, **Debug Locations**,
explicitly permits that backend boundary; PA29 adds no DWARF requirement. The
final inspection checks debug transport through LowIR/MIR and actual hint bytes
in ELF. This corrects an unsupported test assumption, with the initial outcome
retained in the validation evidence; it does not remove a course check.

No known correctness defect remains in the implemented recipe subset. Independent
review still owns scrutiny of recipe legality, effects and resource bounds; that
review is not waived by these checks. Remaining PA29 failures need different
owners: extended type/layout/syntax representation, template demand/hosted ABI,
and structured intrinsic context. Extending arbitrary assembly syntax would not
resolve those failures. Those owners and existing oracle questions remain in the
compact plan; this group boundary does not authorize advancement.
