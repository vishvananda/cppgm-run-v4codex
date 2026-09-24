# PA18 implementation plan

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `3a883d10a27e41d1b126eaef05eaf0b454de1646`.
Target: **PA18 full-stage**, still unfinished. Checkpoint audit 66 reviewed the
entire stage-base→code-tip range, including all three implementation handoffs.
Previous goal turn: **progress** (loop 65 implemented conversion/head behavior).
This audit: **progress** (fixed query prerequisite validity and removed semantic
evaluation from ordering inspection; validated the full accumulated range).

Stage entry **266/420** → handoffs 63 **282/420**, 64 **312/420**, 65 **327/420**.
Audit 66: **327/420**, the same **93 failures**, no new failures or lost coverage.
Earlier PAs pass **2609/2609**. File audit passes with three inherited advisories.
[Audit record](audit.md) contains the review, evidence, and checkpoint ledger.

| Ownership group | Reviewed implementation / remaining work |
|---|---|
| Deduction, ordering and conversion selection | Typed directional partial ordering, nominated call/address/conversion types, prototype pack element identities, overload selection and inherited conversion hiding are reviewed. Scratch bindings belong to the comparison; completed facts use canonical identities. Selected declarations/conversions feed ordinary demand and typed LowIR. |
| Substitution, queries and demand | Explicit/deduced immediate probing, structured failure, class→query→consumer edges and retained member heads are reviewed. Failed aliases/signatures now retain the prerequisite's local revision, so another consumer's recomputation cannot hide completion. Ordering inspects retained query identities without evaluation. No global invalidation or speculative body demand. |
| **Remaining: retained contexts, packs and expression validity** | Finish correlated outer/inner pack lanes, nested/member result/default and out-of-class ownership, compound assignment, braced construction, destructor, cast/access and selected-conversion checks. Extend the appropriate typed facts and failure owners; do not broadly catch exceptions. |
| **Remaining: constructor/explicit deduction and NTTP identities** | Finish braced/explicit template arguments, ADL template-id participation, constructor/inherited-template participation, and pointer/reference/static-member non-type values. Preserve lexical identity and demand boundaries. |
| **Remaining: LowIR initialization and result facts** | Resolve constant/array initialization, bool/result metadata and class-result calling conventions. Existing native success does not waive the course LowIR comparison. |

The remaining **69 status failures and 24 LowIR mismatches** remain required
implementation. Group future work by these semantic/lowering owners and validate
across their interactions before handing off. The three prior handoffs were useful
progress but fragmented the dependent default/query/member context path; repeated
full benchmark/report records and deferred cross-owner review were avoidable.

Spec scope: **O0 LowIR**. Canonical TU facts and immutable frames; scratch released
on return; work follows required candidates/type/query/lexical/dependency edges.
No optional optimizer or native backend was added. [Audit performance evidence](performance66.md)
compares the frozen stage base and checkpoint entry with the reviewed compiler;
all earlier [63](performance.md), [64](performance64.md), and [65](performance65.md)
measurements remain preserved. PA18 mandates no numerical latency/RSS/runtime
ceiling. Inherited PA17 self-selected targets remain diagnostics under spec §9;
required work bounds, correctness and coverage remain gates. Native optimization
and self-hosting performance belong to PA24–PA34.

The sole stage reference change is the [constant-initialization correction](reference-correction65.md),
independently rechecked in audit 66 against its reducer and N3485 proof. No other
fixture, success status, bundle or comparison rule changed.

Next validation: focused required fixtures and explicit personal controls,
`make test-pa18`, `make test-report-through-pa17`, and the PA18 file audit.
Run root report targets sequentially because they share `.test_counts`.
**Do not advance to PA19 until `make test-report-through-pa18` passes.**

Loop 67 implementation entry: `06211ad0438df250952a414eef409d657b0ff5b0`,
**327/420** (93 failures). Previous goal turn classified **progress**: audit 66
changed prerequisite validity and established authoritative validation evidence.
Current group: pointer/reference/function NTTP identity and target conversion.
Owner/data flow: retained typed argument query → parameter-directed conversion
and C++11 address validation → canonical constant storage/entity identity →
substitution frame → ordinary expression/demand/typed ABI and LowIR consumers.
Work is proportional to the argument expression and required overload candidates;
canonical query/storage lookups are O(1) average, TU-owned, with local conversion
scratch. Validate nulls, qualification, linkage, array/function decay, static
members, overload sets, nested forwarding, rejection/SFINAE and emission. Extend
related conversion/identity defects before handoff; independent review remains
separate from unfinished implementation. Frozen entry: `/tmp/pa18-loop67/entry`.
