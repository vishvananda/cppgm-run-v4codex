# PA18 implementation handoff 77

Entry `2ce99c6a` (**385/420**). Implementation commits `86e142ba`, `b6294394`,
`975e6162`. This completes an implementation group, not the independent audit or
PA18. Stage/review markers in `plan.md` are unchanged; review of 75–77 is pending.
The prior turn was verified progress (handoff 76); no live work needed resuming.

## Completed ownership group

A nested member class now has a declaration identity, scope and source/context
before it has a definition. Instantiating its enclosing class publishes that
declaration and retains the definition recipe. Completeness demand performs the
definition once, with not-started/active/success/failure state on the class fact.
Recursive demand observes active state; failure cannot masquerade as a completed
or merely incomplete class. Definition side effects remain hard errors.

The parsed source region is shared. Its initial occurrence projection includes
only a nested class's header. Bases, attributes, members and further nested
classes get occurrences on definition demand; bodies/default arguments keep
their separate demand regions. This preserves source identity rather than
replaying grammar or copying a syntax/semantic tree. Member-template heads retain
their declaration processing; anonymous unions and ordinary function-local
classes preserve required eager checks. Fixed static assertions are checked
when binding the source, while dependent assertions await definition demand.

These boundaries follow N3485 §14.7.1 [temp.inst]/1–2,10 in
[the local standard](../doc/n3485.txt), lines 19603–19720. The scope was extended
through explicit instantiation and specialization: §14.7.2 [temp.explicit]/1,8–10
and §14.7.3 [temp.expl.spec]/4–6 (lines 19780 onward and 19948 onward).
Explicit outer instantiation traverses the nested member set. Direct nested
instantiation demands the same declared class; extern declarations remain dormant.
Explicit nested specialization selects the existing identity before completion,
including forward declarations, and invalidates only that class's query consumers.
Specializing after demand and duplicate explicit definitions still reject.

Inherited type lookup exposed a related immediate-context boundary. Indexed base
traversal now returns ambiguity as a compact result. Ordinary lookup diagnoses it;
qualified type substitution and member/value queries return candidate failure.
Class completion happens before that boundary, so an invalid definition is not
silently discarded. Partial-specialization matching also checks retained access
recipes on its argument list, including arguments erased by an alias result.
It never checks the candidate body just to validate argument access. These rules
follow §14.8.2 [temp.deduct]/8 (lines 20468–20477) and §14.5.5.1
[temp.class.spec.match]. No reference or comparison-rule correction was needed.

| Owner | Data flow and bound |
|---|---|
| `class_enum.cpp`, `nested_class.cpp` | Stable class/scope/source identity → one retained definition state → ordinary member facts. Work is linear in demanded members and language-required base edges. |
| `syntax/occurrence.cpp` | Cached source-region topology → compact header occurrences → demanded definition occurrences. No per-specialization walk through dormant bodies. |
| Explicit instantiation/specialization | Existing declaration → explicit demand/selection → the same class completion and member worklist. No replacement type identity. |
| `lookup.cpp`, `dependent_type.cpp`, `type_query.cpp` | Indexed lexical/base edges → entity/ambiguity → typed substitution result. Ordinary lookup retains hard diagnostics. |
| `class_pattern_selection.cpp` | Deduced canonical arguments + parent frame → retained source access checks → candidate viability. Cache inputs include source recipe and complete immutable substitution frame. |

Records use translation-unit vectors/flat indexes and compact IDs. Temporary
traversals are local. Source-region and completed member facts are reused; no
global retry, generation invalidation, rendered-name key or lowering reconstruction
is introduced. Lowering consumes the same ordinary facts after completion.

## Validation and evidence

- PA18: **388/420**, failures **35 → 32**, no new failures. Original repairs:
  `300-lazy-nested-class-in-member-template`,
  `300-lazy-nested-member-class-instantiation`, and
  `300-ambiguous-inherited-member-type-sfinae`.
- Earlier PAs: **2609/2609**. File audit passes with three inherited header
  advisories. All 420 inputs, 1,686 tracked fixture/reference files, and comparison
  rules are unchanged.
- **70/70** nested and **27/27** lookup controls, compared with **51/70** and
  **12/27** at entry. Every positive control validates LowIR and executes through
  the supplied native backend; negative controls require rejection. All **778**
  inherited semantic controls pass on the final binary: **875 total**.
- Eighteen region/demand controls use 32/128 classes and 4/64/256 nested members.
  Dormant use creates zero definitions and exactly 8 occurrences per enclosing
  specialization, independent of nested member count. Layout demands N definitions;
  repeated use demands exactly one. Three additional nested completion controls
  at 32/128/512 owners invalidate exactly one query after defining one nested class.
  Three inherited completion controls also pass.
- Frozen compiler latency/RSS and checked runtime/size observations are documented
  in [performance 77](performance77.md). The first batch and intermediate failed
  checks remain recorded, including repairs to lexical base scope and source versus
  occurrence IDs. Final measurements use `975e6162`.

Commands, hashes, manifests, results and remaining failure paths are retained in
[loop77 evidence](../student.tests/pa18/loop77-evidence.json). The personal harnesses
are explicit invocations; default course coverage was not changed.

## Boundary and unfinished work

The initial declaration split was extended through demanded layout, defaults,
constructors/destructors, explicit class instantiation/specialization, source-region
allocation, inherited member queries, candidate access and precise completion
invalidation. These consumers now have executable or rejection controls.

The remaining **six rejections** concern empty-pack unknown-bound-array lowering,
first-declaration function-result equivalence, explicit member specialization
deduction, expanded member-template arguments, function-template arguments found
through a using-directive, and alias/function-argument cv syntax. They require
different signature, syntax or lowering owners. Another **26 comparisons** concern
ordinary LowIR policy/facts, including earlier-stage pooling requirements. Further
changes to nested completion or substitution's ambiguity boundary cannot repair
those cases without replacing the owning mechanism. The pending nested-alias cast,
class-ellipsis and inherited empty-tag/root-metadata work remain implementation
defects, not review questions or claimed passes.

Independent review should check declared/defined class identity, deferred source
boundaries, access-frame validity, explicit demand and query invalidation, plus
interactions with handoffs 75–76's constructor/lifetime facts. This handoff does
not waive that review or the 32 course failures. Do not advance until the through-
PA18 report passes and the whole-stage independent audit resolves its findings.
