# PA26 full-stage audit plan

Stage base commit: 369c57f19fa13c0394d4fb0345cf2bb79708fc67
Last reviewed commit: 8c541d1e (checkpoint; acceptance pending)
Last full-stage acceptance: pending.
Independent review checkpoint: `8c541d1e` (audit144).

Target: **PA26 full-stage**. Phase: **audit in progress**.
Required checks pass: PA26 **30/30**, through PA26 **4283/4283**, **26/26 stages**;
file audit passes with four inherited nonfatal header-division warnings.
**Do not advance:** uniform default output remains a demonstrated contract defect.
No fixture, reference, harness, comparison rule or coverage changed.

## Spec Alignment and reviewed design

Immutable sources -> streaming interned token cursor -> integrated source and
semantic graph -> retained template patterns/typed substitution facts -> direct
typed LowIR -> per-function MIR -> direct ELF sections and unwind metadata.
Host configuration is build-time target metadata. User source compilation uses
our frontend, lowering, encoder and object writer; the host final link is the
PA26 handout boundary. Production does not serialize or reparse phase views.

[Audit144](audit144.md) reconstructs the actual ownership and data flow, traces
`Guard::~Guard` and demanded `calculate<11>`, reviews legality/work budgets,
records each README requirement, and revisits both previously unaudited handoffs.
CFI/LSDA use final machine offsets, typed region/selector facts, sparse call sites,
one physical resume terminal per function and one local terminate action per TU.
Source/semantic state dies after lowering; MIR dies per function. `8c541d1e`
repairs duplicate native/ELF buffers by moving ownership and streaming sections.
Thirty-seven objects remain byte-identical, including the required string TU.

## Remaining required work

1. Repair default `-c -o <objfile>` for every output suffix while preserving PA25
   separate/direct/mixed linking. `output_policy144.py` independently confirms
   that `.obj` remains private while `.o`, `.bin` and extensionless output
   host-link and run. A suffix-only flag change breaks private ABI consumers:
   the driver, host/private lowering signatures, EH/TLS encoding, object facts
   and linker/runtime boundaries must be handled coherently.
2. Audit the completed ownership-path repair, remeasure affected fixed workloads,
   and rerun required file audit and the root through-PA26 report. Keep the
   reduced output-policy control as a required correctness check for this gap.
3. Consolidate **final** Spec Alignment, findings, performance, validation and
   ledger only after that defect is closed; commit intended work and verify a
   clean worktree. Current green course tests do not prove full completion.

General private host-runtime integration, full hosted compatibility and
self-hosting remain later/out-of-scope surfaces under the handout. They do not
add independent PA26 exit gates, and they do not waive the output contract or
earlier behavior. No alternate ABI container, host compiler delegation, fixture
special case or compatibility shortcut has been introduced to hide this gap.

## Evidence and acceptance

[Performance144](../student.tests/pa26/performance144.md) preserves 280 fresh
frozen A/A + ABBA observations covering all four dimensions. On the affected
32 MiB object workload, peak compiler RSS is **138160 -> 40016 KiB** with
byte-identical objects/executables and unchanged text. Timing variance is
disclosed; no runtime or precise compiler-latency speedup is claimed.
Historical142/143 evidence remains unchanged. The inherited 15% latency/RSS and
blanket zero-growth targets remain diagnostics under spec section 9; mandatory
limits, correctness and coverage stay required.

[Validation144](../student.tests/pa26/evidence144/validation.json) records fresh
commands, inventories, unchanged contract hashes, a PATH-empty compilation
check, and the open output-policy failure. The 4446 summary supplied on entry
differs from both fresh root logs, which report 4283; coverage is unchanged.

| Handoff | Disposition |
|---|---|
| implementation142 | Host-EH/ELF group independently reconstructed; explicit controls rerun; writer memory ownership repaired. |
| implementation143 | Header prerequisites, typed traits/intrinsics, exception-demand ordering and GOT addresses reviewed; controls rerun. Original passing handoff retained in git and evidence143. |
| audit144 checkpoint | `8c541d1e` committed and validated; architecture/performance evidence consolidated; output-name defect remains open. No claim of full-stage acceptance. |
