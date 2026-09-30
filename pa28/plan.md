# PA28 implementation plan

Target: PA28 full-stage. Phase: implement, handoff151 in progress.
Stage base commit: `bec9389f62014dfecac8b41d330de142e0904b8c`.
Last reviewed commit: `bec9389f62014dfecac8b41d330de142e0904b8c`.
Entry result: **81/97**, 16 failures; earlier PA1–27 **4441/4441**.
Review markers remain fixed until independent review.

## Design and remaining groups

| Group / owner | Data flow, complexity and validation |
|---|---|
| ABI naming: declaration attributes, canonical semantic ABI facts, Itanium encoder | Publish tags and dependent/local identities once; feed typed facts to shared mangler and ELF. Work proportional to declarations and encoded names, TU-owned caches. Inspect required raw symbols and host-link behavior, including rejection and template controls. Initial group: tagged constructors/destructors/support objects, template parameter and local/lambda names. |
| Virtual objects: semantic completion/layout, lowering projections | Final layout owns covariance adjustments and vtable projections; lazy template bases must provide RTTI. Work follows demanded classes, slots and inheritance edges. Validate layout-finalization, lazy-base cross-cast and virtual-base return-condition cases. |
| Exceptions: semantic lifetime facts, lowering EH regions, native LSDA | Cleanup follows active lifetime/handler state; dynamic exception specifications retain host filter semantics. Validate rethrow outer local, resume lexical lifetime and unexpected cases with object/runtime controls. |

Preserve direct typed source → LowIR → MIR → ELF, canonical identity, demand
states and bounded work from spec §§1–8. Complete related fixes at each owner;
no fixture-specific implementation or reduced coverage. GNU callable effects
and already-passing ABI behavior remain required.

## Performance and validation

Freeze entry/final compilers and input hashes. Measure separate compiler latency,
peak RSS, executable runtime and text size using A/A and ABBA observations on
equivalent correct workloads; newly supported cases provide standalone costs.
Retain raw observations and spread. No optional optimization is planned. Spec §9
stage scope applies: inherited blanket 15%/zero-growth targets are diagnostics,
not mandated limits; existing work/growth limits, correctness and coverage stay.
Run explicit personal controls, `make test-pa28`, the root through report and
`perl scripts/cppgm_file_audit.pl --stage pa28 --paths dev/src` before handoff.

## Handoff ledger

151 implementation increment: ABI attributes, dependent template identities,
type-producing builtin queries and local/lambda naming now pass the ten owning
fixtures. PA28 **91/97** and the through report **4532/4538** show no earlier
failures. Fourteen explicit naming/effect controls pass, including a host-built
consumer and effects added after an instance was declared. Final validation and
frozen performance evidence are pending.

Implementation unfinished: virtual layout/completion (3) and EH (3), with the
same six entry fixtures still failing. Independent review pending: cumulative
PA28 architecture, semantic and performance evidence; no review requirement is
waived. The naming increment does not claim these separate owners are complete.
