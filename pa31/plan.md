# PA31 final compact plan

Stage base commit: `c0566ded7ed123e1eb0696f0ae93b28ceb492fdf`.
Last reviewed commit: `6039c065709a32a26d74f4758e235150d621e0ae`.
Target: **PA31 full-stage**. Phase: **final audit complete**.
Implementation: `ffea90db`, `d5bd5aee`. Audit206 changes controls and records only.

## Final Spec Alignment

The compiler owns the cumulative path: immutable source buffers → streaming
preprocessor/token cursor → integrated source/semantic graph → canonical typed
facts → direct LowIR → bounded preparation and per-function MIR → direct ELF.
Templates retain parsed regions, compact identity, parent-linked environments
and separate monotonic demand states. Precise reverse dependencies invalidate
only affected facts. Lowering consumes selected declarations, conversions,
layouts, cleanup and ABI facts. TU/slab and function-local owners release their
storage at explicit boundaries. Hosted metadata uses this same path.

The [independent audit](audit.md) reconstructs the entire architecture, reviews
all five implementation changes and closes the previously unaudited handoff.
The combined hosted/template trace follows list construction and cleanup,
inherited defaults, allocation, callable templates and streams to ELF. A forced
inline fact is traced through legality, budget admission, invalidation and final
encoding. [Design205](design205.md) retains the detailed implementation ownership.
No additional compiler defect or needed implementation refactor was found.

## Completed groups and evidence

| Group | Final disposition |
|---|---|
| Allocation ownership | Separate external hosted allocation identities; multi-TU host replacement control passes. |
| Complete/base ABI entries | Virtual-base classes cannot alias distinct entry semantics; layout/VTT and symbol controls pass. |
| Braced temporary exceptions | Backing conversions own element evaluation/lifetimes without recursive self-entry; nested lists and partial cleanup pass. |
| Inherited construction | Zero-argument candidates, dependent constructor using syntax and other-subobject validity pass runtime and semantic probes. |
| Relocation expectations | Four spelling-only sidecar corrections have a [reducer, C++11/ABI proof and pinned bundle binding](reference-correction205.md); coverage and comparison rules preserved. |

## Performance acceptance

[Performance206](performance206.md) reviews all 544 inherited observations and
adds 280 fresh workload observations, with frozen whole-stage A/B, A/A and six
ABBA blocks for compiler/executable timing, peak RSS and both text sizes. All
common images are identical; stream text decreases 39 bytes. Noise and paired
spreads establish no runtime speedup or repeatable avoidable regression.
Newly accepted list/inheritance families retain proportional costs through N=1024.

No optional pass or growth policy is added. Preserve the 45-second hosted limit
and inline limits: 262,144 units/function, 4,194,304/Program, depth 64. Historical
15% latency/zero-growth targets remain diagnostics under spec §9, with all
measurements preserved. PA32/33 optimization and PA34 inception stay stage-scoped.

## Final validation and ledger

[Validation and bindings](../student.tests/pa31/evidence206/validation.json)
record the required file audit, PA31 suite and through-PA31 report, **46 inherited
control commands**, **104 trace commands**, source/image identities and independent
recomputation of performance evidence. The supplied 5,342-test status was stale;
the primary and fresh root logs report **5,178/5,178 and 31/31 stages**, including
**84/84 PA31 fixtures**. No tests were removed. The file audit passes with four
inherited substantial-header organization warnings and no errors.

Entry205: 78/84 → implementation205: 84/84. Audit206 independently reviews both
code commits and their surrounding cumulative owners, adds three controls and
consolidates the complete [handoff ledger](audit.md). No unaudited handoff or
known required PA31 work remains. No advancement to PA32 is performed.
