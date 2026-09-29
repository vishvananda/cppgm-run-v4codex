# PA22 compact plan — implementation handoff 115

Target: **PA22 full-stage**. Phase: **implementation; incomplete handoff**.
Stage base commit: `a8482d768bd2dcede42ea63ef39e39cf3245c380`.
Last reviewed commit: `a8482d768bd2dcede42ea63ef39e39cf3245c380`.
115 entry HEAD: `5a21daff2c280c8b3a55000e253c50773fe6a6da`, **47/99**, 52 failures.
Current: **91/99**, 8 failures; 44 entry failures repaired, zero regressions.
All 99 contract cases, oracles and comparison rules remain unchanged.

## Design/spec alignment and ownership

114's member representation, conversions, bounded local value proof and single-vptr
void cast remain in place ([prior evidence](performance114.md)). `277c1c44`,
`7336cd8d`, `2fd83db0` complete the dependent member-pointer type/argument/query
group and adjacent owner lookup, ADL, access and parser obligations:

- `Types` owns canonical owner TypeId + member TypeId; the owner reuses the
  existing bound slot, with a Named entity projection for concrete consumers.
  Formation, signatures, substitution, deduction, partial ordering and pack
  traversal all preserve that identity, including nested aliases and owner packs.
- Template argument queries retain canonical member declarations, null values,
  qualifiers, lexical/naming access context and explicit-instantiation context.
  Completed address arguments cache success by query/target/access identity;
  type/frame substitution memoizes completed success and invalid-type failure.
  Non-class owners, void/reference members and invalid NTTP conversions reject
  through compact substitution results. Body demand remains separately owned.
- Ordinary and unevaluated `.*`/`->*` share operand/ref/cv/base-path checking.
  Concrete bound calls retain receiver and member identity. Immediate NTTP
  targets publish one static fact consumed directly by typed LowIR; no lowering
  lookup, invented AST, textual phase transport or new optimization pass.
- Template-name lookup merges inherited injected names of one primary before
  ambiguity resolution ([temp.local]/4); ordinary type lookup stays ambiguous.
  ADL walks member owner/result edges. Typedef lookahead skips member-pointer
  qualifiers instead of accidentally predeclaring them as new typedef names.

TU arenas/flat indexes own the new facts. Work is O(affected type/query edges +
required candidates), memoized per type/environment; packs visit unique graph
edges. Known NTTP targets add O(1) recorded facts and bounded constant lowering,
with zero code growth over generic lowering. Non-dependent nodes are reused;
no source region is reparsed. Function-local traversal scratch is released by
its owner; no global cache, broad retry or unrelated-body demand was added.

## Unfinished implementation and handoff boundary

The remaining eight cases belong to three owners:

1. Runtime member-pointer value proofs: `300-const-member-function-pointer-address-call`,
   `300-repeated-nested-owner-member-template-address`, and spec cases
   `300-member-pointer-parameter-variadic-deduction` /
   `300-overloaded-member-pointer-function-template-deduction`. Their templates
   now form/deduce correctly; mismatches concern adjustment/truth extraction from
   unknown parameters or indirect storage. A class-layout shortcut is unsound
   for inverse conversions. Required completion needs a sound call/storage value
   analysis with ABI/exposure boundaries, or a proved contract correction. No
   reference was changed and this obligation is not waived.
2. Multi-base conversion lookup and ranking: `300-inherited-explicit-bool-condition`
   and `100-qualified-sizeof-overload-static-const-nttp`. The conversion candidate
   collector still follows only the first base; ranking also lacks the required
   preference between base targets in the composite hierarchy.
3. Initialization/lifetime output: `300-data-member-pointer-reference-cast-address`
   lacks zero-initialization of a nested nonaggregate base-bearing object;
   `300-structured-bool-conditional-member-pointer-dead-branch` materializes an
   unnecessary condition temporary instead of the contract's constant storage.

This boundary follows completion of the dependent-type/query group, extended
through all directly related parser, access, ADL and injected-name fixes.
Further fixes require runtime value provenance across callers/indirect storage,
base-path conversion candidate/ranking ownership, or initialization/lifetime
plans; another type-substitution change cannot supply those facts. All remain
implementation work for the full-stage objective, separate from review below.

## Performance and validation

[115 performance evidence](performance115.md) records frozen binaries/inputs,
A/A calibration, ABBA observations, compiler latency/RSS and checked runtime/text.
The immediate-target comparison includes generic correct lowering as a baseline;
mandatory O0 target shape and bounded fact/lowering costs are distinguished from
optional optimization claims. Inherited numeric diagnostic targets remain
non-gating under spec §9; mandated correctness, coverage and bounds are retained.

Validation is recorded in
[`validation115.json`](../student.tests/pa22/validation115.json).
Stage result: `make test-pa22` **91/99**. Prior isolated report: **3712/3712**.
File audit passes with the same three inherited header warnings. Personal
controls cover canonical owners, owner-only packs, cv/ref signatures, NTTP
forwarding/nulls, ADL, protected access and rejection: **12/12** (entry **6/12**);
all **16/16** inherited controls also pass. All **44** repaired fixtures validate
and roundtrip stably; **38** executable comparisons match, five are declaration
only, and one lacks the same external definition in both lanes. None substitutes
for the unchanged course checks. Stage progress is **52→8** original failures.

## Handoff ledger and independent review

114 was progress, not an abandoned implementation. 115 resumed a clean HEAD;
`d0c76b40` recorded ownership before edits and preserved both review markers.
The later continuation preserved committed work; missing process handles and
process inspection established that interrupted validation jobs had stopped.
Concurrent root reports share `.test_counts`; contaminated counts were discarded
and final required reports were rerun sequentially. No harness was modified.
Production behavior is committed through `2fd83db0`; the final evidence commit
records the completed handoff. PA22 remains incomplete; Ralph owns the next
implementation/audit scheduling decision.

Independent review remains pending for 114 and 115. Review must inspect canonical
owner and argument identities, access-sensitive cache keys, demand boundaries,
NTTP target-fact consumption, inherited local write/alias proof coverage and
performance acceptance. These are review questions, not substitutes for the
known unfinished implementation. Whole-stage findings must be resolved before
advancement. Neither review marker nor any implementation obligation is waived.
