# PA22 compact plan — implementation handoff 116

Target: **PA22 full-stage**. Phase: **implementation; incomplete handoff**.
Stage base commit: `a8482d768bd2dcede42ea63ef39e39cf3245c380`.
Last reviewed commit: `a8482d768bd2dcede42ea63ef39e39cf3245c380`.
116 entry HEAD: `7b3685fc18c6e392465f733b40a9aca7f4aaad74`, **91/99**, 8 failures.
Current: **94/99**, 5 failures; three entry failures repaired, zero regressions.
All 99 contract cases, references and comparison rules remain unchanged.

## Design/spec alignment and completed ownership

114/115's canonical member pointers, dependent formation/arguments, selected
targets, bounded local proofs and single-vptr void casts remain in place.
See [114 evidence](performance114.md) and [115 evidence](performance115.md).
116 extends semantic conversion selection and aggregate initialization:

- `conversion_candidates` walks explicit nonvirtual base edges using canonical
  declaration/target IDs. Target hiding is scoped to the current inheritance
  path, restored before siblings. Same-class cv overloads enter together;
  deduplication retains one candidate per entity. Access and ambiguous object
  paths remain checked after selection. The selected conversion retains its
  existing base adjustment for direct typed lowering.
- `better` ranks standard derived-to-base class-value transfers by target
  hierarchy, alongside pointer/reference rules. Construction owns the actual
  subobject binding; ranking invents no second lowering adjustment.
- `prepare_value_initialization` owns omitted aggregate elements' zeroing before
  a non-user-provided default constructor. Its immutable action carries that
  fact and uses the canonical type's existing zero plan. Lowering consumes it
  for fields and bounded array repetition. User-provided and late-defaulted
  constructors retain their distinct behavior.

Lookup work tracks visited inheritance paths and local conversion declarations,
including required candidates in each base subobject; ranking uses the existing
hierarchy query. Per-call flat indexes/vector scratch own traversal work and
release it on return. No registry scan, global cache, token replay, broader body
demand or textual transport is added. Zero plans are TU-owned and cached per
canonical type; no per-element semantic reanalysis occurs. The existing
eight-element expansion bound keeps large initializer arrays in a loop.
These are required semantic operations, with no new optional optimizer.

## Unfinished implementation and concrete boundary

Five original failures remain, under different fact/lifetime owners:

1. Runtime member-pointer provenance: `300-const-member-function-pointer-address-call`,
   `300-repeated-nested-owner-member-template-address`, plus spec cases
   `300-member-pointer-parameter-variadic-deduction` and
   `300-overloaded-member-pointer-function-template-deduction`. Required shapes
   omit adjustment or truth extraction from unknown parameters/member storage.
   An owner-layout shortcut is unsound for inverse conversions. Completion needs
   a sound caller/storage value proof with exposure boundaries, or a reference
   correction proved by the standard/LowIR contract. Existing inverse, assigned
   and escaped-value controls remain required. No oracle was changed.
2. Constant-condition materialization:
   `300-structured-bool-conditional-member-pointer-dead-branch` selects the correct
   arm but differs in discarded receiver storage and static constant emission.
   Its conversion-body/result and temporary-lifetime owners are separate from
   omitted-element zero initialization.

The completed group extends the initial conversion fixes through sibling hiding,
ambiguous repeated bases, access, template conversions, class-value ranking,
and adjacent missing nested zeroing, including array/constructor variants.
Further conversion-candidate or initializer-action changes cannot prove values
across function/indirect-storage boundaries or decide constant receiver effects
and demand. Those require distinct provenance and materialization contracts;
they remain implementation work, not waived review questions. Full PA22 remains
the target; this boundary returns an incomplete implementation handoff to Ralph.

## Performance and validation

[116 performance evidence](performance116.md) records frozen binaries/inputs,
A/A calibration and ABBA compiler/runtime observations with peak RSS and text.
All eleven common/width/count inputs have identical A/B LowIR and native text.
Compiler text grows 1,024 bytes (0.045%); the largest conversion case adds 2.4%
median compile latency with equal peak RSS. Lookup counts match visited base
scopes. `InitAction` remains 56 bytes. No speedup is claimed. Spec §9's acceptance
applies; historical +15%, +16 MiB and 5.5× diagnostics remain non-gating, with all
measurements, correctness, coverage and mandated bounds preserved.

[`validation116.json`](../student.tests/pa22/validation116.json) records the gates.
`make test-pa22`: **94/99**; `make test-report-through-pa21`: **3712/3712**.
File audit passes with the same three inherited header warnings. Focused controls
improve **2/8→8/8**; inherited controls pass **28/28**. Three repaired cases
validate and roundtrip stably; two executables match references, one has no entry.
Stage progress is **8→5** original failures; no advancement is requested.

## Handoff ledger and independent review

115 was progress, with its type/query implementation and evidence committed.
116 resumed a clean checkout with no live validation process. `ca6b0cbb` recorded
ownership and entry state before edits and preserved both review markers.
`c016a259` implements conversion selection/ranking; `a5247137` records mandatory
omitted-element zeroing. `7b6d62b9` adds lookup telemetry/template controls;
`810a9ee1` extends the nested constructor control. Initial overlapping targeted
builds were terminated and rebuilt sequentially; reports never overlap their
shared `.test_counts`. The initial prior report had one PA3 timeout (3711/3712);
the exact case and then the final required report passed without any harness or
timeout change. Raw logs remain in `/tmp/pa22-116/`; structured evidence is saved.
The final evidence commit records this handoff; Ralph owns subsequent scheduling.

Independent review of 114/115 remains pending: canonical owner/argument identity,
access-sensitive cache keys, body-demand boundaries, immediate-target facts,
local write/alias proof coverage and stage-scoped performance acceptance. Review
116's path hiding/deduplication, class-value ranking and zero-action lifetime
coverage too. These questions are distinct from five known implementation
failures. Neither marker nor any obligation is waived; whole-stage findings must
be resolved before advancement.
