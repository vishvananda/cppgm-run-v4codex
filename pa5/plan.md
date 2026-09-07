# PA5 final plan and handoff ledger

Stage base commit: `a27ec8877221e4d9acea5f2f63b97855cc0fd365`
Last reviewed commit: `a27ec8877221e4d9acea5f2f63b97855cc0fd365`
Target: **PA5 full-stage**. Phase: **independent final audit in progress**.
Implementation entry **0/188**; current course result **188/188**.
Independent review reconstructed all syntax owners and shared source/post-token
handoffs through `924ba7dc6`; checkpoint conclusions were not used as proof.

Final audit found and fixed category lifetime leaks (parameters, enum and control
scopes), inconsistent qualified/import lookup, anonymous namespace identity,
array/function declarator classification, for declaration conditions, an unnamed
pack child-list invariant, and a cross-file concatenated literal source range. Personal regressions and stronger
graph/lifetime checks cover those ownership paths. AST indentation now reuses
one buffer; optional name/scope work counters observe existing work.

Profitability review: the isolated indentation experiment did not beat A/A noise
in both blocks and showed no RSS benefit. Remove that optional change before the
final freeze; retain the candidate and isolation datasets. The final source must
be remeasured, not assigned its predecessor's measurements.

Remaining: sanitizer/API checks; frozen final A/B measurements; final audit and
performance consolidation; cohesive commits; fresh exit gates and clean status.
Frozen A is the ordinary `924ba7dc6` binary. Final campaign retains the existing
10%+noise wall / 15%+1MiB RSS / 15% host-text / 6x wall / 5x RSS scaling budgets.
Before timing, fix identical source repetitions: 8 for nested, 4 for other
families, in one invocation. Use ordinary compilation, two A/A pairs, B/B and
two ABBA blocks, CPU affinity, eight startup probes, separate phase/work runs,
and calibrated ordinary/stats comparisons. Require workload wall >=25x startup;
retain every observation and compare exact outputs. No other build/tests overlap.

## Design/spec alignment

| Owner | Data flow and ownership | Complexity / validation |
| --- | --- | --- |
| Source/cursor | PA4 streaming PP → PA2 post cursor → deferred-token ring; immutable TU sources and interned IDs; physical/presumed locations and decoded literal payloads retained | One delimiter-index visit per token; API lifetime/location/literal checks |
| Syntax graph | One 32-byte node array with stable IDs, structured names/type-ids/template arguments and direct child links; dump is a separate view | Geometric TU arenas; no cloned token/AST streams or per-node ownership; all grammar regions parsed once |
| Declarations/statements/expressions | Shared specifier/declarator prefixes, precedence, classes/enums/namespaces/templates, control flow and special members | All 188 unchanged course cases; 25 personal cases plus graph API checks |
| Categories/prediction | Flat `(ScopeId, IdentifierId)` facts; explicit using/base edges; template parameter kinds override hints; class-wide category lookahead | Relevant scopes only; indexed angle/delimiter facts and once-per-ID lexical hints; no semantic-answer cache or global invalidation |
| Driver/rendering | Separate TU ownership in operand order; deterministic syntax view and ordinary failure exits | Full stage/through reports; output-equivalent frozen benchmark binaries |

[Architecture audit](audit.md) traces a template declaration, source locations,
allocation/release, lookup, parsing and view separation. Later semantic demand,
LowIR/MIR, ELF, executable optimization and self-hosting have no PA5 surface;
none is claimed here. **Remaining behavior groups: none.**

## Performance evidence and budgets

[Final evidence](performance.md): A `262b0b61f` vs B `b19de66e0`; frozen binaries,
flags and inputs; two A/A pairs, B/B and two ABBA blocks per input. Final campaign
retains **168 observations +8 startup probes**; the first indexed campaign retains
another 140 +8. Every compared output is identical. The verifier recomputes
protocol, actual final binary/input hashes, paired gains/spread, budgets and work.

Budgets fixed before optimization: ≤10% wall regression plus A/A noise,
≤15% RSS growth +1 MiB, ≤15% host text growth; 4x input/depth <6x wall / <5x RSS
+1 MiB. All pass. Nested input: 5.612879 → 0.135206 s at the larger size;
paired gains 97.59% in both blocks; RSS 14814 → 15174 KiB. Smaller nested input
improves 90.04–90.09%. Fourfold depth has 3.501x B wall growth; angle work
25520 → 102320 tracks tokens 25851 → 102651. Declaration/expression regressions
are disclosed (about 1–3%); procedural results are inconclusive. Host compiler
text 200070 → 203718 bytes (+1.82%). Generated runtime/text: **N/A**.

## Validation and ledger

- `e3953bf8f`: baseline and review markers recorded before implementation.
- `a0adc0d0b`: streaming graph/parser/driver foundation, **131/188**, core 10/10.
- `262b0b61f`: all scoped syntax behavior groups, **188/188**; frozen A baseline.
- `b19de66e0`: linear prediction indexes, shared source records, user-literal
  payloads and lexical-hint cache; final source, tests, audit and raw evidence.
- Final `make test-pa5`: **188/188**. Earlier PA1–4: **205/205**.
  Fresh `make test-report-through-pa5`: **393/393**, all five stages pass.
- File audit: **62 files pass**. Core 10/10, extended 15/15, and C++ API checks
  pass. ASan/UBSan with leak detection passes API, extended and all 188 unchanged
  course fixtures. The performance/hash verifier and whitespace checks pass.
- No fixture, reference, harness, timeout, discovery, comparator or coverage
  changes. New implementation sources are registered. Generated binaries,
  objects, AST outputs and logs stay outside committed changes.
- Handoff reason: **full-stage completion**; no incomplete behavior group,
  deferred related fix or pending required check. Intended changes are committed;
  final status is clean. Independent review markers intentionally remain intact.
