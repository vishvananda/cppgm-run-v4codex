# PA20 compact plan — final audit 101

Stage base: `a9b24ab68f1a75288df10161cb171fa239e1409a`.
Audit entry: `8c86c298`. Last reviewed implementation: `806b38fb`.
Target: **PA20 full-stage**. Phase: **final audit complete**.
Previous handoff: **progress**, confirmed by commits and the primary log;
this audit adds a verified correctness repair, controls, traces and measurements.

## Final Spec Alignment

**Pass for PA20/O0.** [Independent source review](audit.md) reconstructs the
whole stage, including all work since checkpoint 97. Streaming immutable-source
input feeds one source graph with canonical semantic facts. Compact occurrence
contexts reuse parsed template bodies and fixed facts; complete typed keys,
monotonic states and targeted queues govern demand. Lowering consumes selected
operations, capture IDs, initialization/lifetime plans, layout and ABI facts
directly into typed LowIR. TU and function scratch have explicit release points.
Only test tools invoke the supplied native backend.

| Owner | Final design / disposition |
|---|---|
| Deduction and initialization | Canonical cv/pointer/reference deduction, body-owned return deduction, complete array extents and shared query/evaluated conversion facts. |
| Ranges and captures | Typed implicit operations and lifetime stacks; indexed reference/this capture environments. Audit repair binds captured range objects through their checked source expression, preserving original storage across arrays, member/ADL calls and specialization. |
| Aggregate transport / result ABI | Completed immutable independence proofs, conservative ordered fallback, shared helper slots/keys, distinct parameter/result convention facts. |
| Retained grammar | Scope-correct category selection before publication, original operands/precedence retained, published-node guards, live-cursor negative angle cache; no grammar replay or copied tree. |

The range shortcut defect affected 22 of 29 new controls at entry. All now pass.
No PA20 defect is deferred. Student native optimization/allocation/debug/ELF and
self-hosting remain their owning PA24–34 stages; advanced unsupported language
features remain exactly those excluded by the PA20 handout.

## Evidence and exit criteria

[Current validation](../student.tests/pa20/audit101-validation.json): PA20
**144/144**; `make test-report-through-pa20` **3596/3596**, **20/20 stages**, exit 0;
required file audit **pass**, exit 0, same three advisory header warnings.
The raw entry primary log has the same 3596 count; no coverage was reduced.
All separately printed course controls also pass.

Personal controls **373/373**; four ABI checks; six inherited traces plus
[the current captured-range trace](../student.tests/pa20/audit101-trace.json).
The latter records complete LowIR/native disassembly and compiler syscalls,
checks two specialization bodies across three calls and verifies telemetry
does not affect output. All reference reducers/reconstructions/executions pass:
14 documented stage revisions, 639 fixture/contract/harness files unchanged
since audit entry, no new reference correction.

[Final performance](final-audit-performance.md): frozen stage-base/final and
audit-entry/final binaries, fixed old/new workloads, A/A calibration and four
ABBA blocks, compiler latency/RSS and checked native runtime/payload reported
together. Required helper costs remain disclosed; repeated pointer-entry benefit
is supported by runtime evidence. Incorrect baselines are excluded from A/B.
**Stage-scoped acceptance passes**. Inherited +15%, +16 MiB and 5.5× targets are
diagnostics under spec §9, as already classified by PA18/19; historical
observations, correctness, coverage and mandated work/growth limits are preserved.

## Final ledger

| Boundary | Reviewed outcome |
|---|---|
| 94–97, through `882cf523` | Architecture independently reconstructed; original review archived in [audit97.md](audit97.md); inherited controls/traces rerun. |
| 98, through `a1faea7a` | Capture environments, packs, conversions and oracle proofs reviewed; composition repair completed here. |
| 99, through `37f8300b` | Helper safety/cache/sharing, argument/result ABI and member-copy proof reviewed. |
| 100, through `7925464d` | Grammar publication, lexical selection, operand reassociation and work bounds reviewed. |
| 101, `806b38fb` | Captured range repair and complete validation committed; final measurements and consolidated records follow without production changes. |

Outstanding PA20 implementation, independent review or unaudited handoff: **none**.
All intended code, controls, evidence and records are committed before closure;
the closing repository check must report an empty `git status --short`.
