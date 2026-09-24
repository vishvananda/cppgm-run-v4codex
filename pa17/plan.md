# PA17 compact plan — implementation handoff, loop 55

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `c43e8eb68db7e9b3f1dd0bbb18f14c92e4fc4b30`

Target: **PA17 full-stage**, still incomplete. Clean entry `4d1527e7` passed
321/343. Implementation `df97733f` passes **324/343**: three original failures
resolved, no new failures, all 343 course inputs unchanged. PA1–PA16 pass
**2266/2266**; **439/439 personal controls** pass (391 inherited, 48 new).
Entry fails 26 new controls. No reference, fixture, comparison or bundle changed.
Previous goal turn: progress, revalidated from committed fixes and the entry log.

| Completed owner / data flow | Design, bounds and validation |
|---|---|
| Qualified source type → current-instantiation identity → member/type-access fact | Preserve a dependent qualifier when an open base can supply its unknown member; known members retain definition-time lookup. Recheck known base members if a dependent base can introduce ambiguity. Indexed related scopes, immutable type paths, one access check per recipe/frame. Inherited aliases, references, nested templates, partial/current identities, out-of-class heads, access, hiding and required introducers execute or reject correctly. |
| Inline namespace opening → parser import edge → qualified template-id | Publish the implicit directive before parsing the body. Existing interned name/scope table; no new scan or replay. Nested/reopened/aliased namespaces, type/value template-ids, pack calls and local hiding are covered. |
| Source expansion → argument sequence substitution → canonical class application | A fixed-position expansion retains the unresolved suffix; reject known wrong argument kinds immediately. Expand before assigning fixed parameters/defaults and repacking the target tail. Work follows source arguments plus emitted lanes; existing complete frame/type keys cache results. Multiple fixed heads, empty tails, value/type/reference packs, nested/member and recursive applications are covered. |

[Performance evidence](qualified-performance.md) preserves frozen A/B flags,
inputs and binaries, all A/A and ABBA observations, a separate noisy-case repeat,
latency/RSS, checked runtime/text, scaling and a source-to-LowIR-to-ELF trace.
Repeated common compiler pairs are 0.985–1.003× entry, with at most +256 KiB
RSS; compiler text grows 1,024 bytes. New 4× inputs cost 3.56–4.20× latency.
Common executables are byte-identical. No optimizer was added. PA17/O0 has no mandated numerical ceiling; historical
+15%, +16 MiB and 5.5× values remain diagnostic under spec.md §9. Existing
constant-evaluation limits and lowering work/growth caps remain unchanged.

| Remaining implementation owner | Cases | Required next work |
|---|---:|---|
| Expression queries / candidate substitution | 3 | Recursive ADL constraint demands and ambiguous-operator expected failure propagation; preserve complete keys and separate active/failure states. |
| Lambda semantic entities | 1 | Closure identity, callable declarations and body/lifetime facts for chained member templates. |
| Static storage, definitions and initialization | 7 | Static member publication, local guards, static-vs-dynamic reference/address initialization and constant reads. |
| Array initialization effects / O0 representation | 2 | Retain required element evaluation and course initialization form. |
| Scalar conversion / transfer emission | 3 | Constant integer conversion and memberwise copy representation. |
| Object adjustment / empty value initialization | 1 | Consume complete base path and empty-object value plan. |
| Exception cleanup regions | 2 | Required call-region boundaries and cleanup scheduling. |

All **19 failures remain unfinished implementation**, separate from independent
review. The [handoff ledger](../student.tests/pa17/qualified-handoff.json) records
exact cases, owners, data flow, complexity, diagnostics and validation. Do not
advance to PA18. This boundary closes qualified type/name application, extended
through dependent-base ambiguity and fixed-position pack expansion. The three
remaining query failures require a different demand/failure protocol spanning
candidate substitution and recursive class selection; lambdas require a closure
entity owner. The fifteen LowIR mismatches already reach selected concrete
entities and require storage/initialization, transfer or cleanup policy. Extending
this increment into those owners would require separate semantic/lifetime
analysis and its own behavior/performance validation; none is waived.

Independent review remains required for this and both prior implementation
increments. Review current-instantiation access keys and ambiguity timing,
inline namespace visibility, symbolic argument-sequence identity/default timing,
pack frame composition and preserved rejection rules. Preserve the prior
[audit](audit.md), [selection](selection-performance.md),
[publication](publication-performance.md) evidence and their open review questions.

| Loop / phase | Handoff ledger |
|---|---|
| 52 / checkpointAudit | Reviewed through `c43e8eb6`; 306/343, 313 controls. |
| 53 / implement | 306→315, 344 controls; partial/argument/query facts; review pending. |
| 54 / implement | 315→321, 391 controls; publication/construction; review pending. |
| 55 / implement | `deb07c4e`, `df97733f`: 321→324, 439 controls; qualified lookup/application group; prior/file checks pass; 19 implementation failures and independent review remain. |

Run `python3 student.tests/pa17/verify_qualified.py` to verify this handoff.
The records commit follows the frozen implementation tip; review markers stay fixed.
