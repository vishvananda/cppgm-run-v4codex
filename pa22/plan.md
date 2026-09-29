# PA22 compact plan — implementation 118

Target: **PA22 full-stage**. Phase: **implementation**.
Stage base commit: `a8482d768bd2dcede42ea63ef39e39cf3245c380`.
Last reviewed commit: `e90fa3fa514e2990bbe5ea4716252d8c42ae782a`.
Entry `853c4493`: **94/99**, five failures. Current: **94/99**, the same five.
All 99 contract cases, references, statuses and comparison rules remain unchanged.

## Active ownership group (118)

Entry HEAD: `0c710f745f1341536992f43b24ba325578a2fad3`; frozen compiler
`/tmp/pa22-118/entry`. Previous goal turn was progress: audit 117 repaired
canonical member constants and preserved its independent review findings.
The stage/review markers above remain unchanged.

Finish runtime member-value provenance for local object storage: semantic
checking owns canonical local projection identities and per-use zero-adjustment
facts; typed lowering consumes those facts without reconstructing aliases.
Use one bounded, conservative flow traversal of demanded bodies, no grammar
replay, global retry or parameter assumptions. Calls, unknown writes, lifetime
and control boundaries end local facts. Preserve generic parameter/escaped/
adjusted paths. Validate positive nested-template and sibling-storage cases,
inverse conversions, aliases, calls, branches, loops and lifetimes. Measure
frozen entry/final compiler latency/RSS and checked runtime/text with A/A+ABBA;
measure scaling and justify any optional analysis using a correct generic lane.
Then revisit the adjacent parameter/truth and constant-condition failures with
the established ownership evidence; no reference correction is assumed.

## Reviewed ownership and evidence

The [audit](audit.md) reviews all 15 commits across handoffs 114–116 from the
stage base, their combined source changes, and both audit fixes. No handoff is
left unreviewed. `015feb8d` fixes wrapper-aware adjustment proofs, member-pointer
and conversion-result hierarchy ranking, and completed base-path reuse.
`e90fa3fa` replaces declaration-only member constants with canonical
member/displacement values shared by constexpr evaluation, NTTP/ABI identity,
static data and typed lowering. Lowering no longer reconstructs their hierarchy.

The [final gate record](../student.tests/pa22/audit117-final-validation.json)
records PA1–PA21 **3712/3712**, file audit pass (three inherited header warnings),
**58/58** personal controls and **95** stable accepted LowIR roundtrips plus
**four** preserved rejection statuses. Audit controls improve **9/22→22/22**;
inherited controls remain **36/36**. Final runtime and compile measurements are
in [performance117.md](performance117.md), with all historical 114–116 evidence
preserved. Fourteen common correct inputs retain identical LowIR/native text.
The local proof has repeatable runtime/text benefit against its correct generic
lane; final timing noise and median regressions are disclosed without speed claims.

Spec §9's PA22/O0 acceptance applies. Inherited +15%, +16 MiB and 5.5× diagnostics
remain non-gating. Correctness, coverage, comparison rules, the 64-node proof
budget, constant per-operation lowering growth and eight-element initializer
expansion cap are preserved. Later native/debug/self-host work adds no PA22 gate.

## Remaining implementation groups

1. **Runtime member-value provenance and required lowering.** Finish the general
   cases `300-const-member-function-pointer-address-call` and
   `300-repeated-nested-owner-member-template-address`, and spec cases
   `300-member-pointer-parameter-variadic-deduction` and
   `300-overloaded-member-pointer-function-template-deduction`. Parameters and
   indirect storage need sound value/exposure facts; owner layout alone cannot
   prove zero adjustment. Retain inverse, assigned and escaped-value controls.
   Any reference correction needs the authorized reducer and standard/contract
   proof, with its bundle revision; none was made in this audit.
2. **Constant-condition materialization and demand.** Finish
   `300-structured-bool-conditional-member-pointer-dead-branch` with coordinated
   receiver effects, temporary lifetime and static constant emission.

These five failures remain implementation obligations, not waived review issues.
Virtual inheritance, polymorphic multiple inheritance and broader RTTI remain
PA23 work. Pass the full PA22 through report before advancing.

## Handoff discipline

The audit ledger has one row covering the complete range and both fixes.
114–116 made progress but split adjacent fixes, controls and telemetry into
avoidable small follow-ups. Future handoffs should complete a broad ownership
group with its interaction controls and measurements. This audit's code tip was
validated and committed before these records; the records commit changes no code.
