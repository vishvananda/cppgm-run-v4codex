# PA17 compact plan — implementation handoff, loop 54

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `c43e8eb68db7e9b3f1dd0bbb18f14c92e4fc4b30`

Target: **PA17 full-stage**, still incomplete. Entry `3f1f6f0b` was clean at
315/343 (28 failures). Code `5c5756b7` passes **321/343**: six original failures
resolved, no new failures, all 343 course inputs unchanged. PA1–PA16 pass
**2266/2266**. **391/391 personal controls** pass (344 inherited, 47 new);
entry fails 31 of the new controls. No reference, fixture, comparison-rule or
bundle correction was used.

| Completed owner / data flow | Design, bound and validation |
|---|---|
| Anonymous class declarations → scope/storage identities → member access/layout | Explicit injection edges participate before bases; enclosing access checks and recorded base adjustments survive nested injection. One edge per anonymous class, indexed names, visits only related scopes. Alignment, nesting, cv, access, base hiding and second-base offsets execute. |
| Parsed constructor/conversion declaration → retained head → specialization member facts | Conditional explicit retains a canonical query only while dependent; fixed results are copied, renamed heads rebind parameter identities. Specialization computes the condition before constructor filtering. Parsing no longer misclassifies a conversion type as a member callable; out-of-class conversion types retain the template environment. Constant/contextual bool, packs, renamed/nested definitions and copy/direct rejection controls pass. |
| Array functional construction → ordinary list plan → concrete pack lanes and temporary lifetime | Class and array braces share the list owner. Source recipes project once; already concrete pack lanes retain their substitution frame. No token replay or replacement trees. Values, narrowing, zero tails, side effects and class-element cleanup execute. |
| Selected union aggregate zero plan → bounded LowIR stores | Preserve scalar zeroing for a small selected aggregate region using the existing eight-store cap; large regions retain bulk fallback. Direct typed emission consumes semantic sizes/alignment. The required anonymous-storage comparison and fallback controls pass. |

[Performance evidence](publication-performance.md) preserves frozen compiler
latency/RSS, A/A and ABBA observations, the separate recheck of noisy cases,
checked runtime/text and scaling. New workloads scale 4.10–4.21× for 4× input;
compiler text grows 3,776 bytes (0.213%). A changed union-zero executable adds
eight text bytes and its measured runtime falls about 80%. The exact common
executables are unchanged; their timing variation supports no speedup claim.
PA17/O0 has no mandated numerical ceiling; inherited +15%, +16 MiB and 5.5×
targets remain diagnostic under spec.md §9. All prior measurements, constant
limits and initialization expansion/fallback limits remain preserved.

| Remaining implementation group | Cases | Next owning work |
|---|---:|---|
| Qualified lookup, candidate queries and remaining syntax | 7 | Recursive/ambiguous query states, current-instantiation inherited/local type lookup, lambda and qualified-pack calls. |
| Required LowIR storage, initialization, transfer and cleanup | 15 | Static/local storage and emission, constant initializer effects, scalar conversions, transfer representation and exception cleanup regions. |

All **22 failures are unfinished implementation**, separate from independent
review. Exact cases, diagnostics and owner/data-flow records are in the
[handoff ledger](../student.tests/pa17/publication-handoff.json). Do not advance
to PA18. The handoff boundary closes anonymous publication and constructor facts,
extended through nested/inherited access, renamed specifiers, array materialization
and pack-lane preservation. The remaining failures require different owners:
query recursion/failure scheduling and source lookup; static-storage policy;
constant-array effect retention; transfer emission; or cleanup-region scheduling.
Those cases already select their constructors/members or never reach this
construction owner. Continuing them requires separate semantic/lifetime analysis
and validation beyond this completed publication and materialization group.

Independent review remains required for this increment and the prior accepted
increment: canonical query keys and negative validity; anonymous direct-lookup
edges/access/base adjustments; renamed conditional-specifier facts; retained
pack-lane ownership; zeroing legality, bounds and performance. The prior
[checkpoint audit](audit.md), review marker, and historical handoffs/measurements
are preserved. These questions waive none of the remaining implementation work.

| Loop / phase | Handoff ledger |
|---|---|
| 52 / checkpointAudit | Review plus ownership fixes complete through `c43e8eb6`; 306/343 and 313 controls. Review findings and measurements preserved. |
| 53 / implement | Partial/argument ownership extended through query/condition facts: 306→315, 344 controls, prior/file checks pass; independent review pending. |
| 54 / implement | Previous turn classified as progress from committed fixes/check evidence. `5c5756b7` closes publication/construction group: 315→321, 391 controls; required validation and performance evidence recorded. Remaining implementation: 22 cases; independent review pending. |

Run `python3 student.tests/pa17/verify_publication.py` to verify this handoff.
The records commit follows the validated implementation tip and preserves the
review marker. Historical verifiers describe their own frozen code tips.
