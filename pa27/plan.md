# PA27 implementation handoff

Stage base commit: `f833cf1ff361529147361cada33eca62e55330cf`
Last reviewed commit: `f833cf1ff361529147361cada33eca62e55330cf`
Target: **PA27 full-stage**. Phase: **implement146 in progress**.
Current: **133/158**, down from **42 to 25 failures**, no new failures.
PA1–PA26: **4283/4283**. Independent review markers remain unchanged.

## Design/spec alignment and completed group

Host-object placement now consumes canonical binding, section, alignment,
definition and fixup-owner identities. GNU object attributes are parsed once,
stored on immutable source nodes, checked per entity and passed directly through
typed LowIR to native records. No textual phase transport, mangling-based
semantic recovery or external code generation was added.

| Owner | Completed data flow and complexity | Evidence |
|---|---|---|
| Native object demand | Typed roots/operands retain referenced locals, required ABI roots and exported definitions; omit unused locals/inline bodies before selection. Deduplicated O(symbols+operands) worklist. | pruning, qualified member linkage, retained-base fixtures; personal EH/own-link controls |
| Native selection/encoding | Imported and replaceable weak data use GOT addresses, including loads/stores and offsets; known strong definitions keep PC-relative references. O(1) identity lookup per access. | imported data, specialization data, inherited C linkage; DSO/PIE and strong-over-weak controls |
| ELF placement/unwind | Actual weak bodies/data and their relocations share COMDAT groups. Named sections preserve alignment, pointer fixups and aliases. FDEs follow moved bodies. O(S log S + bytes + R); lazy relocation sections. | duplicate/coalescing/body inspections, both EH link orders, native own links, section controls |
| Driver/attributes | `-g0` and both `-isystem` forms; section name/arity/conflict rejection; weak metadata. | 3/3 section controls; system-header move body retained |

## Unfinished implementation (not audit questions)

| Remaining owner | Required next behavior |
|---|---|
| Semantic linkage and typed ABI graph | Dependent result/NTTP substitution spelling, standard substitutions, typedef-anonymous type identity, lambda/internal-template linkage and local-static isolation; two C-linkage cases |
| Template declaration/body demand | Extern-template constructor/destructor, member-template/callee closure, static data and vtable references |
| TLS ABI lowering | Required host TLS wrapper identities and import/export surfaces |
| Initialization/lifetime facts | Anonymous storage owners and multi-level virtual-base/construction-vtable behavior |

The [exact remaining/resolved inventory](../student.tests/pa27/evidence145/validation.json)
preserves all 25 failures. These require missing or incorrect upstream entity,
template-demand, ABI-entry or lifetime facts. Recovering them in ELF placement
would violate the spec's ownership boundary. The completed image/section group
cannot repair them; further work requires a separate coherent semantic group,
not another extension of this writer. Whole-stage correctness remains required.

## Performance and validation

[Performance145](../student.tests/pa27/performance145.md) preserves **448** frozen
A/A+ABBA observations, latency/RSS/runtime/text together. Common text is unchanged;
required pruning removes 30,816 bytes. Compiler paired medians range 0.973–1.085,
with noise disclosed. Lazy relocation sections remove 234,608 avoidable object
bytes from the 2,400-template case. Remaining metadata growth has explicit linear
budgets; no optional optimizer or new percentage exit gate was introduced.

[Validation145](../student.tests/pa27/evidence145/validation.json) pins final
binary/source hashes and exact commands. Prior-through and file audit pass
(four inherited header warnings); `make test-pa27` reports 133/158 plus 3/3
section controls. `make test-report-through-pa27` reports 4416/4441, with failures
only in PA27. Personal object controls pass **53 commands**. All 19,749 tracked
contract paths remain, with no source/harness/comparison changes. The sole
[reference overlay](reference-corrections.md) corrects two global-name inspections
using reduced inputs, C++11/ABI proof and the pinned bundle revision.

## Handoff ledger / independent review

- Entry145 (`554f05f0`): clean entry and authoritative baseline recorded; previous
  turn supplied stage evidence and is classified as progress.
- `c36f3501`: connected object emission/attribute/demand implementation and
  personal controls; first performance observations retained.
- `96e1cb72`: measured empty-relocation-section growth removed; final required
  checks and frozen benchmark observations repeated on this exact binary.
- Handoff boundary: implementation progress is validated; PA27 remains incomplete.
  Independent audit must review source-attribute identity, COMDAT alias/FDE
  ownership, GOT scratch lifetimes and the recorded cost/bounds. These questions
  are separate from the known implementation failures above; neither is waived.

- Entry146: HEAD `5aefb9626f568da2b8060cba7096cde931b68435`, clean; prior
  handoff changed authoritative implementation and validation (progress).
  Turn baseline 133/158, 25 failures; stage/review markers above preserved.
  Initial group: semantic entity linkage → typed ABI graph → stored symbol and
  binding → LowIR/object. Review canonical identity and substitution ownership;
  require O(graph nodes + emitted name bytes) work, no text recovery. Extend to
  related linkage/local-static/template naming failures as that ownership permits.
  Validate focused fixtures, explicit personal cross-TU controls, full stage and
  prior-through reports, file audit, frozen compiler/runtime/RSS/text evidence.
- Implementation146 naming checkpoint: 146/158, resolving 13 entry failures.
  Typed ABI adapter now preserves internal argument linkage, C-language naming,
  typedef linkage names, prescribed standard substitutions, dependent NTTP type
  annotations and source-only default expressions. Unresolved qualifier encoding
  owns substitution order. Personal linkage controls pass 22 commands. Extending
  the group to extern-template emission and local closure identities; root checks
  will run sequentially because their runner bookkeeping is shared.
- Implementation146 emission checkpoint: 155/158. Extern-template suppression
  now reaches function/global/constructor entries; ordinary undefined template
  references are strong (archive extraction verified). TLS wrapper/init entries
  use typed ABI targets and optional weak initialization hooks. TU-local closure
  ordinals are separate from ODR signature ordinals. A TU-owned support-symbol
  cache fixes duplicate internal VTT identities, resolving both virtual-base
  crashes without changing the lifetime model. PA1–PA26 passed 4283/4283 after
  template emission changes; final cache/TLS checks and performance are pending.
- Remaining implementation boundary: two anonymous-storage initialization cases
  require semantic constructor actions retaining the injected field's storage
  path and lifetime owner; they currently reject before lowering. The remaining
  hosted-vtable case stops parsing `/usr/include/c++/15/exception:87`. Neither
  can be repaired by ABI naming, binding or support-symbol emission. Full-stage
  implementation and independent review remain required.
