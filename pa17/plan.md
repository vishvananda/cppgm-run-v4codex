# PA17 implementation plan

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`

Target: **PA17 full-stage**. Entry: clean, 132/343 required tests passing
(211 failures); prior through PA16 passes. Review markers remain fixed during
implementation. No PA17 implementation has yet been independently reviewed.

## Design and work groups

- **Specialization shapes and template-template entities (active):** semantic
  template arguments, type substitution and class-pattern selection own canonical
  identities. Parsed declarations feed typed parameter/argument facts, then
  per-primary candidates and the selected definition/environment. Work follows
  argument edges and eligible candidates; pair ordering is cached by immutable
  candidate IDs. Cover arrays, function/cv shapes, packs, nested template heads,
  parameter matching and alias substitution together where these share ownership.
- **Member/friend declaration ownership (unfinished):** retained member heads,
  out-of-class definition attachment, concrete owners, ADL/access/hiding.
- **Explicit instantiation and specialization demand (unfinished):** declaration
  versus definition, suppressed emission, late definition/selected owner.
- **Dependent syntax and remaining integration (unfinished):** definition-time
  typename/template checks, current instantiation, nested/base/alias cases.

Use shared source regions and canonical semantic facts; no grammar replay,
textual semantic keys, broad retry scans or output delegation. Later PA18
substitution/SFINAE and PA24 native optimization remain distinct owners.

## Validation and performance

Run focused controls explicitly from `student.tests/pa17`, then required PA17,
prior through PA16 and source file audit. Preserve all 343 course cases, status
oracles and comparison rules. Freeze entry/final compiler binaries and inputs;
A/A and ABBA observations record compiler wall time/RSS and native runtime/text
size through the supplied validation backend. PA17/O0 has no mandated numerical
performance ceiling; inherited diagnostic targets are not exit gates. Investigate
avoidable regressions; document necessary semantic costs without claiming
optimization benefit from newly accepted inputs.

## Handoff ledger

Implementation in progress. Initial related group is not a stopping boundary.
Independent review remains pending for all changes; unanswered review questions
are separate from unfinished implementation above. Final ledger will record
completed behavior, test delta, performance evidence and concrete remaining boundary.
