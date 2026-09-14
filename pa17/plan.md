# PA17 compact plan — checkpoint audit, loop 52

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `c43e8eb68db7e9b3f1dd0bbb18f14c92e4fc4b30`

Target: **PA17 full-stage**, implementation incomplete. The accumulated review
covers `58789b00..c43e8eb6`: all three accepted handoffs, their interactions and
two audit fixes (**15 commits; 44 combined implementation paths, 45 touched**).
Entry `b34c1be9` and reviewed code both pass **306/343**, with exactly the same
**37 failures**. PA1–PA16 pass **2266/2266**; through PA17 is **2572/2609**.
The file audit passes with three inherited header-division warnings. All
**313 personal controls** pass: 261 inherited and 52 audit controls. Entry fails
17 of the new controls. Course tests, references, comparison and coverage are
unchanged; no reference correction or waiver was used.

[Audit findings](audit.md) and the [complete range](../student.tests/pa17/checkpoint52-range.json)
cover member-head/class identity, friend ownership/access/demand, current
instantiation, source ambiguity publication and access versus lookup context.
The audit fixed missing ordinary-friend use demand, dormant fixed qualified
friend binding, forbidden out-of-class template defaults, and repeated/misowned
source parameter-name checks. Evaluated function expressions share the existing
selected-function demand owner; conversion consumers keep their recorded facts.
The second fix closes discarded/boolean/void/comma uses and removes the initial
extra checks from general conversions.

| Owner / data flow | Bound and evidence |
|---|---|
| Source heads and canonical specialization selection → member declaration → parent-linked substitution frames | Compact entity/argument/source keys; relevant head/owner edges only. Renamed, nested, pack, partial, default, access and late-definition controls pass. |
| Friend binding and grants → selected function use → indexed body queue | Fixed references checked at definition; dependent recipes retained. Each ordinary friend body shares active/success/failure states across runtime and constant demand. |
| Source introducers and declaration interpretation → shared source/projected views | One source ambiguity interpretation before fact/region publication; no grammar replay or subtree clone. Current/noncurrent types, calls and parenthesized declarations retain behavior. |
| Lexical parameter-name checks → completed source obligation | Each nested template is checked under its complete head; projected declarations reuse the source check. Counters and shadowing controls verify the corrected boundary. |
| Selected entities, conversions, constants and lifetimes → typed LowIR → explicit supplied-backend validation | Ordinary entity-indexed emission; no semantic reconstruction or internal text transport. Own native backend and self-hosting remain later-stage work. |

[Performance evidence](checkpoint52-performance.md) reports frozen checkpoint
and cumulative A/B inputs/binaries, A/A and ABBA observations, compiler
latency/RSS, checked native runtime/text and source-work counters. Intermediate
campaigns and earlier [head](head-performance.md), [friend](friend-performance.md),
[name](name-performance.md) and [audit](audit-performance.md) measurements are
preserved. PA17/O0 has no mandated numerical ceiling. Inherited +15%, +16 MiB
and 5.5× values remain diagnostic targets under spec.md §9. Existing evaluator
limits and lowering work/growth fallbacks are unchanged; no optional optimizer,
ABI/debug relaxation or new exit gate was added.

| Remaining implementation group | Cases | Owning work |
|---|---:|---|
| Specialization entities and arguments | 7 | Alias/variable/member partial or explicit selection, cv and empty/value packs. |
| Qualified lookup and expression queries | 14 | Candidate/query failure states, dependent candidates and remaining syntax. |
| Required LowIR behavior | 16 | Storage/initialization, transfers, cleanup and emission. |

All **37 failures remain required implementation**, not review questions or
performance waivers. Exact cases remain in the
[audit evidence](../student.tests/pa17/checkpoint52-evidence.json), preserving
[loop 51's handoff](../student.tests/pa17/handoff.json). Do not advance to PA18.
Avoidable handoff fragmentation separated member heads, friend demand and source
checks across three checkpoints. Close each remaining broad owner with its
substitution, demand and lowering interactions before another handoff.

| Loop / phase | Disposition and evidence |
|---|---|
| 52 / checkpointAudit | Previous turn was progress (301→306). Full accumulated review and ownership fixes complete through `c43e8eb6`; earlier tests and file audit pass; same 37 PA17 failures; 313 controls pass. Three broad implementation groups remain. |

Run `python3 student.tests/pa17/verify_checkpoint52.py` to verify current evidence.
The records commit changes only audit records; the marker names the reviewed
code tip. Earlier handoff verifiers describe their frozen historical tips.
