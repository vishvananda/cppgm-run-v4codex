# PA24 implementation ledger

Stage base commit: bde9eb3e128e24923a1de40bb63b8e348a13553b
Last reviewed commit: bde9eb3e128e24923a1de40bb63b8e348a13553b

## Active handoff129

Entry HEAD: c88bfaebb06afebb6459595fb1455ab8e782fe4f. Previous turn made
verified implementation progress; no surviving build at entry. Baseline is
250/296 course cases (46 failures), Ralph inventory 250/435. Review markers stay.
Work group: multiword values and object ABI, extended through shared call/frame
placement and scratch effects. Unit owns complete numeric literal payloads;
Selector owns value fragments, argument classification, homes and liveness;
Encoder consumes explicit typed chunks. Register classification is bounded by
six GPR/eight XMM carriers; object copying is linear in produced chunks, with
bounded bulk-copy fallback. No name-based semantic recovery or textual phases.
Validate required stage/prior reports and file audit, explicit personal wide/ABI
tests, and frozen compiler latency/RSS plus executable runtime/text evidence.
Known unfinished runtime/TLS/EH and canonical MIR policies remain requirements.
First increment: typed 128-bit literals, fragment arithmetic/compare/shifts/atomics,
shared object/wide ABI classification and bounded stack-copy dependency snapshots.
Stage 282/296, prior 3856/3856, independent wide 1175, object 160, floating 1259.
Audit passes; work continues through wide numeric completeness and ABI pressure.
A converting parameter store is no longer incorrectly promoted as an identity.


## Design / completed execution groups

PA8 typed Unit -> function-owned placement and flat MIR -> shared MIR view/x86
encoder -> typed fixups/direct ELF. No host/reference compiler, assembler, text
transport or fixture recognition. Unit tables retain compact IDs; function facts
and MIR die after encoding. Actual frame/scratch/save facts drive the encoder.

The scalar foundation covers integer/pointer widths through 64 bits, loads/stores,
conversions, compare/branch/switch, parallel phi edges, direct/indirect calls,
stack/by-address arguments, hooks, globals, bulk memory and scalar atomics.
This handoff adds f32/f64 XMM and conservative f80/x87 arithmetic, signed-zero
negation, NaN-aware comparisons/truth, integer/float and float-width conversions,
mixed calls, call-crossing homes, variadic GPR/XMM saves and overflow arguments.
Implicit floating-width store/return conversion follows the required course
fixture. External decimal literals acquire per-format numeric payloads at the
reader boundary, eliminating double rounding; their spelling is a rendering view.

Placement reserves xmm14/xmm15 and GPR rax/r10/r11 for encoder effects. A dying
XMM lhs can share its destination; otherwise operands remain distinct. Complete
private scalar-home windows of at most 64 MIR instructions may replace stores
and reloads with reserved-register moves. Calls, floating/bulk/EH operations,
globals, large-immediate effects, escaping homes and carrier conflicts retain
the frame form. MIR records the resulting moves and telemetry counts the reloads.
When live parameters would occupy all five preserved GPRs, immutable parameter
homes plus incoming-carrier availability avoid unnecessary pre-call reloads.
Clobber bits propagate through joins/backedges; calls restore later reads from
homes, and derived addresses require carriers valid for their entire interval.

Owners/data flow: literal payloads belong to Unit; placement, home IDs, move
schedules and flow facts belong to Selector; Encoder consumes these typed facts.
Work is O(N + E log E): fixed linear walks, at most nine GPR/fourteen XMM probes,
at most fourteen ABI carrier moves, six monotonic clobber bits per CFG edge,
and three bounded 64-instruction carry probes per candidate. No global retries,
fixed-point body rescans, inlining, unrolling or text transport. O0 selection is
also the conservative policy for higher levels; later optimization remains due.

## Unfinished implementation (requirements retained)

- Wide values and object ABI: i128 literals/arithmetic/conversions/atomics;
  one/two-eightbyte and larger object parameters/results, padding and slot aliases.
  Owner: shared value representation and ABI classification. Needs explicit
  fragments, aggregate register rollback and coordinated caller/callee storage.
