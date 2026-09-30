# PA23 checkpoint audit 124

Target: **PA23 full-stage**. Disposition: **checkpoint audit complete; stage implementation incomplete**.
Stage base commit: `f33dd0775073bf5db6665fb2f4f504783159df76`.
Last reviewed commit: `36e612a07bae32eb20d10ffc76bb03529555d4bb`.

The first audit covers the entire accumulated range
`f33dd077..2bba7273` (18 commits, three accepted handoffs), its combined changes
in all 32 implementation/build-registration files, and audit fix `36e612a0`.
The previous review marker was the stage base, so no handoff was excluded.
The previous goal turn made progress: handoff 123 committed the 20→24/45
improvement. Entry was clean and no interrupted build/test process was live.

**Commit coverage.** Each implementation delta was checked against the final
combined ownership paths; intermediate fixes were not treated as independent
proof of the resulting implementation. Documentation, scripts, raw observations,
fixture manifests and historical performance binaries were also inspected.

| Commit | Reviewed contribution and interaction |
|---|---|
| `b28e12ce` | Original stage boundary, retained requirements and validation plan |
| `b1aed715` | Ordinary multi-base views, primary selection, covariance, RTTI and non-null `this` |
| `55e2053a` | Contiguous view/slot arenas and explicit semantic RTTI demand; supersedes eager RTTI reconstruction |
| `72433afa` | Reacquisition by identity after covariant result-layout demand can move class storage |
| `6ae06461` | Distinct native names for private views and pure-virtual runtime role |
| `26b96f27` | Handoff 121 evidence, 17/45 checkpoint, 32/33 controls and preserved measurements |
| `b2010915` | Lifecycle owner plan and unchanged review marker |
| `9417f88f` | Key-owned table references/definitions, bounded deletion shape, lifecycle publication; one justified RTTI oracle correction |
| `8b505a75` | Declaration-to-definition table upgrade, group offsets and telemetry |
| `3b9cf2b5` | Unified base-only lifecycle identity and covariance/group interactions |
| `a04a8cae` | Cross-TU base alias and schedule deduplication |
| `9688f9e5` | Handoff 122 evidence, 20/45 checkpoint, separate/merged TU controls and performance uncertainty |
| `2f27c0e5` | Shared-base owner plan and frozen baseline |
| `c7944d23` | Canonical virtual anchors, ordinary occurrence paths and final-overrider candidates |
| `67354b79` | Nonvirtual extents, segment-local rows, dynamic projections, RTTI, covariance and dispatch demand |
| `08cb8657` | Completion-local receiver-containment cache; key includes both occurrences in this class |
| `6a762f25` | Conservative virtual-path cast hints and nonpolymorphic layout-table RTTI |
| `2bba7273` | Handoff 123 evidence, 24/45 checkpoint, reference reducer and explicit lifecycle boundary |
| `36e612a0` | The five audit findings below, with frozen before/after controls and performance evidence |

**Findings fixed.**

1. Adjustor thunks were cached by TU-local entity IDs even though their external
   ABI entries survive a TU. Two different derived tables in merged inputs
   emitted the same native thunk twice. `Linkage` now owns adjustment interning
   and the cache, keyed by program target-symbol identity plus this/result/virtual
   result adjustments. The target includes internal linkage and deleting-entry
   category. The first requesting TU emits the body exactly once.
2. Secondary-view identities also died with the lowering adapter. In reversed
   input order, a later complete constructor could create a fresh view symbol
   after another TU had already emitted the table. Views now use the program's
   table-symbol identity plus completed view ordinal, retaining internal-class
   separation and reusing already published segments.
3. Key-owned virtual-base classes referenced undefined private views when their
   key definition lived in another TU. Referenced segments now have explicit
   declarations and stable support-symbol identities; definitions upgrade those
   same typed globals. The required separate-global LowIR shape is preserved.
   These are compiler support entries, not a claim of later host C++ ABI closure.
4. Canonical slot origins did not prevent completed views from retaining every
   path through a shared diamond. Nesting produced exponential storage/work:
   depth 16 retained 786,358 views and 262,143 slots. Every imported candidate
   still participates in final-overrider resolution; afterwards views coalesce
   by physical occurrence and view type, with parent/receiver IDs remapped.
   The same input now retains 578 views and 35 slots. Ordinary repeated bases
   and primary aliases remain distinct where their identities/types require it.
5. Layout of each base projection rewalked its entire interned nonvirtual path.
   The semantic owner now completes and reuses the next projection's fact,
   rebasing a shared anchor in the current source class and retaining its
   nonvirtual tail. Work is once per complete path key; no extra offset cache
   or lifetime is introduced. The new counter records existing computations.

