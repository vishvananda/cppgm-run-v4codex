# PA29 compact plan — implementation171

Target: **PA29 full-stage**. Phase: **implement; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous review: `f07f78236eb475648834ed78afbca6c864f64408`.
Audit entry: `ecb69d94227d60653a7874f9daea63be8a202840`.
Last reviewed commit: `221d6d0e4930da05db2913bdf5f50d808f89c744`.

## Reviewed ownership and fixes

The [accumulated audit](audit.md) reviews every commit and combined source change
across implementation167–169: canonical vector layout and deferred inline demand,
aggregate/designated/compound initialization and mutable constant storage, and
block-pointer types/calls/ABI/RTTI. It includes shared query/template/lifetime
interactions and the registered implementation sources. The [range record](../student.tests/pa29/evidence170/range.json)
contains twelve entry commits plus the audit fix.

Audit170 fixes two shared owners: aggregate constant-call keys now include every
changed address-bearing value on the storage owner, and dependent vector-list
queries use canonical lane counts and existing initialization checks. Neither
adds source replay, semantic reconstruction, global retries or textual transport.
Integrated source→typed LowIR→MIR→ELF controls cover the handoff interactions.
The tested code was committed before this records-only update.

## Validation and stage-scoped performance

- PA29 **361/403**; exactly the same **42 failures**, no new failures.
- PA1–28 **4538/4538**; through PA29 **4899/4941**, with PA29 alone unfinished.
- File audit passes with the same four inherited header warnings.
- **376** explicit behavioral checks and **377** inspection checks pass.
- [Validation](../student.tests/pa29/evidence170/validation.json),
  [coverage](../student.tests/pa29/evidence170/coverage.json) and
  [failure delta](../student.tests/pa29/evidence170/stage-delta.json) preserve all
  403 inputs and 1,707 contract paths against entry and the previous review.

[Performance170](performance170.md) reports latency/RSS and runtime/text together:
**1,384** final observations plus six launchers, with A/A calibration and ABBA
pairs. Common and inherited affected A/B images and work counts are identical.
The pointer correction retains linear scalar-array work and adds complete keys
without whole-array snapshots. Audit-only capability costs use N=600/1200/2400;
entry rejects those inputs. Compiler growth is **88 bytes**. All historical
measurements remain; no speedup is claimed. Additional optional optimizer work
and growth budgets are zero. Historical blanket 15%/zero-growth targets remain
diagnostic under spec §9; mandated limits, correctness and coverage are unchanged.
Broader hosted runtime, optimizer/allocation and self-hosting retain PA30–34 scope.

## Remaining broad work

The [remaining ledger](../student.tests/pa29/evidence170/remaining.json) preserves:
extended syntax/types/layout **27**, template demand/hosted ABI **13**, legacy
trait/contract **1**, and source-invocation intrinsic operands **1**. This includes
numeric representations, templated lambdas/folds/bindings, conditional explicit,
zero-length arrays, packs/aliases, hosted emission and source coordinates.
Retain alignment, dependent offsetof ABI and class-convertible-index reducers.
Aggregate mutation, including this audit's pointer alias gap, is resolved.

Both trait oracle questions were reviewed. Ordinary reducers demonstrate the
language behavior, but reserved-name/`std` rules prevent a strict proof for the
original fixtures; references and counted failures remain unchanged. The optional
external Clang block value-catch observation remains documented. No reference
correction, implementation waiver or coverage reduction occurred.

Separate vector ABI and aggregate-query followups added avoidable fragmentation.
Complete each broad owner through direct/fixed/dependent/query use, ABI emission
and mutable storage dependencies before handoff. Independent review through the
recorded code tip is complete; full through-PA29 success is still required before
advancing. Earlier audits and the single audit170 ledger row remain in audit.md.

## Active implementation171

Entry HEAD: `6525c1af86e05edcf558186adec96a8c72521500`; baseline 361/403,
42 failures. Prior turn classification: progress (audit170 corrected storage/query
facts and recorded verified evidence). Stage base and last-reviewed markers above
remain unchanged.

Initial owner: builtin template type operations and argument-pack generation.
Parser retains argument nodes once; semantic builtin queries own canonical typed
argument tuples; substitution composes immutable frames and resolves selection or
generation; existing alias, deduction, class demand and typed lowering consume the
result. Work is linear in consumed/emitted arguments, with O(1) indexed selection
after substitution; generation keeps the existing 1,048,576 element limit.
No optional optimization or growth is introduced. Validate direct/dependent,
empty/nested packs, aliases, SFINAE, ABI and executable uses; extend to related
pack substitution defects while this owner is understood.

Freeze entry/final binaries, inputs and flags; collect A/A and ABBA common-path
latency/RSS/runtime/text evidence plus capability scaling for newly accepted
inputs (entry rejection is not an equivalent timing baseline). Required stage,
prior-through and file-audit gates remain unchanged. New implementation remains
unreviewed; independent audit is separate from unfinished language groups.
