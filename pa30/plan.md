# PA30 implementation203 handoff

Stage base commit: `27029f978e65b78331233123922d342033d5d1f7`.
Last reviewed commit: `378d1bd83df4dd3f7a9c1693c18afa7e7c7bd61f`.
Target: **PA30 full-stage**. Phase: **implementation handoff complete; independent audit pending**.
Entry HEAD: `404e468e829c6b76642678b991a09067306c1efb`.
Implementation commit: `247c383a`.

## Design and spec alignment

The [previous audit](audit.md) covers implementations199–201. This increment
completes its remaining vector/packed-operation group in the shared compiler.
Canonical intrinsic declarations and immediate constraints feed typed LowIR;
ordinary native selection and ELF emission consume the same facts. Vector
lvalues retain their storage, while value conversions snapshot operands before
later effects. Volatile reads/stores remain explicit. No fixture names, header
shortcuts, host code generation or alternate hosted backend are involved.

[Design203](design203.md) records ownership, data flow, cache keys/lifetimes,
serialized target-operation fields and work limits. Fixed packed expansion is
bounded by 16 lanes; general vector operations unroll at most eight lanes, then
use a loop. Architectural state/float operations use a fixed 64-byte record.
Comparison fallback is bounded by 32 alternatives; known predicates emit one.
No optional optimization, global retry or mutable process cache was added.

## Validation and performance

[Required checks](../student.tests/pa30/evidence203/validation.json): earlier
PAs **4941/4941**, PA30 **153/153**, through30 **5094/5094**, file audit passes
with four inherited warnings. Both original random-header failures are fixed;
[identities and outcomes](../student.tests/pa30/evidence203/stage-delta.json)
show unchanged coverage. The external 154 count disagrees with the preserved
153-case entry inventory. No fixture, reference, timeout or comparison changed.

Personal controls: **391 inherited + 48 new commands**, including the old
`pending199/vector-subscript.cpp`; **286/286** hardware differential cases,
each with two runtime inputs; **95** LowIR/native trace and **42** ELF inspection
commands. Seven programs execute through direct objects and reparsed LowIR; stats do not change
objects. Malformed IDs, non-pointer records and invalid predicates reject.
Positive and required-negative controls cover vector cv/category, snapshots,
volatile casts, dependent operators, shuffles, integer boundaries, floating
rounding/status, memory operations and direct-only/immediate restrictions.

[Performance203](performance203.md) preserves **512 measurements + 16 launcher
calibrations**, frozen A/B images, A/A noise and six ABBA blocks. Final hosted
measurements peak at **5.978 s / 177304 KiB**. Common/header A/B objects match;
vector snapshots add 24N text bytes with approximately 1–2% runtime cost and
linear measured work. The 45-second compile limit remains mandatory. Historical
15% latency and zero-growth targets remain diagnostics under spec §9; all
historical records are retained. No speedup is
claimed, and a baseline that rejects a new case is not a timing comparison.
PA31 hosted runtime, PA32/33 optimization and PA34 self-hosting remain scoped to
their stages without waiving PA30 correctness, coverage or architecture.

[Verification](../student.tests/pa30/evidence203/verification.json) passes
**2994** bindings, outcomes, coverage and work-bound checks against the final code.

## Handoff ledger

| Owner / completed group | Validation and boundary |
|---|---|
| Vector expression, query and storage owners | Subscripts, unary/compound operators, constant subobjects, cv/category, snapshots, shuffles and volatile conversion/storage closed together. Focused and inherited controls pass. |
| Packed vocabulary, canonical signatures and lowering | Saturation, interleave, arithmetic, safe shifts and SSE/MXCSR operations implemented with real semantics. All 286 hardware comparisons pass. Serialized LowIR retains all required facts. |
| Full-stage closure | Required prior, PA30 and through30 reports pass; frozen correctness/performance evidence is complete. Full-stage independent audit is the next boundary. |

**Unfinished implementation:** no known PA30 defect remains in this behavior
group or the required suite. **Independent review:** Ralph must audit the full
stage, including the new typed target boundary and accumulated ownership/spec
alignment, before advancement. This handoff is implementation completion, not
that certification. The review marker intentionally remains unchanged.
