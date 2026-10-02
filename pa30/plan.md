# PA30 compact implementation plan — audit198

Stage base commit: `27029f978e65b78331233123922d342033d5d1f7`.
Last reviewed commit: `4a081cb05b25638be7a759882f67d4d8ae97eb6a`.
Target: **PA30 full-stage**. Phase: **implementation199 in progress**.
Implementation entry HEAD: `d9a625844ca9a182f74152abfb08e2e67b0057b9`.
Stage base and last reviewed markers above remain unchanged.
Previous goal turn: progress (audit198 completed); no live compiler/test job at entry.
Entry evidence: 132/153 (21 failures); Ralph cached 132/154 (22 failures).

Implementation199 begins with the eight dependent constant/access failures.
Owners: type-builder array bounds consume semantic constant facts; member access
consumes declaration ownership and lexical privilege. Trace those facts through
substitution, declaration completion and typed lowering; fix their shared owners.
Work must track demanded expressions and lexical/base edges, with no whole-program
retry or rendered semantic keys. Validate reduced positive/negative controls,
all affected hosted cases, explicit LowIR/object execution, scaling, equivalent
A/A+ABBA compile/runtime/RSS/text measurements, and course through reports.
Then extend related fixes while this understanding applies. Remaining callable
and emitted-code groups below remain implementation work, not waived audit items.
Independent review: implementation199 changes require review after handoff.
Reviewed all three accumulated checkpoints, `27029f9..c0b26910`, and the audit
repairs through the code tip above. This record adds no implementation changes.
Previous goal turn: progress (committed implementation197); no live job at entry.

## Reviewed and repaired

[Audit198](audit.md) records every commit, combined ownership paths, interactions,
architecture/optimization traces and the ledger. The parser's class/friend/angle
boundaries, injected construction types, current-instantiation qualifier scope,
substituted alias shapes and namespace canonical identity have been reviewed
as one accumulated implementation. No pending independent-review item remains.

- `88c25337`: extend canonical type identity to base typedef lookup. The old
  negative oracle contradicts C++11 [class.member.lookup]/3,6–7. The unchanged
  reducer, standard proof and bundle binding are in
  [reference-correction198](reference-correction198.md). Distinct types/entities,
  alias templates and inaccessible aliases remain rejected.
- `4a081cb0`: reserve implicit LowIR role spellings when displaying ordinary
  namespace/member functions. A combined checkpoint control exposed a valid
  namespace `main` receiving an entry role only after serialization. Typed
  roles, ABI names and the external LowIR comparison contract are preserved.

## Validation and performance

[Required checks](../student.tests/pa30/evidence198/validation.json): PA1–PA29
**4941/4941 pass**; file audit passes with four inherited header-division warnings;
PA30 **132/153**, the **same 21 failing cases** as entry; through-PA30
**5073/5094**. No earlier regression, new failure or reduced coverage.
The supplied 154 total disagrees with the authoritative primary log, fixture
inventory and isolated reports: all contain 153 cases. The standard-backed base
reference correction changes one expectation, preserving its input and coverage.
[Coverage delta](../student.tests/pa30/evidence198/stage-delta.json) binds every case.

[Controls](../student.tests/pa30/evidence198/controls.json): **107 commands pass**,
rechecking all checkpoint reducers plus combined and new boundaries through
objects and validated LowIR/native execution. [Trace](../student.tests/pa30/evidence198/trace.json):
**34 commands pass**, including the old adapter failure, current MIR/ELF/CFI,
telemetry invariance, access, dormant demand and token size.

[Performance198](performance198.md) records frozen stage-base/final and
checkpoint/final comparisons, A/A calibration, six ABBA blocks, all observations,
compiler latency/RSS, checked executable runtime and text size. The 45-second
limit and inherited optimization budgets are unchanged. Blanket 15%/zero-growth
targets remain diagnostic under spec §9, with all historical measurements kept.
No optional optimization or runtime-profit claim is introduced. PA31 hosted
execution, PA32/33 optimization levels and PA34 inception remain later-stage work.

## Remaining implementation groups

| Existing failures | Broad owner and required work |
|---:|---|
| 9 | Template/callable prerequisite scheduling and overload/construction facts: six callable/shared-pointer cases, bind-member call selection, and two piecewise tuple/pair constructor cases. |
| 8 | Dependent constant/access facts: six array-bound constant-evaluation cases and two random-library nested-member access-context cases. |
| 4 | Required emitted-code and declaration behavior: one target-vector builtin, one replaceable-new exception specification, cross-function local-reference rejection, and reachable missing-return rejection. |

The 27 accumulated fixes remain passing (105 → 114 → 129 → 132), with no
additional stage pass claimed for correcting an already-passing erroneous oracle.
Do not advance until the complete root through-PA30 report passes.

Earlier handoffs split related parser/semantic and identity work too narrowly;
in particular the base-lookup contradiction was unnecessarily deferred. Work
through the three broad groups above, including their parser-to-ELF and negative
controls, rather than handing off after individual header/fixture improvements.
Historical performance195–197 and their evidence remain unchanged. The audit's
source/binary bindings and reproducible verification (**1,218 checks pass**) are
in `evidence198/` and `student.tests/pa30/verify198.py`; the single audit ledger
row is in `audit.md`.

Implementation199 ledger (in progress): runtime new[] query extents are separate
from canonical element types; current-instantiation friendship is an indexed
source edge; fixed-base alias signatures use indexed member lookup; protected
call access through dependent bases remains a substituted query obligation.
Fixed MMX vector construction/extraction and equal-size representation casts use
recorded intrinsic/conversion facts and typed LowIR storage. No optional optimizer
was added. First full stage report: 137/153, no new PA30 failures. 67 explicit
object/LowIR controls pass. Prior report found one vector-to-void regression;
the corrected case passes, full earlier report will be repeated. Current regex
failure is parser class lookahead; random now reaches missing packed SIMD
arithmetic. These are unfinished implementation, not independent review waivers.
