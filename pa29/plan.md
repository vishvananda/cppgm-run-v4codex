# PA29 compact plan — implementation159

Target: **PA29 full-stage**. Phase: **implementation; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `1ab3499d7046daf5c298d958a8770b413edb3615`.
Audit entry: `764305061a4d88a8bb9088216ac31c2ab8395a08`, clean, **316/403**.
Current: **317/403**, **86 failures**, zero new failures; all 403 sources retained.
The extra course pass is the [proved reference correction](reference-correction158.md).

## Reviewed ownership

[Audit158](audit.md) covers every commit from the stage base through the reviewed
code tip, including all three handoffs and their combined source changes.
Hosted probes/driver metadata, canonical traits, runtime/scalar builtin signatures,
exact floating storage, declaration alignment, empty-member layout and member
paths retain shared compiler ownership. Parser → semantic graph → typed LowIR →
per-function MIR → direct ELF remains the production path.

The audit fixes retain GNU `using` alignment operands through alias substitution
and carry optional storage types through expression facts, queries, casts,
indirection and selected/indirect/template returns. Language type equality and
callable ABI signatures stay canonical. Complete query keys retain raw storage
inputs; shared expression properties and sparse declaration facts own them.

## Broad remaining implementation groups

[Fixture ledger](../student.tests/pa29/evidence158/remaining.json) enumerates all
86 failures. Labels guide reducers; they do not prove each root cause.

| Owner / failures | Required work and completion evidence |
|---|---|
| Atomic/assembly: 21 | Atomic storage, ordering/effects and assembly constraints → typed LowIR/native operations; layout, noexcept, runtime controls. |
| Extended syntax/types/layout: 37 | Canonical extended numeric/complex types; designated initialization, folds, lambda and binding syntax; required vector width/layout and unused-wrapper validation. Selected source modes and fixtures define scope. |
| Legacy traits/lifetimes: 5 | Special-member, binding/materialization and lifetime facts with positive/negative reducers. The forward-declared `std` trait oracle remains unchanged and requires separate resolution. |
| Template demand/hosted ABI: 19 | Complete specialization/context keys and demand edges; packs/aliases/caches, pretty-function, extern/inline and emitted symbols. |
| Structured intrinsic operands: 4 | Constant-evaluation context, address-of/fence semantics, invoke receiver/member-pointer recipes and lexical/caller source-location facts. |

Unfinished extension paths include code-alignment placement, dependent `offsetof`
in ABI signatures and class-convertible designator indices. Recognizing extended
float suffixes with legacy precision is not an implementation of extended types;
that remains in the numeric owner above. No passing parse is a substitute for
required layout, semantics, lowering or ABI behavior.

The README excludes runtime vector lowering; an inherited plan must not turn
that into a PA29 gate. Broad hosted headers belong to PA30, general hosted
execution to PA31, optimization/allocation to PA32/33 and self-hosting to PA34.
Required PA29 behavior and current limits remain mandatory.

## Validation, evidence and next checkpoint

- `make test-pa29`: **317/403**, exit 2; failure set strictly shrank by one.
- `make test-report-through-pa28`: **4538/4538**, exit 0.
- File audit: exit 0, four inherited substantial-header warnings.
- Explicit controls: **34/34** new, **77/77**, **40/40**, **53/53** inherited;
  inspection **10/10**, exact LowIR roundtrip and telemetry byte equality.
- [Validation hashes](../student.tests/pa29/evidence158/validation.json) and
  [performance158](performance158.md) preserve commands and all observations.
  Cumulative paired compiler medians 1.0107–1.0335, audit-only 1.0075–1.0296;
  common generated objects/executables are identical. New capability has linear
  demand counters. Necessary storage-fact costs are disclosed; no speedup claimed.
- New optional optimization work/growth budgets: **zero**. Historical blanket
  15%/zero-growth targets are diagnostic under spec §9, not extra exit gates.
  Measurements, mandated limits, correctness and coverage are preserved.

Continue in cohesive owner groups spanning parser, semantics, lowering and
validation. The earlier scalar and layout groups were sensible, but successive
small publication/access/metadata fixes and repeated plan/evidence handoffs
fragmented review unnecessarily. Close each ownership path before handing it off.
Run `make test-pa29` and the root through report; full PA29 acceptance still
requires all remaining failures to be resolved. This audit does not advance stages.

## Active implementation159

Entry HEAD `044d9627e7ed4f466426e1f0470c58646dc9b592`, clean, 317/403.
Previous goal turn: progress (audit158 changed authoritative code, evidence and
required-suite result). Stage/review markers above remain unchanged.

Owner group: legacy construction/copy/assignment and reference-temporary traits.
The shared builtin registry feeds probes and parsing; canonical type queries
feed existing special-member/exception and conversion recipes; constants flow
through template substitution and typed lowering. No body demand merely to
answer a trait. Inspect required member families/selected conversion only, cache
by operation and canonical operands, TU lifetime; no whole-program search.
Validate dependent/nondependent, cv/ref, access/deletion, exception and
materialization cases, runtime constants, LowIR and telemetry; extend related
trait failures as the same facts support them. Freeze entry/final binaries;
measure common A/A and ABBA compile/RSS plus runtime/text, and new-demand
scaling. Optional transform work/growth budget remains zero.

The forward-declared std trait fixture remains an independent contract question
recorded by audit158, not a reason to inject library-name semantics. All other
groups above remain unfinished implementation. This is not a handoff boundary.

First increment: shared registry plus six legacy member traits, three lifetime
traits and ordered direct-reference conversion selection. Course 321/403 (four
resolved, no new failures); explicit controls 41/41. Before handoff, close the
identified selected-subobject triviality edge and rerun all required checks.
