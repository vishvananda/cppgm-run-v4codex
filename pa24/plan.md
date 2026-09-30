# PA24 checkpoint plan

Stage base commit: bde9eb3e128e24923a1de40bb63b8e348a13553b
Last reviewed commit: 9f3f9b5caaec9677af0e8431b51cacebcf823dce

Implementation131 entered at `8c10ee78`; code commits are `45774fa2` and
`c7587f72`. Audit130 made concrete correctness progress and remains the last
independent review. These markers are preserved; PA24 must not advance to PA25.
[Audit130](audit.md), [validation131](../student.tests/pa24/validation131.json),
and [performance131](../student.tests/pa24/performance131.md) retain evidence.

## Design and completed behavior

Typed PA8 Program -> function-owned placement/flat MIR -> direct x86 encoding
and typed image fixups -> ELF. Text is confined to explicit LowIR/MIR adapters.
Unit-owned compact indexes survive across functions; selection and MIR release
after each function. No reference, host compiler or assembler implements output.

Inherited groups cover scalar/wide integer, pointer and f32/f64/f80 execution,
parallel phis, atomics, globals, bulk memory, direct/indirect calls, variadics and
shared object ABI classification. Audit130 repaired fixed-register effects,
implicit widening, indexed stores and unsafe reload carrying. Prior evidence
and measurements remain in the audit and performance127–130 records.

131 completes **generic EH and stack lifetime**, including related alignment
and call-boundary behavior:

- Selector records exceptional targets, parameter unavailability and stable
  value homes. Handler edges never qualify as adjacent ordinary retention edges.
- Flat MIR owns push/pop/throw/resume/stack-allocation operations and explicit
  exception-base, allocation-floor and aligned-frame facts. Debug locations
  survive selection; reserved runtime storage is visible in the dump.
- Encoder installs 80-byte dynamic handler records and restores the target
  stack, frame and preserved GPRs. Cleanup and catch pop before entry; resume
  continues the same payload. Normal pop and early return remove stale handlers.
- The image owns generic exception-top/payload storage. Payload transport covers
  every scalar width through i128 and f80. This standalone generic ABI is separate
  from later host exception metadata and private runtime calls.
- Dynamic storage lives until function return, including after catch or normal
  pop. Only functions combining handlers and allocation need a floor home.
- An aligned frame base preserves incoming ABI offsets. Caller and callee share
  one stack-argument origin; over-aligned outgoing arguments align the call stack
  and restore its prior value. Floating scratch follows the selected frame base.

Work remains O(N + E log E): linear fact/selection walks, fixed register pools,
monotonic six-bit parameter flow, bounded reload probes and constant native work
per handler operation. Alignment padding is bounded by alignment-minus-one.
No optional optimization, global retry, IR text transport or broad invalidation
was added. Higher optimization levels retain the current O0 policy.

## Validation and performance

- Final `make test-pa24`: **287/296**, up from **282/296**; failures **14 -> 9**,
  with no new failing fixture. All five formerly blocked EH cases now pass.
  Focused properties remain **17/17**. Required fixtures/comparators are unchanged.
- The 435-file inventory includes 125 excluded solution regressions and 14
  control inputs; it is not the course denominator. On that inventory's counting
  convention progress is 282/435 -> 287/435 (153 -> 148).
- Final prior-through: **3856/3856**, **23/23** stages. File audit passes with four
  inherited header warnings. Personal scalar 1820, floating 1259, wide 1495,
  objects 160, ABI/debug/ELF integration and 21 audit controls pass.
- New final runtime/frame suite: **75/75**. Aligned class-template source trace,
  two argument counts, reserved-state/debug inspection, disassembly and six
  MIR/native byte-identity controls pass. A test evidence-path collision was
  corrected and the complete personal suite rerun; its failed invocation remains
  recorded separately and is not counted as passing validation.
- Frozen A/A + six ABBA blocks measure all four performance dimensions. Final
  paired compiler ratios are **0.971 / 1.076 / 0.988** (scalar/float/wide), with
  unchanged peak RSS. Broad host noise prevents a speedup or regression claim.
  Six inherited executable images are byte-identical. The new mixed exception
  workload checks eight million calls in about **0.166 s**, with **726** text bytes.
  Full spreads, noise calibration, hashes and observations are preserved.
- Inherited 15% compiler latency/RSS and zero optional text-growth targets remain
  diagnostics; mandated correctness, MIR limits and finite work budgets bind.
  Source/native driver integration, host EH and self-hosting belong to later stages.

## Remaining implementation and handoff boundary

- **TLS storage and wrappers (3 failures):** definitions, declared/defined wrapper
  binding and pressure-safe access. This needs a thread-storage/image/startup ABI,
  distinct from completed function-local handler and stack restoration.
- **Canonical placement/frame protocol (6 failures):** strict object-ABI frame;
  stack arguments; mixed direct/indirect calls; setup forwarding; single-edge
  retention. Programs execute correctly but mandatory MIR shapes still fail.
  Closing these requires coordinated placement/setup policy, not further changes
  to generic EH semantics. Exact comparisons remain required.

This boundary completes the shared EH/frame group and its alignment consumers.
Further related fixes within that owner are not known; proceeding now would
start either independent TLS image/runtime ownership or canonical placement
policy and its performance validation. These are unfinished implementation, not
review questions or waived requirements. Whole-stage completion is not claimed.

Independent review remains pending for the accumulated 131 code: verify handler
nesting/exceptional value ownership, restored allocation lifetimes and aligned
ABI/frame facts against the handout/spec, including interactions with inherited
wide/FP/call behavior and the recorded work/performance bounds. Implementation
validation does not replace that review and does not move Last reviewed commit.

| Handoff | Code / evidence | Disposition |
|---|---|---|
| Audit130 | through `9f3f9b5c`; audit.md | Independently reviewed; 14 failures remained |
| 131 | `45774fa2`, `c7587f72`; validation131/performance131 | EH/frame group complete; 9 failures; independent review pending |

Evidence directory: `/home/vishvananda/work/private/v4codex/artifacts/pa24-131/`.