[Audit controls](../student.tests/pa23/audit124-final.json) pass **21/21**;
[entry observations](../student.tests/pa23/audit124-entry.json) pass 9/21.
The twelve resolved failures cover separate and merged TUs in both orders,
ordinary/covariant/null thunks, templates, and external virtual-base key owners.
Internal targets stay separate. Chain and nested-diamond inputs check layouts,
work and valid emission. [Deep overrider probes](../student.tests/pa23/deep124-final.json)
retain the later ambiguous override rejection and accept the resolved class.
Their resolved executable still faults in both frozen entry and final binaries:
this is an additional retained reproducer of the unfinished virtual-base
construction owner, not a fixed runtime test or a newly accepted behavior.
The provisional control/performance records remain available and are not
relabeled as final results.

**Architecture trace and ownership.** The ordinary trace is `D::b` called
through `B&` in `shared-thunk`; the demanded-template trace is `D<5>::b` in
`template-thunk`, whose unrelated dependent `dormant` member stays undemanded.
`lowering/driver.cpp` owns immutable preprocessing sources, the streaming
post-token/parser cursor, the source graph and semantic fact arenas for one TU.
`Parser::translation_unit` hands each parsed declaration to semantic construction;
there is no completed syntax-tree copy into a second semantic tree. Identifiers
and types have compact identities. Retained template source regions project
occurrence/context IDs; instantiation does not replay grammar or clone source
bodies. `template_type_facts.cpp` keys parent-linked substitution frames with
all specialization/parameter/argument/context inputs. Nondependent source/type
facts remain shared; dependent checking and selected-declaration publication
belong to the concrete environment.

Scope/name indexes, candidate sequences, canonical subobject paths and immutable
class-base edges supply the selected member and receiver. Completion separates
shared virtual anchors from ordinary occurrences; final-overrider candidates,
abstractness, primary signatures and covariance are established before lowering.
Containment scratch belongs to one class completion, including cached misses.
Layout publishes complete/nonvirtual extents, virtual rows and tails, and each
segment's actual address point. Completed base paths and RTTI flags are computed
once at their semantic owners. RTTI demand is deduplicated and finalized after
source/body discovery; key-definition notifications wake the owning table,
not every pending class. No global generation counter, retry sweep or textual
semantic recovery was introduced.

`Procedural` consumes those identities directly into `lowir_model::Program`
and `FunctionBuilder`. Manglings are output spellings, never lookup keys.
`Linkage` and the typed ABI graph survive merged TUs; view/thunk keys now have
that same lifetime. Class/slot vectors and path indexes release with the TU;
completion traversal/coalescing scratch releases after publication, table buffers
after emission, and the function builder after its body. These are contiguous
arenas/slices and flat indexes, not individually owned slot/view heap nodes.
No new process-global mutable cache exists. Retained LowIR is the explicit
PA23 output program, released after its writer finishes; full validation is an
explicit audit option, not a repeated ordinary phase boundary.

The trace reaches ELF through the handout's supplied backend, which consumes
our explicit LowIR output. `readelf` on the merged template control confirms
one `_ZThn8_N1DILi5EE1bEv` entry, and the saved disassembly shows its receiver
adjustment and call to the selected definition. All 21 audit programs execute
through that standalone backend as well as hosted object linking. The compiler
does not delegate source semantics/lowering. Its own MIR/selection/allocator/
ELF writer belongs to PA24, so no source-to-native production implementation,
debug or self-host completion is claimed at this stage.

**Optimization legality, profitability and budgets.** The inherited useful
fact is non-null source `this` ([N3485](../doc/n3485.txt) §5.1.1/2).
`expression.cpp` records it; `base_projection` preserves it across a valid base
conversion; `pointer_projection` eliminates a null branch only when that proof
(or a zero fixed adjustment) applies. Unknown pointer values stay conservative,
and covariant pointer returns retain their null check. There is no fact hoisting
across effects or strengthening of alias/unwind promises. This is constant work,
zero code growth, and the original repeated 428→396-byte runtime/text evidence
is preserved. Audit-generated text for that input remains 396 bytes.

The O0 deleting-entry policy expands at most two prepared nontrivial actions,
never a source body; larger/observable bodies call the complete destructor once.
This bounds normal/cleanup growth (at most five subobject-call sites and four
deallocations). Exception/order controls remain 20/20. The prior +146-byte cost
is required comparison shape, not an optional optimization accepted on a noisy
speed claim. Shared-view coalescing and path-fact reuse eliminate duplicated
compiler work with unchanged executable code. They preserve all candidates and
ABI decisions before publication, use linear scratch/work in imported facts,
and retain no speculative bodies. There is no fixed point or optional O1–O3
transform requiring a new work/growth policy. Native loop/spill quality belongs
to the later native stages; checked runtime benchmarks ensure compiler gains
are not concealing changed executable work.