- Runtime/storage: TLS, required EH/runtime operations and dynamic-stack paths.
  Owner: runtime/layout, consuming typed LowIR facts; later host metadata remains
  outside PA24. No empty operation or reference fallback substitutes for this work.
- Six canonical MIR policies still fail despite correct execution: strict
  `100-object-abi-lowered`; structural `200-stack-arguments-beyond-six`,
  `500-mixed-gpr-xmm-call-abi`, `600-indirect-mixed-gpr-xmm-call-abi`,
  `700-call-setup-forwarding-no-preserve`, `800-single-edge-callee-saved-retention`.
  Owner: ABI/frame/result placement protocol, not the dump renderer. These are
  unfinished implementation, including the scalar cases; no shape waiver applies.

Handoff boundary: floating numerical execution, mixed/variadic scalar boundaries,
and bounded scalar pressure handling are coherent and validated. The work expanded
through literal input, shared validation, variadic overflow, phi/call lifetimes,
reload carrying and parameter flow. Further instruction-local extensions cannot
finish the remaining failures: multiword/object storage requires a new fragment
representation and the canonical cases require a coordinated placement/frame
protocol revision. Those are separate implementation groups, not small rendering
fixes. This handoff does not complete PA24 or advance to PA25.

## Validation / performance

- Entry HEAD: 9d8113ebe734f947d577e5ec6309dbbdd3fbbdbf; previous turn classified
  as verified implementation progress. No surviving build process at entry.
- `make test-pa24`: 250/296, versus 221/296 at entry. Failures fall 75 -> 46;
  29 original failures resolved, no new failures. All 17 focused native controls
  pass. All 255 compiled positive fixtures match runtime exit/stdout, including
  canonical-MIR failures. Forty feature cases and six MIR policies remain.
- `make test-report-through-pa23`: 3856/3856. File audit passes with three
  inherited warnings. Personal floating suite: 1259 programs; scalar suite: 1820
  cases; integration and parameter-flow tests pass. No fixture/reference edits,
  coverage cuts or comparison changes. The inventory of 435 includes 125 excluded
  design regressions and the original 14 controls; course denominator stays 296.
- `student.tests/pa24/performance.md` preserves handoff127 measurements.
  `performance128.md` records frozen current A/B data, A/A and six ABBA blocks,
  latency/RSS and runtime/text measurements. Final paired compiler ratios are
  0.971 integer / 0.917 floating (no speedup claim); max RSS changes by at most
  8 KiB. Floating runtime ratio is 0.458, text 2169 -> 1522 bytes. Pressure
  timing is inconclusive, with text 567 -> 557 bytes. Diagnostic budgets: <=15% compiler
  latency/RSS increase and no text growth for local selection. They are not extra
  exit gates; mandated course bounds remain binding. Existing loop/memory output
  is byte-identical to entry. XMM coalescing gives a repeatable runtime benefit.
  Source/template/self-host benchmarks belong to later native-driver stages.

## Handoff ledger / independent review

- Handoff127: `f9ff5dd4` scalar foundation 199/296; `7d1be282` bounded placement
  221/296; `9d8113eb` evidence. Review markers above remain unchanged.
- Handoff128: `2425715e` floating execution, typed literals and mixed/variadic ABI;
  `ae72c874` bounded pressure/parameter-flow; the final telemetry/evidence
  commit closes this implementation turn. No claims about unfinished groups are waived.
- Independent audit remains due: scratch-effect completeness; XMM alias and
  call lifetimes; NaN/rounding and numeric-payload fidelity; parameter clobber
  worklists; private-home carry legality; shared LowIR validation compatibility;
  MIR/encoder fidelity; function-state release and measured work bounds.
  These review questions are distinct from the unfinished implementation above.

Evidence directory: `/home/vishvananda/work/private/v4codex/artifacts/pa24-handoff128/`.
`stage-accepted.log`, `prior-final.log`, `file-audit-accepted.log`, `progress.json` and frozen
performance observations record this boundary. Independent review is not waived.
