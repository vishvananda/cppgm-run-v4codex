# PA27 checkpoint audit

Stage base commit: `f833cf1ff361529147361cada33eca62e55330cf`
Last reviewed commit: `cbe7871e211c284ef4a1c571de12db5f26c29777`
Target: **PA27 full-stage**. Phase: **checkpointAudit complete; stage incomplete**.
Reviewed all three accumulated handoffs, all 15 entry commits, their combined
implementation and audit repair `cbe7871e`. Previous goal turn: **progress**;
no previous compiler/test process was live at entry.

The stage remains **157/158**, with the same one failing fixture as audit entry;
PA1–PA26 pass **4283/4283**. Section controls pass **3/3**. Coverage and comparisons
are preserved. The stage cannot advance until its through report passes.

| Group | Accepted checkpoint state |
|---|---|
| Object boundary | Direct ELF, typed section/relocation ownership, lazy COMDAT relocation sections, local-function demand, GOT imports, required base entries and unwind ownership checked together. |
| ABI and emission | Canonical names/arguments, prescribed substitutions, language/internal linkage, extern-template suppression, TLS hooks/guards and TU-local support identities reviewed. |
| Construction and access | Shared storage/action paths preserve defaults, selected variants, lifetimes and constants. Audit fixes inherited anonymous-member access and qualified receivers; semantic projection facts replace repeated lowering walks, and constexpr receivers share activation-owned groups. |

[Audit148](audit.md) records the complete range, ownership/optimization traces,
reference proof review, measurements, exact validation and one ledger row.
[Performance148](../student.tests/pa27/performance148.md) compares the stage base
with the final compiler and the audit entry with final projected-storage paths.
All prior145–147 observations remain intact. Inherited 15% latency/RSS and
zero-growth targets remain diagnostics under spec §9; mandated limits,
correctness, coverage and comparison rules remain required.

Remaining work is one broad **hosted-library/extern-template integration** group:
`200-host-extern-template-vtable-reference.t` still fails at `__builtin_strcmp`
in `<typeinfo>`, before its vtable behavior can be tested. The README excludes
general hosted-header support, but this checked-in fixture remains a required,
unwaived failure. Preserve it and resolve its prerequisites and object demand
together, then pass `make test-report-through-pa27`. No further anonymous-storage
handoff is needed for this checkpoint.

ELF and naming were useful separate groups. The storage handoff's follow-on
scope, reference-dependency and buffer repairs were avoidable fragmentation;
review their shared receiver/lifetime/cache invariants as one group next time.
