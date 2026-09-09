# PA11 consolidated plan

Stage base: `a97e14d49c7edfc7acc115b974ab667cc90480db`.
Last independently reviewed implementation: `9c0a9c4e` (including `b262971d`
and `cd09b606`; the final naming cleanup preserves the frozen compiler bytes).
Target: **PA11 full-stage**. Phase: final audit; no PA12 work is advanced.

## Spec Alignment

The shared streaming frontend and sole source graph produce canonical typed
semantic facts and direct LowIR. The independent [audit](audit.md) reconstructs
all stage ownership and representative source-to-executable test flows, rather
than relying on the earlier checkpoint summaries.

| Owner | Final design and evidence |
| --- | --- |
| Source/parser | Immutable buffers, interned names and bounded deferred class lookahead. Grammar is parsed once. LowIR mode omits joined literal display copies; language linkage consumes decoded literals. Earlier AST/type/semantic outputs remain passing. |
| Selection | Indexed scope/base/friend/ADL edges select declarations, object adjustments and conversions. Constructor initializer ownership and contextual value-initialization access checks are explicit. |
| Layout/initializers | Canonical layout and field descriptors feed source/type action plans. Shared omitted-value shapes retain volatile qualification and union boundaries. Static bit packing preserves neighboring scalar bytes and required startup ordering. |
| Construction/lifetime | Required member/base ABI entries are demanded once. Default calls retain conversions and temporary identities; arrays destroy default-argument temporaries between elements. Namespace/TLS arrays use the ordinary bounded constructor path. Cleanup suffixes retain control context and saved returns. |
| Lowering/resources | Typed plans replace aggregate semantic reconstruction. Repetition expansion is bounded across dimensions, with exact bulk zero or scalar loops as appropriate. Temporary preliminary zero has one owner. TU and function scratch have explicit release points. |
| Static construction | A cached summary requires an effect-free body, supported field actions, preserved conversions and static values for every argument, including unused/default arguments. Failure retains dynamic evaluation. |

## Findings, changes and acceptance

`b262971d` fixes the initialization ownership, access, qualifier, static bit-field,
default-conversion/lifetime, global/TLS-array and argument-effect defects found
independently. `cd09b606` removes duplicate preliminary zero for temporary objects.
Eight executing reducers, two rejection cases and explicit range/dimension checks
supplement the unchanged course suite and earlier personal tests.

[Final performance evidence](final-audit-performance.md) retains the initial
candidate and corrected frozen A/B campaigns, A/A calibration, ABBA pairs,
follow-ups, hashes and all observations. It reports latency, RSS, compiler text,
executable runtime and native text together. The supported O0 work is bounded;
no optional optimizer or later-stage native gate is introduced. Historical
numeric targets remain diagnostics under spec.md's stage-scoped rule. Correctness,
coverage, comparison rules and the eight-element expansion budget remain intact.

The earlier seven [reference corrections](reference-corrections.md) retain their
reducers, C++11/LowIR proofs and pinned bundle revision. This audit changes no
reference, fixture input, comparison rule or test discovery.

## Validation and ledger

- Required file audit: **pass**, with the existing Analyzer-header advisory.
- `make test-report-through-pa11`: **1327/1327; 11/11 stages**, including
  **302/302 PA11** cases and four controls. The fresh count agrees with the
  retained primary log; the supplied 1351 count was not reproduced.
- `student.tests/pa11/check.py`: 17 native successes and ten rejection cases.
- `student.tests/pa11/audit_check.py`: all audit reducers and growth checks pass.
- The same audit suite passes on the registered ASan/UBSan compiler build.
- Frozen artifact/observation verification and native checks are recorded in
  the final evidence and [audit ledger](audit.md#validation-and-handoff-ledger).

All 21 stage commits from the base through `0773e4d8` were reviewed: member ABI,
construction/defaults, lifetime/noexcept, selection/access/operators, layout and
initializers, inherited/converting construction, TLS/ABI/literals and narrowing.
The audit closes the stale base review pointer and all intervening checkpoint
handoffs. The audit fixes, action-identity naming cleanup and retained measurement
continuations complete the ledger. No PA11 implementation work is deferred.
