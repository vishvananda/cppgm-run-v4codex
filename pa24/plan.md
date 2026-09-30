# PA24 checkpoint plan

Stage base commit: bde9eb3e128e24923a1de40bb63b8e348a13553b
Last reviewed commit: 9f3f9b5caaec9677af0e8431b51cacebcf823dce

Implementation131 entry: `8c10ee78f9b301ceaba73d6c31c197339745e6ba`.
The preceding audit turn made progress (eleven independent correctness failures
were repaired); fresh entry validation confirms 282/296 and the same fourteen
course failures. The 435 inventory also counts excluded regression/control inputs.
Review markers above remain unchanged.

Current work: generic EH and stack lifetime. Selector owns exceptional edges and
stable homes; per-function MIR records handler/frame facts; encoder owns native
handler records, restoration and typed payload transport; image owns reserved
runtime storage. All work is linear in instructions/edges, with constant-size
handler operations. Validate nested/cross-function cleanup, payload widths,
pressure, normal exits, dynamic storage and alignment explicitly, then rerun the
course and prior suites and frozen compiler/runtime measurements. Canonical MIR
placement/frame protocol and TLS image/wrapper binding remain implementation,
not independent-review waivers.

Checkpoint audit130 reviews the entire first-stage range, including all nine
commits through entry `5bbf5325` and the audit fix `9f3f9b5c`. The checkpoint
review is complete; PA24 is incomplete and must not advance to PA25.
[Audit](audit.md), [validation](../student.tests/pa24/validation130.json), and
[performance](../student.tests/pa24/performance130.md) retain the evidence.

## Architecture and completed groups

PA8 typed Program -> function-owned placement and flat MIR -> direct x86 encoder
and typed fixups -> ELF. Explicit LowIR input and MIR output are adapters, not
production phase transport. Unit-owned compact indexes survive across functions;
selector facts/MIR release after each function. No reference/host delegation.

Handoffs127–129 implement scalar and wide integer/pointer/f32/f64/f80 execution,
parallel phis, atomics, globals, fixed bulk memory, direct/indirect calls, mixed
variadic state and shared object/integer ABI classification. Whole-argument
register rollback, padded object homes, distinct parameter identities and direct
frame chunks are preserved. Decimal floating literals round once per target
format; integer payloads retain all 128 bits without enlarging Operand.

The audit repaired five correctness paths: prepare implicitly widened RHS values
before fixed arithmetic carriers; widen scalar slots by their own type; check
fixed effects over a reused register's new interval; preserve indexed destination
addresses while loading/widening store values; reject reload carrying across
hidden byte-multiply scratch effects. Fixed-effect classification now has one
owner shared by placement and parameter flow. The 21 independent controls expose
11 entry failures, all repaired; 21 canonical LowIR roundtrips also execute.

Work remains O(N + E log E): fixed linear walks, nine GPR/fourteen XMM allocation
choices, at most fourteen call carriers, six-bit monotonic parameter flow and
three 64-instruction carry probes per eligible store. Wide division emits constant
code with 128 target iterations; FP-to-wide splits four digits. Unknown proofs
retain conservative locations. Higher levels currently use the O0 policy;
PA32/33 optimization remains later work.

## Validation and performance acceptance

- `make test-pa24`: **282/296**, the exact same **14** failing fixtures as entry;
  **17/17** focused properties pass. All **287** successfully compiled positive
  fixtures execute correctly, including the six canonical-MIR failures.
- Earlier PAs: `make test-report-through-pa23` **3856/3856**, **23/23** stages.
  File audit passes with four inherited header warnings. Coverage and all
  fixture/reference/comparison files are unchanged.
- Explicit personal suites: scalar 1820, floating 1259, wide 1495, objects 160;
  ABI/debug/ELF integration, parameter flow, audit controls and source/template
  trace pass. Native/MIR output identity and raw instruction bytes were inspected.
- Frozen A/A + six ABBA blocks measure compilation separately from execution.
  Paired compiler ratios: scalar **0.952**, floating **0.964**, wide **0.982**;
  peak RSS is unchanged or lower. All six runtime images are byte-identical.
  Historical measurements and their demonstrated improvements remain preserved.
  No new speedup is claimed. The inherited 15% latency/RSS and zero optional
  text-growth targets are diagnostics, not exit gates; course limits still bind.
  Driver-integrated native/template and self-hosting benchmarks remain later-stage
  work, not invented PA24 gates.

## Broad remaining work

- **Runtime and layout completion:** TLS definitions/wrapper binding; generic EH
  regions, cleanup/resume and required runtime operations; dynamic and over-aligned
  stack storage. Three positive failures stop at TLS and five at `eh_try`.
  Required behavior must be implemented, never replaced with empty operations or
  reference fallback. Later host EH metadata and object linking are separate.
- **Scalar placement and final frame protocol:** strict `100-object-abi-lowered`;
  structural `200-stack-arguments-beyond-six`, `500-mixed-gpr-xmm-call-abi`,
  `600-indirect-mixed-gpr-xmm-call-abi`, `700-call-setup-forwarding-no-preserve`,
  `800-single-edge-callee-saved-retention`. Their execution passes; their mandatory
  canonical MIR contracts remain unsatisfied. Finish this group together, retain
  scratch/ABI correctness and revalidate affected performance.

The three accepted handoffs covered coherent behavior groups, but repeatedly
revisiting scratch effects, incoming parameters and wide consumers was avoidable
fragmentation. Future handoffs should close shared ownership paths with mixed
width/pressure/CFG controls before recording evidence. There is one cumulative
review baseline now; the records-only successor does not move it past code.

Evidence directory: `/home/vishvananda/work/private/v4codex/artifacts/pa24-audit130/`.
