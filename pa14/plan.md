# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement**; stage remains incomplete.
Stage entry **84/314**; previous checkpoint **281/314**; current **286/314**.
**202 original failures resolved; five this continuation; no lost passes or reduced coverage.**

## Design/spec alignment and remaining groups

Canonical template/type/argument identities feed ordinary semantics and typed
LowIR. Qualified dependent types retain structural paths. Indexed retained
out-of-class definitions own parameter overlays and narrow body/storage demand;
class defaults retain their declaring head's types. Source is parsed once.
[Ownership, data flow, complexity and validation](implementation.md) gives details.

| Remaining owner | Next coherent group |
| --- | --- |
| Symbolic expression/signature facts | Dependent `decltype`, trailing returns and bounds; parameter environments before concrete signatures; share fixed semantic facts. |
| Definition-time binding/control | Fixed lookup and base provenance; unused-body checks with real block/condition/jump scopes. |
| Inherited object/lifetime/ABI facts | Move/reference/empty transfers, local enum identities, constructor entries, virtual destruction and reentrant layout. |
| Parser/template contexts | Declaration-owned categories for remaining inherited calls and template forms. |

Dependent-only semantic checking, finer occurrence demand, typed dependency edges,
distinct monotonic fact states and narrow failure memoization remain PA14 spec
requirements. The completed definition-demand group was extended through explicit
instantiation, defaults, static addresses, elaborated types and alignment. The
next group requires symbolic expression/binding facts before concrete signatures;
more eager concrete lookup would violate the required design. This is the
concrete incomplete-checkpoint boundary. PA15 has not been started.

## Performance evidence

The [performance review](performance.md) preserves **1,456** timed processes
(1,148 historical + 308 new), frozen binaries and historical diagnostic misses. The new campaign
compares continuation entry/current code on common correct inputs with A/A and
ABBA; newly supported definitions are measured only on the working compiler.
Compiler wall/RSS, native runtime/size and work counters are recorded. O0 has no
optional optimizer or mandated numeric compiler threshold. Ownership, correctness,
coverage and mandated limits remain requirements; unsupported inherited diagnostic
gates are not stage exit gates. Common outputs/native binaries are byte-identical;
4× new definition inputs yield 4× applications, 4.27× wall and 3.61× RSS.
Compiler text grows 35,008 bytes this continuation; no optimization gain is claimed.

## Handoff ledger

Continuation at `e27474c2`: prior turn is **verified progress** (59 existing
failures resolved, committed code and checked evidence). Current group owner:
typed expression/signature facts. Bind parameter ordinals in declaration-owned
scopes, retain canonical operation/type/declaration edges, substitute dependent
edges and reuse completed fixed type queries. Work follows recipe edges and
required candidates. Validate trailing-return arithmetic/calls, reference-array
parameters, callable-reference `decltype`, then definition-time fixed binding.

Query increment: canonical typed recipe IDs retain bound names, parameter ordinals,
operations, type arguments and call edges. Queries share fixed facts and have
active/success/failure states; substitution visits dependent edges. Typed query
calls reuse deduction, standard conversions, ADL and ranking helpers. ABI queries
use the existing graph plus unresolved names. Through report **1907/1935**;
prior **1621/1621**, ten personal executables, six query/ABI checks and file audit
pass. Full expression legality/user conversions and fixed body binding continue;
this is not a complete type-query interpreter or a stage handoff.

Previous turn: **verified progress**, committed `4fafa38c`, revalidated 222/314.
Earlier increments: `2cec3424` function demand; `73409fcc` canonical class demand;
`45b15b80`/`c48def7d` calls/base provenance; `7e88952a` graph-read correction;
`e0eb788f` overloaded arguments; `4fafa38c` preserved evidence.

| Current increment | Commit / result |
| --- | --- |
| Typed dependent names and retained definitions | `885cefb1`: 264/314; 42 continuation failures fixed |
| Explicit class demand and defining scopes | `66e9e426`: 273/314 |
| Canonical default owners and member storage | `16e7c163`: 281/314; cumulative 59 continuation failures fixed |

Current through report: **1902/1935**, only PA14 fails; prior **1621/1621**.
Nine personal executables pass. **323** release/ASan/UBSan status/output checks
pass (rejection parity does not mean 323 course-correct programs). File audit
passes with three inherited header advisories. Sequential `make test-pa14` is
**281/314**; `make test-report-through-pa13` is **1621/1621**. All **1,456** timed
observations verify. Fixtures/references and review
markers are unchanged. Full current logs/frozen artifacts:
`$RALPH_ARTIFACT_DIR/pa14-dependent/`; earlier evidence remains under
`pa14-measurements/`.
