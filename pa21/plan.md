# PA21 implementation plan

Stage base commit: `ac988ea33d4997b44e82baaca5a86623fff3127a`.
Last reviewed commit: `ac988ea33d4997b44e82baaca5a86623fff3127a`.
Target: PA21 full-stage. Phase: implementation handoff (loop 104).
Entry: clean `fa079cdeffb19a1a69e92ff56f8bc154c4047c36`, 49/116 pass,
67 fail. Now: **71/116 pass, 45 fail**; 22 original failures resolved, no new
failures or coverage reduction. Earlier PAs: **3596/3596**. Independent
whole-stage audit and the unfinished implementation below remain required.

## Design/spec alignment

RTTI/casts (loop 102): typed formation/evaluation facts feed cached RTTI/ABI
identities; lowering emits once per identity. [Ownership and bounds](rtti102.md).

Closures (loop 103): indexed capture edges retain source type, mode, forwarding
field and checked conversion/destructor dependencies. Construction and generated
copies record completed subobjects for reverse unwind cleanup. Template/query cv
facts do not create storage demand. [Ownership and bounds](captures103.md).

Lists (loop 104): initialization owns canonical library type/field identities,
immutable element conversion plans and backing storage. Selection/deduction,
constexpr evaluation and range lowering consume those facts. Local/reference,
nested and static retention reuse the lifetime owner; class elements construct
in their final slots. Mandatory inline body checking is separate from emission
demand. [Data flow, complexity, validation and boundary](lists104.md).

## Remaining implementation groups

| Owner | Required data flow and bounds | Remaining validation |
|---|---|---|
| EH regions and list-lifetime integration | Construction states feed full-expression/lexical regions; region exits and continuation keys must carry complete handler context. Reuse completed element addresses and bound prefix work. | Two list fixtures still differ in required LowIR shape despite native lifetime controls passing. |
| Source EH, lifetime control flow and generated special members | Typed throw/exception object/catch binding facts feed matching, rethrow and handler exit. Share only identical complete cleanup contexts. Work follows actual control/dependency edges. | 43 inherited failures: source handlers (including two lambda compositions), subobject failure, conditional temporaries, destructor termination, argument/return ownership and local-class/recursive-template cleanup. |

Handoff boundary: list formation, two-phase selection, deduction, scalar storage,
ranges and class-element constructor selection are complete. Work extended through
constexpr/nested backing storage, template recipes, static finalization, partial
construction and checking/emission separation. The two remaining list fixtures
need the shared EH region/continuation owner, also required by the 43 other
failures. Source `try` checking still lacks handler/exception-object facts.
Further list ranking/storage changes cannot supply those states; fixture-specific
cleanup shaping would duplicate that owner. These are **unfinished requirements**,
not independent-review questions, and native success does not waive comparison.

Independent review questions: list/query key completeness, dependent recipe reuse,
nested storage ownership, constructor checking/emission separation, and the
inherited RTTI/capture facts. Review is still owed; no requirement is waived.

## Performance evidence

[Protocol/results](performance104.md) and [raw observations](../student.tests/pa21/performance104.json).
Frozen A/A + ABBA: all six equivalent inherited workloads have identical LowIR
and native executables. Compiler text +20,800 B (0.97%); heavy-template RSS
+168 KiB. No speedup claimed. 800→3200 list specializations yield exactly 4×
plans/objects, one library representation, 36,805→147,205 instructions,
125→499 ms compilation and 25,288→82,656 KiB RSS. Explicit class-list growth
is seven instructions per element; array cleanup uses the inherited eight-element
expansion cap then a loop. Checked live list, calls, memory, floating-point and
capture loops include separate runtime/payload measurements.
PA21/O0 uses spec §9 bounds, with no optional optimization or extra numerical
exit gate. Native size is explicitly a sectionless-ELF payload proxy. Historical
self-selected ratios remain diagnostics; all earlier measurements are preserved
in [loop 102](performance102.md) and [loop 103](performance103.md). Own native
optimization/self-hosting retain their later-stage owners.

## Handoff ledger

Loop 102: `30fe6353` proves/corrects one oracle substitution digit; `9f2181f9`
implements RTTI/casts; `f4224e0b` records 37/116 and prior-through 3596/3596.
[Validation](../student.tests/pa21/validation102.json).

Loop 103: `e835d6dc` implements captures and construction lifetimes;
`1c541f84` completes generated-copy unwind cleanup; `fa079cde` records 49/116,
prior-through 3596/3596 and unchanged coverage.
[Validation](../student.tests/pa21/validation103.json).

Loop 104: `e2af8963` implements list selection, storage, deduction and lifetimes;
`f03b9371` removes backing-helper dependencies and adds measurements;
`511fe9b9` separates checking from inline emission demand.
[Validation](../student.tests/pa21/validation104.json): `make test-pa21` 71/116
(exit 2); prior-through 3596/3596 (exit 0); through-PA21 3667/3712 (exit 2);
file audit passes with three inherited header advisories. Personal controls:
51/51 semantic/rejection/native controls and 25/25 host-unwind/static controls.
All 116 required inputs, references and comparison rules are unchanged.
Review markers remain unchanged. This completes the implementation handoff;
PA21 completion and its independent whole-stage audit are still outstanding.
