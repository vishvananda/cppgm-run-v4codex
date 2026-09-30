# PA24 checkpoint plan

Stage base commit: bde9eb3e128e24923a1de40bb63b8e348a13553b
Last reviewed commit: 9f3f9b5caaec9677af0e8431b51cacebcf823dce

Implementation132 entered at clean `bcf1e3b5`; code is `db965a86` and `03eeff9e`.
The preceding turn made validated EH/frame progress; no live work was abandoned.
Audit130 remains the last independent review. Preserve both markers and do not
advance to PA25. [Audit130](audit.md), [validation132](../student.tests/pa24/validation132.json)
and [performance132](../student.tests/pa24/performance132.md) retain evidence.

## Design/spec alignment and completed groups

Typed PA8 Program -> function-owned placement/flat MIR -> direct x86 encoding
and typed image fixups -> ELF. Text is confined to explicit LowIR/MIR adapters.
Unit-owned compact indexes survive across functions; selection and MIR release
after each function. No reference, host compiler or assembler implements output.

Inherited groups cover scalar/wide integer, pointers, f32/f64/f80, parallel phis,
atomics, bulk memory, calls, variadics and object ABI. Audit130 fixed scratch,
implicit-width and parameter ownership defects. Handoff131 completed generic EH,
dynamic-stack lifetime and aligned frames/calls; its evidence and review questions
remain in validation131/performance131 and the ledger below.

132 completes **TLS storage, wrappers and address consumers**:

| Owner | Data flow / correctness | Work and lifetime |
|---|---|---|
| LowIR unit and native Workspace | Existing typed `storage` / `tls_for` identities -> dense wrapper lookup; no rendered-name decisions | One unit classification; O(1) lookup; unit release |
| Selector | Raw and derived symbol addresses -> `tls_addr`; loads, stores, calls, phis, pointer arguments and bulk memory consume actual addresses | Constant selection work per access, function-owned MIR; only selected carrier changes |
| Image and encoder | Dense target map -> initial-thread storage, FS:0 addressing, signed thread-offset fixups; declared accessor thunks; defined wrappers retain bodies | Linear layout/fixups/demand; each demanded thunk emitted once; image-owned storage/maps |
| Native startup | Initializes the thread pointer before init/main; checks setup failure; preserves initial argc/argv stack | One bounded startup sequence, eight-byte self pointer, bounded alignment padding |

Declared accessors use the shared typed Function/MIR path and appear in dumps
when demanded; their frame/return facts reach the ordinary encoder. This removes
a separate direct-thunk emitter without changing executable bytes.

The group extends beyond the three failing fixtures: deferred constant indexes
now resolve as TLS, and calls through TLS function-pointer tables load the correct
storage. Atomic, floating/wide, bulk, pressure, CFG and ABI consumers are checked.
The `thread_local` global directive and `tls_addr` operation describe native TLS
semantics; target startup/accessor expansion consumes the same typed image facts.
Standalone initial-thread setup is implemented here; hosted object TLS allocation
and relocation belong to later hosted stages. Addresses already follow FS:0.

Total work remains O(N + E log E): fixed linear selection walks, bounded register
pools/reload probes, monotonic parameter flow and sorted phi edges. Each TLS address
has at most 17 native bytes; an accessor adds one return byte. No optional transform,
global retry, broad cache invalidation or body duplication was added. O1–O3 retain
the current O0 policy; later optimizers remain separate work.

## Validation and performance

- Final `make test-pa24`: **290/296**; failures **9 -> 6**, no new failing fixture.
  All three TLS failures pass. Focused properties: **17/17**. All **295** positive
  course programs execute correctly, including the six MIR-shape failures.
- The unchanged 435-file inventory includes 125 excluded solution regressions
  and 14 controls. Its counting convention moves **287/435 -> 290/435**, with
  failures **148 -> 145**. Course coverage and all comparators/references are intact.
- Prior-through: **3856/3856**, **23/23** stages. File audit passes with the same four
  inherited substantial-header warnings. Final checks ran sequentially.
- Personal suites: **236 TLS**, **1820 scalar**, **1259 floating**, **1495 wide**,
  **160 object**, **75 EH/frame**, **21 audit** controls, plus ABI/debug/ELF integration.
  TLS-specific debug, startup initialization, multifile binding, helper-only MIR
  and four native/view identity checks pass. Early invalid personal test inputs
  and their diagnostics remain in evidence; corrected final runs are explicit.
- `trace132.cpp` follows a TLS object and demanded `Counter<3>::advance` through
  source facts, typed LowIR, MIR and ELF; the invalid unused member stays dormant.
  Both runtime argument counts pass. Two functions, 229 text bytes; ELF and native
  disassembly inspected. The source/native driver integration remains later work.
- Frozen A/A plus six ABBA blocks: compiler paired B/A **0.993 / 1.045 / 1.002**
  (scalar/float/wide), peak RSS effectively unchanged. The possible floating
  compiler cost and initial wide samples are disclosed with spreads; no speedup
  is claimed. Final accessor MIR produces byte-identical measured runtime images.
  Seven inherited executable pairs are byte-identical. New TLS checks forty
  million accessor/increment calls in about **0.175 s**, with **247** text bytes.
- Inherited 15% compiler latency/RSS and zero optional text-growth targets remain
  diagnostic and are met by paired medians here. Correctness, course MIR limits,
  comparison rules and finite work/growth bounds remain mandatory. All observations,
  calibration, hashes and absolute new-semantic costs are preserved.

## Remaining implementation and handoff boundary

**Canonical placement/frame protocol (six failures):** strict object-ABI frame;
stack arguments; mixed direct/indirect calls; call-setup forwarding; single-edge
retention. The next owner is function placement and frame finalization: incoming
parameter homes, call-result carriers, setup ordering, Boolean materialization
and final preserve/stack facts must agree with mandatory canonical shapes.
Those programs execute correctly, but exact/structural comparisons still fail.

TLS storage, accessors and their related address consumers are complete. Further
work on these six failures requires a coordinated placement-policy change across
non-TLS ABI functions and its own runtime/compiler validation. TLS symbol maps,
startup or fixups cannot close those shape differences. This is the concrete
handoff boundary, not a waiver of the remaining requirements or whole-stage success.
No additional known TLS correctness/spec defect is deferred as a review question.

Independent review remains pending for both 131 and 132: handler nesting/value
ownership, allocation/alignment restoration, TLS layout/accessor demand and all
mixed-width/pressure interactions against the handout/spec and bounded-work claims.
Implementation validation does not replace that review or move Last reviewed commit.

| Handoff | Code / evidence | Disposition |
|---|---|---|
| Audit130 | through `9f3f9b5c`; audit.md | Independently reviewed; 14 failures remained |
| 131 | `45774fa2`, `c7587f72`; validation131/performance131 | EH/frame group complete; 9 failures; independent review pending |
| 132 | `db965a86`, `03eeff9e`; validation132/performance132 | TLS/address group complete; 6 failures; independent review pending |

Evidence: `/home/vishvananda/work/private/v4codex/artifacts/pa24-132/`.
