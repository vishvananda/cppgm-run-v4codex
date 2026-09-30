# PA24 implementation ledger

Stage base commit: bde9eb3e128e24923a1de40bb63b8e348a13553b
Last reviewed commit: bde9eb3e128e24923a1de40bb63b8e348a13553b

## Design / completed behavior groups

PA8 typed Unit -> function-owned placement/flat MIR -> shared MIR view and x86
encoder -> typed fixups/direct ELF. No host/reference compiler, assembler, text
transport, fixture recognition or name-based semantic recovery. Function facts,
move schedules and MIR die after encoding; Unit tables retain compact identities.

Handoffs127/128 established scalar integer/pointer and f32/f64/f80 execution,
parallel phis, call/variadic boundaries, atomics, hooks, globals and bulk memory.
Handoff129 adds complete 128-bit literal payloads without growing Operand,
two-word integer arithmetic/division/shifts/comparisons, scalar/FP conversions,
truth/switch/phis/variadic overflow and cmpxchg16b-based atomic operations. Direct
compare-fed branches decide signed/unsigned high words and unsigned low words;
value comparisons have a frame fallback. Float conversion retains rounding bits,
avoiding an intermediate rounding error; integer decimal parsing checks overflow.
A converting parameter store no longer aliases an incompatible incoming value.

One ABI cursor classifies both caller and callee: one/two-eightbyte objects and
integers, whole-argument GPR rollback, stack arguments and indirect large-object
returns. Addressable/padded homes preserve partial tails and distinct parameter
identities. Direct results load/store their ABI chunks from frame storage.
Pending scalar/address dependencies are captured before a large outgoing object
copy can clobber copy registers. Narrow stack arguments acquire the boundary's
width. Object loads and stores participate in the same effect/parameter flow.

Owners/data flow: Unit owns numeric payloads; Selector owns fragment locations,
ABI cursors, homes, live intervals and fixed effects; Encoder consumes typed
chunks and conversion operations. Allocation probes at most nine GPR/fourteen
XMM choices; call register schedules contain at most fourteen carriers. Work is
O(N + E log E), including fixed linear walks and existing six-bit clobber worklists.
Wide division emits constant code with 128 target iterations; float-to-wide uses
four 32-bit digits. Literal work is linear in input digits. No global retries,
body fixed-point rescans or semantic searches. Existing carry windows remain
bounded to three 64-instruction probes; new complex effects keep frame traffic.
O0 is also the conservative policy for higher levels; optimization remains later.

## Unfinished implementation (requirements retained)

- Runtime/layout: TLS definitions and wrapper binding; generic EH regions,
  cleanup/resume and required runtime operations; dynamic/overaligned stack paths.
  The eight current execution failures stop at TLS or `eh_try`. An empty operation
  or reference fallback cannot substitute for these features. Later host exception
  metadata and object linking remain outside PA24.
- Scalar placement/frame protocol: strict `100-object-abi-lowered`; structural
  `200-stack-arguments-beyond-six`, `500-mixed-gpr-xmm-call-abi`,
  `600-indirect-mixed-gpr-xmm-call-abi`, `700-call-setup-forwarding-no-preserve`,
  `800-single-edge-callee-saved-retention`. Their programs execute correctly but
  their canonical MIR policies still fail. No shape waiver applies.

Handoff boundary: wide numeric execution and object/integer ABI transport are
complete for this behavior group, including partial tails, register rollback,
fixed effects and shared validation. Work extended beyond initial failures through
all integer operations, FP rounding, variadics, implicit widths and control-flow
transfers. Further related instruction-local changes cannot finish the remaining
failures: TLS/EH need runtime storage and region/unwind state; the six canonical
cases require a coordinated scalar placement/frame protocol revision, with
performance revalidation of already-correct scalar paths. These are unfinished
implementation, distinct from independent review below. PA24 remains incomplete.

## Validation / performance

- Entry HEAD: c88bfaebb06afebb6459595fb1455ab8e782fe4f. Previous turn was verified
  implementation progress; no surviving build at entry.
- `make test-pa24`: 282/296 versus 250/296. Original failures fall 46 -> 14;
  32 resolved, no new failures. All 17 focused controls pass. All 287 compiled
  positive fixtures match runtime exit/stdout, including canonical-MIR failures.
  Ralph's inventory of 435 includes excluded design regressions; coverage and
  comparison rules are unchanged. No course fixtures or references were edited.
- `make test-report-through-pa23`: 3856/3856. File audit passes (four warnings,
  including the enlarged LowIR model header; no fatal findings). Personal suites:
  wide 1495, objects 160, scalar 1820, floating 1259; ABI/debug/ELF integration
  passes. `readelf` and raw `objdump` confirm separate RX/RW segments and actual
  cmpxchg16b, carry/borrow, scaling and conversion instruction encodings.
- `student.tests/pa24/performance129.md` records frozen final evidence. Existing
  handoff127/128 observations remain intact. Compiler wall/RSS and executable
  runtime/text are measured separately with A/A and six ABBA blocks, fixed inputs
  and flags. The entry compiler cannot execute wide values; new-wide measurements
  compare the correct final binary with itself and make no speedup claim.
  Final paired compiler ratios are 0.996 integer / 0.997 floating; peak RSS does
  not increase. Four inherited runtime images are byte-identical. New-wide
  baselines measure 308-byte call/phi and 2211-byte numeric workloads.
  Diagnostic targets are <=15% compiler latency/RSS growth and no optional text
  growth, not additional exit gates. Course bounds remain binding. Necessary
  wide semantic costs do not excuse avoidable regressions. Source/template-native
  and self-hosting benchmarks remain owned by later driver stages.

## Handoff ledger / independent review

- Handoff127: `f9ff5dd4` scalar foundation; `7d1be282` bounded placement;
  `9d8113eb` evidence. Handoff128: `2425715e` floating/variadic execution;
  `ae72c874` pressure/parameter flow; `c88bfaeb` evidence. Review markers unchanged.
- Handoff129: `7e9d4569` numeric payloads, fragments and object ABI;
  `7def93ba` complete wide numeric lowering and extended boundaries; final evidence
  commit records the validated boundary. This returns implementation control to
  Ralph and does not certify the assignment or advance to PA25.
- Independent audit still due: scratch-effect completeness, XMM/call lifetimes,
  numeric payload/NaN/rounding fidelity, parameter worklists and carry legality,
  shared LowIR validation compatibility, MIR/encoder fidelity and state release.
  Added questions: ABI rollback and padded homes, large-copy dependency capture,
  two-word carry/borrow/division, conversion rounding/control-word restoration,
  atomic alignment/fixed-register preservation and value/edge identity.
  These questions neither replace nor waive unfinished implementation.

Evidence: `/home/vishvananda/work/private/v4codex/artifacts/pa24-handoff129/`.
`progress.json`, stage/prior logs, explicit personal runs, disassemblies and frozen
performance observations preserve this boundary. Independent review is not waived.
