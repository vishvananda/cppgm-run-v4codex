# PA30 compact implementation plan — implementation203

Stage base commit: `27029f978e65b78331233123922d342033d5d1f7`.
Last reviewed commit: `378d1bd83df4dd3f7a9c1693c18afa7e7c7bd61f`.
Target: **PA30 full-stage**. Phase: **implement; stage incomplete**.
Reviewed range: `4a081cb0..56ecc31c`, extended through audit fix `378d1bd8`.
All implementations199–201 and their interactions are covered by [audit202](audit.md).

## Reviewed implementation

Canonical array queries retain the runtime first extent separately from the
allocated element type. Their shared conversion owner now permits C++11's
unique non-explicit integral conversion; evaluated bounds record the selected
call and signedness, while unevaluated queries do not demand bodies.

Typed constant-join reachability repairs the new non-void checker without
changing emitted IR or earlier structural oracles. Its function-local worklist
tracks integer values and unescaped scalar slots, preserves exceptional edges,
and has at most two monotonic transitions per fact. Volatility, escape and
unknown inputs remain conservative. The new source is registered in the shared
frontend source set. The accumulated access, deferred-body, member-alias,
destruction, capture, vector and noreturn paths were reviewed together.

## Validation and performance

[Required reports](../student.tests/pa30/evidence202/validation.json): earlier
PAs **4941/4941**, file audit passes with four inherited warnings, PA30
**151/153**, through30 **5092/5094**. The exact same two failures remain;
coverage and comparison rules are unchanged. The supplied 154 count disagrees
with the preserved primary log and 153-case inventory. The previous replacement-
new [reference correction](reference-correction201.md) has a valid standard proof;
this audit makes no additional reference changes.

Explicit controls: **391** passing commands, **71** trace/inspection commands,
and **1506** [evidence checks](../student.tests/pa30/evidence202/verification.json).
All 27 historical traced objects match current rebuilds. The code is committed
before these records; `Last reviewed commit` names that validated code tip.

[Performance202](performance202.md) retains 576 observations and 32 launchers,
frozen A/B images, A/A calibration, six ABBA blocks, runtime/text, compiler
latency/RSS and all outliers. Common and conditional A/B images match exactly;
flow work scales as N functions, 119N instruction visits and 58N edge/operand
visits. Final hosted samples peak at **5.390 s / 177028 KiB**.
The **45-second compile limit** remains mandatory. Historical 15% latency and
zero-growth targets are diagnostic, not gates under spec §9; all measurements
remain. No optional optimization or speedup is claimed. PA31–34 requirements
remain stage-scoped without waiving PA30 correctness or architecture.

## Remaining implementation

| Group | Required work |
|---|---|
| SIMD and vector completion | Finish typed packed operations, saturation/lane behavior and subsequent random-header operations; both failing fixtures currently stop at `__builtin_ia32_packsswb`. Complete general vector subscripting/lvalue/storage lowering, including `pending199/vector-subscript.cpp`. No stubs, name-based library shortcuts or invented unused results. |
| Full-stage closure | Run all personal interaction controls and the required PA30 and root through30 reports after those fixes. Advance only when the full root through30 report passes. |

The three reviewed handoffs separated bound legality and conditional-join
interactions from their initial owner fixes, leaving avoidable follow-up work
for audit. Finish the remaining vector operations, conversions, volatility and
storage interactions as one broad group before the next handoff. This checkpoint
audit does not certify PA30 completion.

## Implementation203 active ledger

Entry HEAD: `404e468e829c6b76642678b991a09067306c1efb`. Previous turn:
progress (audit repairs, committed evidence); no inherited worker is live.
Entry required inventory: 151/153, two random-header failures (the external
154 summary is inconsistent with its own primary log).

| Owner / group | Data flow and work bounds | Validation / status |
|---|---|---|
| Packed SIMD vocabulary and lane lowering | Immutable builtin descriptions → canonical signatures and typed intrinsic facts → scalar typed LowIR lanes → ordinary native/ELF. Fixed-width work is bounded per builtin; no library-name dispatch or dummy results. | Implemented typed packed lanes and a bounded architectural LowIR operation for SSE/MXCSR. Focused boundary/state tests pass; frozen serialized and hardware controls running. |
| Vector expressions and storage | Shared operator/type-query facts → lane addresses, snapshots and stores. One operand evaluation; cv and categories preserved. O(lanes), generated loops for wide vectors. | Closed subscripting, snapshots, volatile conversion/storage and dependent assignment/query interactions. All 44 new positive/negative commands pass. |
| Closure and evidence | Freeze entry/final compilers, A/A and ABBA equivalent controls plus final-only new behavior. | Required prior/PA30/through30 and file audit; explicit inherited controls; latency/RSS and executable runtime/text. |

Independent review remains separate from these unfinished implementations; no
known vector defect is assigned to audit. Preserve both review markers above.

Implementation203 code checkpoint: shared vector/packed owners implemented; final
required reports and frozen performance evidence are running. See [design203](design203.md).
