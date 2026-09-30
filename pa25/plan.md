# PA25 implementation135

Stage base commit: fee6ad9076ff35c5272526e1c4c4df235fbf3bfe
Last reviewed commit: fee6ad9076ff35c5272526e1c4c4df235fbf3bfe

Entry: clean worktree, 0/101 required cases pass (101 stub failures).
Previous goal turn: no implementation evidence in this thread; the fresh
inspection confirms the driver remains unimplemented. PA24 audit accepted.

## Design and remaining groups

1. Driver/object/link ownership: one source TU -> shared streaming frontend ->
   typed LowIR -> function-local native selection -> compact relocatable native
   image. Source, object and mixed invocations share this path. An explicit
   binary object adapter persists bytes, typed relocations and symbol records;
   production never serializes LowIR. Link-time ABI spellings are boundary keys,
   never semantic identities. Local symbols remain per-object IDs; external
   definitions resolve once, respecting strong/weak binding and aliases.
2. Include/target/library options and foreign ELF helper objects: preprocessing
   owns ordered include paths; driver owns input/options; linker owns native
   symbol resolution, relocation and startup order. No host compiler/linker is
   used to implement output. Selection state dies per function, frontend per TU.
3. Source/runtime integration findings: measure the real failure groups after
   enabling the driver; repair related semantic/ABI ownership defects together.
   Source EH may require a distinct runtime group beyond PA24's LowIR EH model.

Linking should be linear in bytes/relocations plus average linear symbol lookup,
with flat indexed identities after interning and no repeated global retries.
O0 adds no optional optimization or code growth; native policy is inherited.

## Validation and performance

Required: PA25, root through PA24/25, dev/src file audit; explicitly run personal
controls for object identity, relocation, parity and malformed objects. Freeze
correct binaries/inputs for A/A and ABBA measurements after enabling behavior;
report compiler wall/RSS and checked runtime/text together. The entry stub is
not a semantically equivalent performance baseline. Preserve PA24 measurements;
its diagnostic 15% targets are not additional stage gates.

## Handoff ledger

Implementation unfinished: all groups above. Independent review pending:
object boundaries, coalescing/alias identities, lifecycle/runtime and stage
performance evidence. Neither implementation nor independent review is waived.
No handoff boundary has yet been reached.
