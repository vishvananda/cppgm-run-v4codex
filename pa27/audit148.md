# PA27 checkpoint audit148

Stage base commit: `f833cf1ff361529147361cada33eca62e55330cf`
Last reviewed commit: `cbe7871e211c284ef4a1c571de12db5f26c29777`
Audit entry: `7dbb067893e16c136d4f79b1048092f7c1f8e044`.
Disposition: **checkpoint audit complete; PA27 remains incomplete, 157/158**.

This is the first independent PA27 review. Its range is the stage base exclusive
through the code tip inclusive: all three handoffs, all 15 entry commits and the
audit repair, including their combined **61 implementation paths** and interactions.
[Reviewed range](../student.tests/pa27/evidence148/reviewed-range.json) enumerates
every commit/path and final implementation hashes. `spec.md`, PA27's README,
`TESTING_AND_REFERENCES.md` and the inherited plan governed review. No earlier
handoff was used as a substitute for reviewing the combined implementation.
The interrupted preceding turn left no live process; this turn made progress
through review, a reduced failure, repairs, measurements and committed validation.

## Complete accumulated range

| Commits | Review and interaction evidence |
|---|---|
| `554f05f0`, `c36f3501`, `96e1cb72`, `5aefb962` | Entry plan, ELF/GOT/sections/demand implementation, lazy relocation correction and evidence. Reviewed byte-lane movement, alignment, symbol aliases, relocation-owner offsets, group membership/signatures, FDE section references and imported-address scratch lifetimes. Object controls cover DSOs, both COMDAT link orders, exceptions, explicit sections and own-link paths. |
| `364ebbcc`, `b0553a06`, `bb608e0b`, `8d595af6` | Naming plan, canonical semantic/ABI identities, suppression/TLS/support corrections and evidence. Reviewed typedef linkage names, dependent NTTPs, unresolved-name substitution slots, standard substitutions and near misses, C/internal linkage, closure ordinals, strong undefined archive references, constructor aliases and per-TU support caches. TLS controls cover constant/dynamic imports/exports, threads and both object orders. |
| `7c71b6e6`, `1c0b2d10`, `ebf263c4`, `2eba9b7b`, `53b8a76d`, `79dffd03`, `7dbb0678` | Storage entry, action/lifetime/default/constant implementation, enclosing-constant and pointer-attribute fixes, dependency completion, flat buffers and evidence. Reviewed selected union variants, destination versus receiver paths, declaration-order actions, partial unwind, immutable initializer contexts, reachable-reference dependency keys and flat activation ownership. |
| `cbe7871e` | Audit repairs and explicit controls: anonymous inherited access, qualified receiver identity, canonical semantic field projections and shared constexpr receiver groups. Reviewed together with all previous storage, template and lowering consumers. |

## Findings and repairs

**Inherited anonymous-member access was incorrectly rejected.** A base with a
prefix and anonymous union, inherited twice through `L` and `R`, rejected valid
`d.L::a` and `d.R::a` as inaccessible. The entry failure is retained in
[the reducer record](../student.tests/pa27/evidence148/entry-reducer.json).
Access checking now checks inheritance at the enclosing declaration and checks
the injected member at its own introduction. Anonymous storage is not an extra
base. Qualified receiver records retain the selected naming class, so lowering
includes the anonymous-storage offset after the recorded base adjustment.
N3485 [class.union]/5 places anonymous-union members in the enclosing lookup
scope; [class.member.lookup] and [class.access.base] govern the selected base and
its access. The [local standard](../doc/n3485.txt) supplies these rules; host
agreement corroborates the reduced standard anonymous-union cases. The course's
nested anonymous-struct extension is additionally checked against explicit values.

**Lowering repeated storage-path reconstruction.** Expression and binding paths
walked injected-field ownership and composed offsets for each use. The semantic
owner now publishes cached `FieldProjection` facts keyed by canonical field and
normalized receiver-class identities, sharing tails. Completed layout is an
invariant; a missing layout throws rather than triggering semantic reconstruction.
Qualifiers and pointer/reference wrappers do not affect this key; specialization
entities and the owning TU do. Facts are immutable after layout, need no global
invalidation, and die with the TU. Lowering consumes the recorded offset/root.
The focused 600-class input records 5,400 projection facts, 1,200 constructor paths
and 3,000 actions; the 8,192-object constant input records only nine projection
facts and two prepared constructor paths.

**Constant receivers repeatedly traversed shared prefixes.** Default actions
previously recursively rebuilt the same receiver path. They now reuse the
activation's flat storage-group address index, also consumed by projected value
construction. Each selected path is prepared once per activation, then frozen
once. References still include reachable storage versions/liveness in activation
keys; the audit did not weaken dependency validity or the constant step/depth
limits. No optional generated-code transform was added.

[Controls](../student.tests/pa27/evidence148/controls.json) contain **63 passing
commands** for qualified repeated/virtual bases, constexpr reads, nested defaults,
private/protected/ambiguous rejection, and imported integer/FP/aggregate/indexed
address operations. The inherited 261 personal commands also pass on this tip.
A host compile attempt for nested default-initialized anonymous structs was
rejected by GCC's extension restriction; the course compiler's explicit-value
control remains, while standard anonymous-union controls run on both compilers.

## Architecture and optimization traces

[Trace source](../student.tests/pa27/audit-trace.cpp) constructs nontrivial
`Resource` inside demanded `Value<11>`, reads its projected fields and destroys it.
[Trace evidence](../student.tests/pa27/evidence148/trace.json) pins source, binary,
LowIR/MIR/object inspection hashes and counters. The checked host-linked program
returns zero and observes exactly one destruction.

