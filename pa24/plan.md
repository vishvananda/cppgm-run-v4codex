# PA24 final audit134

Stage base commit: bde9eb3e128e24923a1de40bb63b8e348a13553b
Last reviewed commit: a81684807f1f2c6da2cd83d99fa3d08dfaaf770a
Entry: clean `63dc6c11`; previous goal turn made verified implementation progress.

Target: **PA24 full-stage**, O0 native backend. **Accepted for advancement**;
independent architecture, correctness and performance review is complete.
See [final audit](audit.md), [checkpoint130](audit130.md),
[validation134](../student.tests/pa24/validation134.json) and
[performance134](../student.tests/pa24/performance134.md).

## Final Spec Alignment

Immutable source buffers and streaming cursors feed one parsed graph with typed
semantic facts, canonical identities, parent-linked template environments and
demand queues. Typed `Program`/`FunctionBuilder` lowering feeds the PA24 explicit
LowIR adapter. The native tool validates external LowIR once, selects one
function's flat MIR, directly encodes x86 instructions, then patches typed
references into its own ELF image. No assembly, textual MIR or reference tool
transports or implements production phases. Source/native driver integration,
relocatable/hosted EH objects, later optimization levels and self-hosting remain
their handouts' later-stage work.

Whole-stage review covered the 20 entry-stage commits, 42 production/build files,
shared LowIR changes, phase consumers and representative source-to-ELF data flow.
The source trace checks aligned ordinary fields, a demanded template member,
mixed ABI and TLS; one template class completes, one member region is demanded,
and the unused invalid dependent member remains dormant.

## Finding and completed repair

Deferred conditional/switch phi transfers used the last selected source block's
parameter-clobber mask. A call could therefore replace an incoming argument
register before the edge incorrectly read it. `a8168480` records each source
block's exit mask and selects deferred transfers under their predecessor's fact.
This fixes the shared branch/switch ownership path without changing placement
policy: one extra compact word per block, one write per source block and one read
per deferred edge. No new analysis, global invalidation or iterative transform.

The 61 new controls cover all six GPR argument positions, three integer widths,
direct/conditional/switch edges, TLS payloads and cross-function unwind.
Frozen entry fails 36 valid cases; final code passes all 61. Initial malformed
personal-generator variants remain in scratch evidence and were corrected.
Required fixtures, references, bundle and comparison rules remain unchanged.

## Validation and performance

- `make test-pa24`: **296/296**, plus **17/17** focused properties.
- `make test-report-through-pa24`: **4152/4152**, **24/24** stages, plus
  **39/39** separately reported focused properties. The supplied 4315 summary
  is not the count in either the primary log or the fresh root report.
- Required file audit: pass, zero fatal findings, four inherited header warnings.
- Personal suites pass: 1820 scalar, 1259 floating, 1495 wide, 160 object,
  75 EH/frame, 236 TLS, 205 placement, 21 checkpoint and 61 final-audit cases;
  ABI/debug/ELF and TLS integration also pass.
- New source trace: two successful runtime inputs, three native functions,
  **389 text bytes**, separate RX/RW segments and dump/native byte identity.
  Native compilation with an empty PATH produces the identical image.
- Frozen entry/final measurements use A/A calibration, six ABBA blocks, fixed
  flags/inputs and checked outputs. Final numerical evidence is consolidated
  in performance134; all 37 inherited manifests match.
- Native compiler paired medians span **0.983–1.026**, with effectively unchanged
  RSS. All ten runtime images are byte-identical to entry; no new speedup is
  claimed. The fixed 9600-specialization source also executes through our native
  backend (806082 text bytes); frozen frontend binaries and outputs are identical.
- The inherited 15% compiler latency/RSS and zero optional text-growth targets
  are diagnostic. Required correctness, MIR envelopes/comparisons and finite
  work/growth bounds remain mandatory; no historical observations were removed.

## Handoff ledger

| Group | Reviewed code | Final disposition |
|---|---|---|
| 127–130 scalar/FP/wide/object/phi/atomic | through `9f3f9b5c` | Independently reconstructed again; earlier controls preserved |
| 131 EH/frame | `45774fa2`, `c7587f72` | Stack floor, handler chain, saved frame and alignment consumers reviewed; accepted |
| 132 TLS | `db965a86`, `03eeff9e` | Layout, wrapper demand, address consumers, FS startup and fixups reviewed; accepted |
| 133 placement/frame | `1d73b01e`, `e5b5dada` | ABI dependencies, Boolean/debug facts and canonical costs reviewed; deferred-edge defect repaired |
| 134 whole-stage | `a8168480` | Accepted: architecture, correctness, performance and required exit checks complete |

No implementation or review handoff remains unexamined. Evidence is retained in
`/home/vishvananda/work/private/v4codex/artifacts/pa24-134/`.
