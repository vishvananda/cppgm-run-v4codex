# PA27 implementation handoff

Stage base commit: `f833cf1ff361529147361cada33eca62e55330cf`
Last reviewed commit: `cbe7871e211c284ef4a1c571de12db5f26c29777`
Target: **PA27 full-stage**. Phase: **implementation149 complete; full audit pending**.
Entry HEAD: `226b88668284e116a4d273627bf4810bd682226e`.
Code tip: `df5aef0879cb53129cf6b524229559fd4b0a4aa9`.
Previous turn: **progress** (audit148 repairs/evidence); no live process at entry.

The required stage improved **157/158 → 158/158**. Earlier PAs pass **4283/4283**;
the root through-PA27 report passes **4441/4441**, section properties **3/3**.
File audit passes with four inherited header warnings. All fixture, reference,
harness and comparison paths are unchanged. The hosted stream fixture remains
required despite the README's broader hosted-header exclusion, and now links
and executes successfully. [Validation](../student.tests/pa27/evidence149/validation.json)
pins commands, outputs, code/binary hashes and unchanged coverage.

## Design and completed ownership groups

| Owner | Data flow, complexity and validation |
|---|---|
| Preprocessor / syntax | Named variadics use the existing argument slices; omitted and explicit-empty tails remain distinct. Header probes share include search, inspect candidates without reading headers and preserve header tokens. GNU null literals and enum attributes use ordinary typed syntax. Work follows tokens and include candidates; positive/negative macro and search-path controls pass. |
| Builtin semantics / lowering | Interned builtin → canonical signature → ordinary conversions → typed intrinsic or C runtime symbol. String comparison calls retain ordinary linkage. Atomic signatures are cached by kind/pointee; arguments evaluate once, and each call adds one atomic plus at most one subtraction. Width, pointer-byte, side-effect and four-thread controls pass. New lowering source is registered. |
| Template declarations / access | Dependent aliases retain typed bases; friendship follows canonical primary/partial identities. Explicit instantiation applies existing partial ordering over matching candidates. Sole-void lists normalize before raw-parameter reconciliation. First-signature lookup is retained while the selected source head owns access recipes. Qualified receivers stay typed; lexical access may remain in an unrelated friend template without demanding that class. Receiver/body contexts still require projection. `(frame, recipe)` facts and subtree pruning bound checks to demanded source uses and live for one TU. Positive, negative, SFINAE and earlier signature controls pass. |
| Existing object / ABI / storage | Earlier direct ELF, lazy COMDAT relocations, GOT imports, local-function demand, base entries, unwind, canonical mangling, TLS, extern-template suppression and projected storage remain shared. The newly accepted source reaches that pipeline directly, without host compilation or textual transport. Required host link/runtime and inspection suites pass. |
| Diagnostics | Missing declaration names/locations retain a valid generic diagnostic. The 33 earlier signature cases now reject normally rather than asserting while formatting an error. |

[Audit148](audit.md) retains the prior cumulative review and reference proofs;
[performance148](../student.tests/pa27/performance148.md) retains full-stage
base comparisons. No reference correction or coverage reduction was made here.

## Performance acceptance

[Performance149](../student.tests/pa27/performance149.md) records 304 final
observations: frozen A/B, A/A calibration, six ABBA blocks, compiler latency/RSS,
checked runtime and text size. All five supported pairs produce byte-identical
objects and executables. Signature checks add 2000 required access facts for
1000 demands, with unchanged parsing, specialization/body and native counts.
Final RSS rises 272 KiB; paired latency is 1.138 (0.973–2.919). The earlier
pre-diagnostic measurement was 1.003 (0.980–1.022); all 304 earlier samples remain.
Timing variation does not support a precise speedup or slowdown claim.
New hosted behavior costs 1.0747 s / 67,568 KiB to compile, 0.5043 s runtime for
200,000 checked parses, and 31,873 executable text bytes. The entry cannot compile
that source, so it is not an optimization baseline. No optional transform was
added. Existing mandated work limits remain; inherited 15%/zero-growth targets
remain diagnostics under spec §9. Correctness and coverage are not waived.

## Remaining work and handoff ledger

Unfinished implementation in this group: **none known**. Required checks pass;
66 new control commands and 33 inherited signature cases pass explicitly.
Independent review: Ralph's full-stage audit must review the combined stage and
resolve any whole-stage findings before advancement. This implementation handoff
does not certify that audit and does not waive its requirements.

| Handoff | Commits / boundary | Result |
|---|---|---|
| 149 | Entry `226b8866`; initial plan `d1b017e5`; prerequisites `2cfcb7c9`; template/access integration `0994586d`; diagnostic repair `df5aef08`; final evidence follows. | One hosted integration group completed through source semantics, object execution, prior-stage validation and measured costs. The original failure is resolved with unchanged coverage. Clean committed handoff returns control for full-stage audit. |
