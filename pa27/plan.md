# PA27 implementation handoff

Stage base commit: `f833cf1ff361529147361cada33eca62e55330cf`
Last reviewed commit: `f833cf1ff361529147361cada33eca62e55330cf`
Target: **PA27 full-stage**. Phase: **implement; incomplete handoff146**.
Current: **155/158**, down from **25 to 3 failures** this turn; no new failures.
PA1–PA26: **4283/4283**. Independent review markers remain unchanged.

## Design/spec alignment and completed groups

Semantic identities and immutable type/query facts feed the typed PA9 ABI graph;
LowIR records carry the resulting spelling, binding and emission facts. No text
roundtrip, fixture-specific answer, semantic name reconstruction or external code
generation was added. Standard-library abbreviations implement the ABI's required
name compression; they do not alter language/library semantics.

| Owner | Data flow, complexity and boundary | Validation |
|---|---|---|
| Semantic declarations | First typedef denoting an unnamed type supplies a separate linkage-name fact; C linkage survives namespace context and rejects static redeclaration. O(declarators); facts keyed by entity. | Host-built typedef boundaries, C addresses/rejections |
| ABI type/query adapter | Original template signatures, dependent NTTP types, source expressions and canonical components produce standard/dependent spellings. Per-type/query caches; O(graph edges + emitted bytes). | All required naming fixtures, standard-type near misses |
| Linkage/emission | Internal type arguments propagate through specialization ownership; suppression reaches function, ctor/dtor and static-data emission. Ordinary undefined references are strong. Cached scope/type identity; suppression follows lexical owners. | Cross-TU storage separation, archive extraction, extern-template closure |
| TLS lowering | One typed wrapper/init identity per variable, optional weak host init hook, one guard per dynamic definition. O(TLS entities + initializer IR), fixed wrapper growth. | Required import/export; constant/dynamic, O0/O2, both link orders, threads |
| ABI support entries | One TU cache entry per `(special-name kind, canonical ABI type)`; internal VTT uses share the defined symbol and remain separate across TUs. Average O(1) lookup, TU lifetime. | Both virtual-base fixtures; cross-TU VTT/body/LowIR controls |
| Local closures | Separate source ordinal for TU-local names and signature ordinal for ODR contexts. One existing O(C log C) source sort. | Local EH lambda binding and distinct-signature controls |
| Prior145 object group | Canonical demand/placement, GOT, actual-body COMDAT/FDE ownership, named sections and relocation alignment. O(symbols+operands), placement O(S log S + bytes + R). | Inherited 53-command object controls and section properties |

## Unfinished implementation (not audit questions)

1. **Anonymous-storage constructor actions**: two required fixtures reject
   `initializer does not name a member or base`. Injected fields need their
   storage-owner path retained in initialization/default/lifetime actions. Merely
   accepting the name would still incorrectly default-construct the storage
   before a nontrivial projected initializer. This needs constructor-action and
   cleanup ownership work, not ABI symbol changes.
2. **Hosted-header parsing/vtable demand**: the remaining hosted-vtable fixture
   stops at `/usr/include/c++/15/exception:87` (`expected ')', found '('`). The
   parser/hosted-header surface must be completed before its vtable behavior can
   be assessed. No hosted fixture, comparison or requirement is waived.

The [exact inventory](../student.tests/pa27/evidence146/validation.json) records
all 22 resolved cases and the three remaining cases. The ABI/emission group is
complete at this handoff. Continuing those failures requires different upstream
parser and constructor-action models; repairing them through symbol emission
would violate the spec's ownership boundary. Whole-stage implementation remains
required; this handoff does not certify the assignment.

## Performance and required checks

[Performance146](../student.tests/pa27/performance146.md) retains **280 frozen
A/A+ABBA observations**, including compiler latency/RSS and checked runtime/text
size. Text is unchanged on all five workloads. Prescribed substitutions remove
110,400 object bytes on the focused naming input. Paired compiler medians range
0.964–1.011; full spread/noise and necessary structural budgets are disclosed.
No optional transform or new percentage gate was introduced;145 evidence remains.

[Validation146](../student.tests/pa27/evidence146/validation.json) pins binary and
source hashes and exact commands. Sequential prior-through and file audit pass
(four inherited warnings); `make test-pa27` is **155/158**, with **3/3** section
controls. `make test-report-through-pa27` is **4438/4441**, failures only in PA27.
Personal controls pass **48 linkage + 38 TLS + 53 inherited object commands**.
All 19,749 tracked contract paths and 158 stage anchors are preserved. No source,
reference, harness or comparison changes this turn. Prior145's documented
[reference overlay](reference-corrections.md) is unchanged.

## Handoff ledger / independent review

- Entry145 `554f05f0`, implementation `c36f3501` and `96e1cb72`: object demand,
  placement and measured relocation-section correction; handoff145 `5aefb962`.
- Entry146 `364ebbcc`: clean HEAD `5aefb962` and 133/158 baseline recorded; the
  previous turn changed implementation/evidence and is classified as progress.
- `b0553a06`: typed semantic linkage/ABI naming; 146/158 and personal controls.
- `bb608e0b`: template/TLS/local-closure/support-object emission; 155/158, required
  checks, explicit controls and frozen performance on this exact binary.
- Handoff146 boundary: ABI/linkage/emission implementation progress validated;
  parser and anonymous-storage action work above remains unfinished.
- Independent audit must review semantic linkage-name identity, substitution
  slot ownership, suppression versus body demand, TLS weak-hook/guard lifetime,
  internal support cache isolation and recorded costs/bounds. Preserve prior145
  review questions about source-attribute identity, COMDAT alias/FDE ownership
  and GOT scratch lifetimes. These are separate from unfinished implementation;
  neither category is waived and the review markers above are not advanced.