[Performance124](performance124.md) applies spec §9's PA23/O0 acceptance to the
whole range and inherited plans. All historical binaries/hashes, measurements,
paired spreads and disclosed semantic costs remain preserved. The longer final
shared-diamond input improves compiler median **566.00→257.99 ms**, RSS
**208,624→54,400 KiB**, with all four ABBA pairs improving; all **19** measured
native texts are byte-identical. Compiler text grows 4,352 bytes and ordinary
peak RSS at most 236 KiB. Smaller timing uncertainties remain disclosed.
Historical +15%, +16 MiB and 5.5× targets are diagnostic, not mandated gates;
no correctness, coverage, work bound or mandated requirement was reclassified.

**Reference review.** The only oracle change in the entire accumulated range
is the VMI RTTI flag 0→1 in the nonvirtual destructor diamond. The reducer,
bundle revision `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, and proof are in
[reference-correction122.md](reference-correction122.md). N3485 §10.1/4 gives
one subobject per ordinary base occurrence, and §5.2.7/8 supplies the public
cross/downcast rule. Independently inspected [Itanium ABI §2.9.5](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#rtti-layout)
assigns flag 0x01 to repeated non-diamond inheritance, including indirect bases.
A virtual destructor does not make the base-specifiers virtual. The correction
is justified independently of compiler agreement. The audit manifest checks
that exact one-field change against the stage base; source, slot/body order,
status and comparison rules are intact. No oracle changed during this audit.

[Reference observation123](reference-observation123.md) still documents a
separate incorrect fixed-offset read through a nonpolymorphic virtual-base
reference. Its reduced proof and measurements are retained. Full affected
oracles must be reconciled alongside completed lifecycle/ABI output; this audit
neither copies the erroneous result nor weakens the comparator.

**Required checks and coverage.** [Validation124](../student.tests/pa23/validation124.json)
contains exact commands, terminal exits, log hashes, reviewed commits, compiler
identity, evidence hashes and all 45 fixture hashes. `make test-pa23` is
**24/45**, exit 2, with the identical **21** failing fixture names as entry.
The status field `stageTests fail (2)` was a command exit code, not two failed
fixtures. `make test-report-through-pa22` passes **3811/3811**, 22/22 stages;
`make test-report-through-pa23` is **3835/3856**. The file audit passes with its
three inherited header-organization warnings. Thus `priorThroughTests`,
`fileAudit` and `stageProgressPreserved` pass. Additional personal passing cases
have not been used to offset any additional course failure.

All 44 accepted outputs roundtrip stably. Existing semantic controls pass 26/26,
completed layout controls 17/17, lifecycle controls 20/20, and inherited controls
32/33. The two known lifecycle probes and virtual-member-pointer defect remain
visible. The standalone backend passes 16/17 completed layout controls; its
shared-RTTI scan limit is reproduced again with reference and student IR in
[backend-limit124](../student.tests/pa23/backend-limit124.json), while both pass
hosted execution. No fixture, comparator, expected status or required coverage
was removed. PA23's through report is not green; advancing to PA24 is prohibited.

**Remaining work and handoff assessment.** Finish virtual-base complete/base
lifecycle actions and the by-value hidden-parameter ABI as one group across
construction/VTT selection, forwarding, transfer, placement-new and cleanup;
include the documented oracle reconciliation. Finish virtual member-function
pointer formation, conversions, value facts and dispatch as the other group.
Then rerun the full stage/through contract and applicable performance evidence.
The three handoffs identified real semantic owner boundaries, but repeatedly
sealing partial table/lifecycle publication caused avoidable fragmentation:
ABI identity lifetime and nested shared-graph scaling should have been checked
across those boundaries earlier. Broad owner-complete work, including both TU
orders and depth as well as width, is the next unit of delivery. None of the
remaining stage failures is waived by accepting this checkpoint audit.

| Audit | Reviewed range | Findings/evidence | Disposition |
|---|---|---|---|
| 124 | `f33dd077..36e612a0`, including all three accumulated handoffs | Five ownership/complexity fixes; 21/21 new controls; frozen 19-workload performance; prior 3811/3811; PA23 unchanged 24/45; exact fixture/ref audit | Checkpoint accepted; lifecycle/parameter ABI, oracle reconciliation and member-pointer implementation remain required |
