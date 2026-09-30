# PA22 compact plan — final audit 120

Target: **PA22 full-stage**. Phase: **final independent audit complete**.
Final Spec Alignment: **aligned at PA22/O0**.
Stage base: `a8482d768bd2dcede42ea63ef39e39cf3245c380`.
Entry: `17bc7a06`, clean. Last reviewed implementation: `2b1a02c9`.
Previous goal turn: progress (completed handoff 119). This audit reconstructs the
whole stage from source and closes both unaudited handoffs; no PA23 work began.

## Completed design and findings

- The shared streaming frontend, canonical typed facts, retained template source
  regions and precise demand queues feed direct typed LowIR. The independent
  [architecture review](audit.md) traces nontrivial multi-base lifetimes and a
  demanded member-pointer template through the supplied validation backend to
  execution. No text transport, semantic reconstruction or external compiler
  delegation implements student output.
- Member values preserve canonical targets and signed displacement. Local value
  and storage proofs remain conservative across writes, aliases, effects,
  overloaded arrows and lifetime/control boundaries. Constant scalar receiver
  elision consumes the selected conversion's completed result/lifetime facts.
  Parameter values retain generic adjustment and target-word truth.
- **Audit fix: empty-subobject identity.** Repeated empty bases/members/arrays
  formerly overlapped same-type objects. The layout owner now records bounded
  canonical empty-type summaries and reserves separate storage on overlap or
  unknown. Constexpr addresses, static member constants, runtime projection and
  constructor/destructor receivers consume the corrected completed layout.
- **Audit fix: access paths.** Later-base public using/protected access and
  alternative public paths formerly encountered first-base-only checks. Access
  now traverses relevant base edges with visited sets and canonical completed
  path/miss facts. Implicit-object ranking follows the related selected path;
  private, protected-object and ambiguous cases retain required rejection.
- One [new reference correction](reference-corrections120.md) changes only the
  broken layout's size/receiver metadata/offset. Its reducer and C++11/contract
  proof reproduce the supplied reference defect. All four [119 corrections](reference-corrections119.md)
  were independently reviewed and rerun. No source fixture, exit status,
  comparator, timeout or coverage changed.

## Budgets and performance

Empty summaries consume/retain at most **64 type identities** per direct edge/
completed class; overflow uses disjoint storage and never expands array elements.
They release with the TU. Access scratch releases after each query; completed
path facts retain their existing owner and validity. No global invalidation or
broader body/layout demand is added.

Inherited bounds remain **64 nodes** per member-value proof, **4096 visits and
64 depth** per requested function flow, **8 wrappers** per result/receiver proof,
and **8 initializer elements** before loop fallback. Proofs have zero generated
code-growth budget. No fixed-point optimization or body cloning is introduced.

[Final performance](performance120.md): frozen entry/final binaries, A/A noise
calibration and four ABBA blocks per workload/phase cover **14** fixed inputs.
All preserve identical LowIR and executable `.text` bytes. Compiler text grows
**5120 bytes**; required empty-layout/access work has small measured median
costs, with all outliers and the largest **2200 KiB** RSS increase disclosed.
No new speed claim follows from noisy runtime differences or IR counts.
Historical 114–119 measurements, live-loop profitability, native work inspection
and the 119 placement-dependent slowdown remain preserved and reviewed.

Spec §9's stage-scoped rules govern acceptance. Historical +15%, +16 MiB and
5.5× self-selected targets remain diagnostics, not gates. Mandated work/growth
limits, correctness, comparisons, coverage and default timeouts are unchanged.
Student native optimization/debug and full self-host benchmarks belong to later
stages; their absence does not add a PA22 exit criterion.

## Validation and ledger

[Sealed evidence](../student.tests/pa22/audit120-validation.json): fresh root report
**3811/3811**, **22/22 stages**, plus passing focused property controls;
`make test-pa22` **99/99**; required file audit passes with three inherited
header-organization warnings. **95** accepted LowIR outputs roundtrip stably and
**four** required rejections remain. New audit controls improve **7/26→26/26**;
all **126/126** inherited personal controls and five reference reducers pass.
The supplied primary log also reports 3811; no contract coverage was removed to
reconcile the separately supplied 3835 status summary.

| Range | Independent review disposition |
|---|---|
| 114–117, stage base through `e90fa3fa` | Reconstructed/rechecked with the full stage; original [checkpoint](audit117.md) and its historical failures preserved |
| 118, `f18dfb62..e10bdd7f` | Storage facts, repeated/equivalent paths and arrow effects reviewed; controls rerun |
| 119, `247c7de4..321c93db`, sealed at `17bc7a06` | Receiver facts, lifetimes, wide truth and four oracle corrections reviewed; controls/reducers rerun |
| 120, `2b1a02c9` | Both whole-stage ownership defects repaired and validated; performance, architecture, reference proof and final evidence consolidated |

No unaudited handoff or known unresolved PA22 defect remains. Final delivery
commits the audit/evidence and verifies an empty `git status --short`.
The explicit next-stage boundary is PA23 virtual inheritance, polymorphic
multiple inheritance and the broader RTTI ABI.
