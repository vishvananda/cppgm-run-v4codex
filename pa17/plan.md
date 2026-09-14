# PA17 compact plan — implementation in progress, loop 50

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `58789b00e19ae2aff032c8b04980b7d43265617f`

Loop 50 entry: `d3ef5e5d5adc89baf13f9697e5c38b165bba0dcc`, clean,
284/343 (59 failures). The previous turn produced a validated retained-head
increment (progress); these remaining failures are implementation work.
Current group: friend declarations and specialization access/ADL. Owner:
semantic friend/entity graph. Data flow: retained class/head → canonical
namespace or member template → explicit friendship edge → specialization
access and associated lookup → ordinary demand/LowIR. Work must follow head
width, lexical/associated edges and demanded bodies, with no global retries.
Validate positive and negative friendship, hidden lookup, specialization
isolation and deferred body controls, required course reports, file audit and
frozen entry/final latency/RSS and executable observations. The loop 49 record
below remains historical until this increment's validated handoff.

Target: **PA17 full-stage**, incomplete. Entry was clean at
`df0904deaa9a21544ab4c10f3a9e68120740d982`: **248/343**. Implementation through
`807e0989cc2e83f7dfa8804e9bb4db471ea35af9` passes **284/343**: **36 original
failures fixed**, no new failures or reduced coverage. PA1–PA16 pass
**2266/2266**; through PA17 is **2550/2609**. File audit passes with the same
three inherited header warnings. All **158 personal controls** pass, including
43 new head controls and four LowIR ownership controls. Tests, references,
comparison rules and mandated limits are unchanged; no reference correction.

The completed group owns retained member/nested class template heads and their
definitions. Source declarations retain canonical identity before layout.
Renamed heads and overload roles select the declared prototype; definition
requests compose immutable parameter slices with selected class argument tuples.
Late definitions attach to earlier selections. Inner aliases/defaults, nested
constructors, template conversion and enclosing packs consume those identities.
Callable signatures expand enclosing packs before inner deduction; body
parameters expand once under the final frame. Source class template-ids bind to
their template even inside a different specialization of that class.

| Owner and data flow | Complexity / validation |
|---|---|
| Source binding and definition index: parsed heads → owner paths/prototypes → selected definition | Work follows source head widths, required owner depth and selected definitions. Renaming, partial owners, nested classes, three heads, duplicate/mismatch rejection and late demand controls. |
| Substitution and body demand: typed arguments + source slices → shared frames → declarations/body → ordinary LowIR | Cache by full frame/type identity; share tuples, avoid copied visible environments and global retries. Defaults, outer/inner packs, sibling specialization and native constructor controls. |
| Receiver syntax and constructor selection: declaration scope / argument types → member-template parse / concrete candidate | Cache completed expression subtrees; visit each receiver node once plus required name lookup. Deduce before converting-constructor ranking; preserve ordinary/template ties. |

No grammar replay, alternate semantic tree, output delegation or optional
optimization was introduced. PA17 continues O0 LowIR; own native backend,
optimization profitability and self-hosting remain later-stage responsibilities.
[Performance evidence](head-performance.md) freezes entry/final binaries and
inputs, checks common output equality, and records A/A+ABBA compiler latency/RSS
and native runtime/text size. New-only inputs record the entry rejection and
final cost. All observations, including the superseded pre-pack campaign, are
retained. PA17/O0 has no mandated numerical ceiling. Inherited +15%, +16 MiB and
5.5× targets remain diagnostics under spec.md; historical measurements in
[audit-performance.md](audit-performance.md) remain intact. Evaluator limits
and existing lowering growth policies are preserved.

| Remaining implementation group | Cases | Next owning decision |
|---|---:|---|
| Friend entities, access and ADL | 16 | Publish namespace/class relationships; consume them in access and associated lookup. |
| Dependent `typename` / `template` obligations | 5 | Check source introducers using current/noncurrent-instantiation identity. |
| Alias/variable/member specialization and arguments | 8 | Complete partial/explicit candidate selection, cv and empty/value-pack substitution. |
| Qualified grammar, lookup, deduction and queries | 14 | Resolve syntax and dependent candidates/query dependencies at their required point. |
| Required LowIR differences | 16 | Complete storage/initialization, transfer, cleanup and emission behavior. |

The group expanded through nested class definitions, three-head definitions,
partial-owner definitions, inner defaults, converting constructors and enclosing
pack signatures. Further fixes require the algorithms above: for example a
nested member-class partial must register/select its partial argument pattern;
additional definition-head overlays cannot make that selection. These **59
failures remain required implementation**, not independent-audit questions.
The exact cases, current diagnostics, owner/data-flow/complexity/validation
records are in [handoff evidence](../student.tests/pa17/handoff.json).

| Handoff ledger | State / evidence |
|---|---|
| Loop 49 retained-head behavior group | Complete implementation increment; four commits after entry, followed by this evidence record. Required reports and 158 controls are bound to source/binary hashes. |
| Remaining implementation | 59 failures above, no waiver. No stage advancement. |
| Independent review | Pending: assess canonical-head, source-context and class-identity cache validity across the cumulative stage diff, including late and partial-owner definitions. Existing controls support implementation; they do not replace audit. |

Reproduce the evidence check with `python3 student.tests/pa17/verify_handoff.py`.
The prior checkpoint audit remains in [audit.md](audit.md). Review markers above
are preserved; this handoff returns control to Ralph and does not certify PA17.
