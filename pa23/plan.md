# PA23 compact plan — implementation handoff 125

Target: **PA23 full-stage**. Phase: **implementation handoff; audit pending**.
Stage base commit: `f33dd0775073bf5db6665fb2f4f504783159df76`.
Last reviewed commit: `36e612a07bae32eb20d10ffc76bb03529555d4bb`.
Entry: `7a644d69fa5eed60841ed2a231c38591c46d9390`, clean, **24/45**.
Implementation tip: `d9179e848e8b6a5b9ca378a0ee20152404ce166a`.

## Design and spec alignment

Completed class facts own physical shared-base order, lifecycle entries and VTT
slices. Complete/base construction, destruction and transfers consume those
facts; shared bases initialize once and source transfers use the source layout.
Program signature facts own by-value hidden base pointers across every call
and definition; references/pointers retain table-based access without hidden
arguments. Construction tables use the active class's RTTI and complete-object
base locations, including separately owned key definitions and merged TUs.

Program identities own construction segments, receiver adjustors and virtual
member-pointer dispatch callables. Formation, storage, conversion, proofs and
calls share the current callable-plus-displacement LowIR representation.
Semantic member identity remains separate for later host ABI adaptation.
Typed lowering consumes facts directly; it does not replay parsing or delegate
source compilation. [Handoff125](handoff125.md) records owners, data flow,
complexity, lifetimes, bounds and independent audit questions.

## Validation and performance

[Validation125](../student.tests/pa23/validation125.json): PA23 **45/45**, prior
PAs **3811/3811**, through PA23 **3856/3856**; file audit passes with three
inherited warnings. Personal lifecycle/ABI, member-pointer, TU, ownership,
layout, semantic and inherited controls all pass; **44/44** LowIR roundtrips.
Former lifecycle probes, inherited member-pointer failure and deep runtime
fault now pass. All 45 fixtures/statuses and comparison rules remain intact.
[Correction proofs](reference-correction125.md) justify 20 oracle revisions
with reduced C++11/LowIR cases, exact hashes and the pinned reference bundle.

[Performance125](performance125.md) preserves frozen binaries/inputs, A/A and
ABBA latency/RSS, separately checked runtime/text and explicit work/growth
bounds. Apply spec §9's **PA23/O0** acceptance. Historical +15%, +16 MiB and
5.5× diagnostic targets are not extra gates; mandated complexity, correctness
and coverage remain required. New semantic costs are measured separately from
correct A/B lanes. All 19 common native text outputs are identical; six new
behavior inputs execute successfully. The forest adds 8 ms and 3,384 KiB RSS;
compiler text adds 35,072 bytes (1.50%). No optional optimizer was added. Earlier
measurements and [performance124](performance124.md) remain preserved.

## Remaining groups and handoff ledger

No known PA23 implementation group remains unfinished. Independent whole-stage
audit must review construction and signature identity ownership, private LowIR
member-pointer ABI, oracle proofs and measured costs before advancement. The
supplied standalone shared-RTTI backend limitation is reproduced independently;
hosted execution passes. It does not relax source requirements or comparisons.
Native backend, host compatibility, later debug/optimization and self-hosting
remain with their owning assignments.

| Handoff | Range | Result and boundary | Review |
|---|---|---|---|
| Earlier checkpoints and audit124 | Stage base through `7a644d69` | 24/45; owner fixes and remaining lifecycle/member-pointer groups recorded in [audit](audit.md) | Review marker preserved above |
| Implementation125 | `7a644d69..d9179e84`, followed by evidence-only commits | 24→45/45; lifecycle, value ABI, virtual member pointers and proven oracle corrections complete | Full-stage audit pending; implementation handoff does not certify advancement |
