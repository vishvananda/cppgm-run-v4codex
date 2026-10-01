# PA30 compact implementation plan

Stage base commit: `27029f978e65b78331233123922d342033d5d1f7`.
Last reviewed commit: `27029f978e65b78331233123922d342033d5d1f7` (PA29 boundary; PA30 review pending).
Target: PA30 full-stage. Phase: implementation195.
Previous goal turn: progress (PA29 audit completed); current entry clean.

## Design and active groups

The existing parser/semantic graph, canonical specialization facts, typed LowIR
and direct object path remain the owners. First group: heavy-header declaration
parsing (tuple constructor grammar), then its exposed semantic demand failures.
No header-name shortcuts, host compilation, reference or coverage changes.
Record each repaired group's owner, flow, complexity and checks below.

## Baseline and remaining work

Fresh `make test-pa30`: **105/153**, **48 failures**; all 153 inputs and
reference/harness hashes recorded in evidence195. Supplied 105/154 summary is
inconsistent with its raw log; fresh cases govern progress.
Most failures enter tuple constructor parsing; other groups include class
access/constant bounds, alias convergence, partial ordering, callable demand,
vector builtins, exception redeclarations and negative body validation.
Full-stage implementation remains open; independent architecture review remains
separate and unwaived. Extend related work while this ownership is understood.

## Performance and handoff ledger

Mandated compile timeout: 45 seconds per fixture. Freeze entry/final binaries,
inputs and flags; retain A/A and ABBA observations for compiler latency/RSS and
checked executable runtime/text where available. No optimizer change planned.
Spec stage scope applies: necessary semantics are measured, arbitrary inherited
percentage/no-growth diagnostic targets are not additional exit gates.

- Entry: recorded base/review markers before edits.
- Parser group: complete-class category lookahead recognizes assignment operators
  before initializer scanning; friend elaborated tags retain lexical lookup;
  memoized angle endings retain which half of `>>` closes the type. Owner:
  syntax cursor/prediction/declarator/class parser. Flow: compact source tokens
  -> indexed category lookahead -> the single retained syntax/semantic graph.
  Work: O(1) added per visited delimiter/specifier, no new grammar replay or
  semantic search; the existing live-token cache owns/releases the split bit.
  No LowIR/backend/emission/optimization route changes or code-growth policy.
  Reducers, positive/negative boundaries and direct/native LowIR controls:
  **28 commands pass**. Full required suites and performance pending.
- Handoff: pending implementation, validation, measurements and clean commits.
