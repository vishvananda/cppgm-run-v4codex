# PA30 compact implementation plan — implementation199

Stage base commit: `27029f978e65b78331233123922d342033d5d1f7`.
Last reviewed commit: `4a081cb05b25638be7a759882f67d4d8ae97eb6a`.
Target: **PA30 full-stage**. Phase: **implementation handoff; stage incomplete**.
Implementation entry HEAD: `d9a625844ca9a182f74152abfb08e2e67b0057b9`.
Code tip: `d6408429`. Both review markers are preserved.

## Design/spec alignment

[Design199](design199.md) records semantic owners, source-to-ELF data flow,
complexity, legality and validation. Runtime allocation extents are separate
from canonical element types. Current-instantiation friends use indexed source
edges; dependent protected access remains a substituted query obligation.
Fixed-base aliases normalize through member lookup. Complete-class lookahead
preserves member-template boundaries. Fixed vector builtins, representation
casts, volatile reads and zero static storage lower from typed facts with bounded
lane work.
No textual production bridge, global retry, host compilation or optional
optimizer was added. New implementation sources were not needed.

## Validation and performance

[Required reports](../student.tests/pa30/evidence199/validation.json): earlier
PAs **4941/4941**, file audit passes (four inherited warnings), PA30 **138/153**,
through30 **5079/5094**. [Delta](../student.tests/pa30/evidence199/stage-delta.json):
**six existing failures fixed; 21 → 15**, no new failure or coverage reduction.
Audit198 already reconciled Ralph's cached 132/154 to authoritative 132/153.
All inputs, references and comparison rules are unchanged in this turn.
Explicit controls: **88** current + **107** inherited + **93** trace commands pass.
[Verifier](../student.tests/pa30/verify199.py): **426** evidence checks pass.

[Performance199](performance199.md) binds frozen entry/final binaries, A/A and
six ABBA blocks, all observations, compile latency/RSS, checked runtime/text,
corrected-owner scaling and repaired hosted costs. The **45-second** compile
limit remains mandatory. Historical blanket 15%/zero-growth diagnostics remain
non-gates under spec §9 with measurements preserved in performance195–198.
No optional optimization benefit is claimed. PA31 hosted execution, PA32/33
optimization levels and PA34 inception remain later-stage work.

## Remaining implementation groups

| Required failures | Owner and next work |
|---:|---|
| 6 | Template prerequisite scheduling: std::function recursion/captures/typeid/nullary base, shared-pointer allocator shadowing, nested callable pack result_of. Track demanded fact/dependency edges rather than global retries. |
| 4 | Call/constructor selection: bind member-template invocation, map/piecewise pair/index-sequence constructors, regex compiler construction. Record candidate substitutions and conversion failures. |
| 2 | Packed SIMD arithmetic: random now reaches missing `__builtin_ia32_packsswb`; implement required typed operations and saturation/lane semantics, not declarations without behavior. |
| 3 | Declaration/control-flow: replaceable-new exception-spec redeclaration, local callable cross-function reference rejection, reachable missing-return rejection. |

Also unfinished: valid general vector subscripting rejects in both entry and
current (`pending199/`, evidence199/pending.json). No known defect is relabeled
as an audit question. Do not advance until the full root through30 report passes.

## Handoff ledger

| Work | State and evidence |
|---|---|
| Accumulated checkpoints195–197 | Reviewed and repaired by [audit198](audit.md), including canonical base-alias reference proof and LowIR role preservation. Historical evidence unchanged. |
| `a7a6e3a8`, `6b5645c0`, `c8526635`, `d6408429` | Related allocation/access/signature, vector and member-template boundary repairs complete; downstream failures traced into the groups above. Final reports and explicit object/LowIR controls pass for this behavior group. |
| Incomplete handoff boundary | Expanded beyond initial constant/access failures through parser and vector emission. Remaining callable scheduling, constructor selection, saturating SIMD and flow/declaration rules have distinct owners/proofs; further fixes cannot follow from relaxing these completed facts. See design199. |
| Independent review | Implementation199 delta awaits Ralph's review schedule. This is separate from the explicitly unfinished implementation above; neither is waived. |

Handoff goal is implementation progress, not whole-stage certification.