Immutable source and the streaming interned cursor feed `parser.translation_unit`
and the cooperating semantic analyzer (`lowering/driver.cpp`). The trace consumes
148 tokens with maximum pending 59. `syntax/occurrence.cpp` reuses parsed source
regions and compact context occurrences; `template_instantiation.cpp` uses
canonical specialization/context keys and monotonic body states. Non-dependent
facts are inherited, dependent facts checked in parent-linked substitution frames;
no template body reparses tokens. The trace records two specialization entities,
two inherited expression facts and zero expression variants. Existing typed
member/demand queues preserve suppression without retrying unrelated bodies.

Completed layouts and selected declarations feed constructor-owned actions and
storage paths. The trace has four actions, one projected storage path and five
semantic field projections. Direct typed LowIR carries offsets 8 and 12 and
partial cleanup at offset 8. Native selection consumes that LowIR; one function's
MIR/frame/placement temporaries are released after encoding. The native trace has
seven functions and 101 selected instructions. Native buffers transfer into the
ELF writer; symbol placement, alias, relocation and FDE ownership follow typed
identities, and final sections stream directly. Six observed COMDAT groups have
relocation members only where needed. Source/semantic state dies before native
selection; ABI linkage pools live for the program; group/value buffers live for
one constant activation. No new hot shared ownership, per-node allocation,
process-global mutable cache or production text roundtrip appears in this range.
Explicit LowIR/MIR views were used only for inspection.

The useful fact traced through optimization is the proven field displacement.
`Selector::index` folds constant displacements into effective addresses when
carrier lifetime permits; otherwise it materializes the address. The adjacent
load fold is limited to one nonvolatile use in the next instruction, with carrier
interval extension. These inherited local choices reduce address/load work
without alias, lifetime or FP reassociation assumptions; no code-growth/search
pass was added. Unknown or ineligible cases keep the ordinary load/address.
Selected instruction debug locations survive, and relocation plus unwind records
use the final owner. Final disassembly contains the expected offset-8 LEA/load
and offset-12 store; GOT controls verify scratch-dependent imported accesses.
The PA24 course properties in the prior-through report also pass.

Whole-pipeline bounds remain O(tokens + demanded facts/edges + output) except
recorded sorting at O(S log S). Object reachability visits each root/body once;
placement moves bytes once and fixes relocations once. Section/group and constant
limits, ownership/release points, fallback behavior and all four performance
dimensions are recorded in [performance148](../student.tests/pa27/performance148.md).
O0 is the present native policy; accepted O2 controls are compatibility checks.
PA32/PA33's later optimizer/debug requirements and PA34 self-hosting do not create
new PA27 exit gates. No required current-stage limit was reclassified.

## Reference preservation and performance acceptance

The only contract edits in the accumulated range are the four inspection files
in [overlay145](reference-corrections.md), replacing `_Z1g` with `g`. I verified
the two-source reducer, N3485 [basic.link] entity proof, the PA27 raw-host-name
contract and Itanium 5.1.2's global-namespace-variable rule in the checked-in ABI
text. Bundle revision `c2f713cd70d06170632bfde3e75dd6fe1aa44d98` and bundle hash
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7` remain pinned.
The repair changes no source fixture, relocation class, runtime/status oracle or
comparison rule. There are no further reference corrections in this audit.

There are **336 final new A/A + six-block ABBA observations**, and all **1960**
historical observations remain. Full-stage common executable text is unchanged
except required pruning of 30,816 bytes. The audit's storage objects/text are
byte-identical. Compiler RSS increases are bounded and disclosed; timing spreads,
including EH's 1.110 paired runtime ratio, are retained. Required COMDAT metadata
and the imported-RTTI GOT load explain the relevant code/object differences.
No precise speedup or unsupported profit claim is accepted. Inherited 15% and
zero-growth diagnostics remain diagnostics under spec §9, as their original
measurements already established; they neither override stage scope nor waive
correctness, required limits or coverage. Identified avoidable repeated projection
work is repaired, and no unprofitable optional transform is retained.

## Validation, ledger and remaining work

[Final validation](../student.tests/pa27/evidence148/validation.json) pins the
committed code tip, compiler SHA-256 and source hashes. Required prior-through:
**4283/4283**; file audit: **pass, four inherited header warnings**; PA27:
**157/158**, section controls **3/3**; through PA27: **4440/4441**. The exact entry
failure set is unchanged, not offset by additional passes. All **19,749** tracked
contract paths and **158** stage anchors are preserved; audit-entry-to-tip
fixture/reference/harness/comparison changes are empty. All **324** personal
command checks pass. Records are committed separately after the code tip, with
no subsequent code edits.

| Audit | Reviewed range / code tip | Findings, evidence and disposition |
|---|---|---|
| 148 | `f833cf1f..cbe7871e` (three handoffs, 16 commits) | Anonymous access/receiver and projection ownership repaired; complete range, reference overlay and stage-scoped performance accepted; 4283 prior tests, file audit, 324 personal commands and unchanged 1/158 stage failure prove checkpoint exit. |

Remaining work is the broad **hosted-library/extern-template integration** group.
[`200-host-extern-template-vtable-reference.t`](../student.tests/pa27/evidence148/remaining.json) fails at `__builtin_strcmp` in
`<typeinfo>` before the intended vtable behavior. The README's exclusion of
general hosted headers does not justify removing or waiving this checked-in
fixture. Resolve its prerequisites and emission behavior together, then pass the
full through-PA27 report before advancing. This record certifies the requested
checkpoint audit, not PA27 completion. No independent review question from the
three handoffs remains unresolved at this checkpoint.

Separating ELF from ABI naming was useful. Splitting storage into initial actions,
scope repair, reference-key repair and temporary-buffer repair fragmented one
ownership problem unnecessarily. Future work should validate the whole broad
receiver/lifetime/dependency group before another implementation handoff.
