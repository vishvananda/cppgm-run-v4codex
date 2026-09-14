# PA17 compact plan — implementation handoff, loop 50

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `58789b00e19ae2aff032c8b04980b7d43265617f`

Target: **PA17 full-stage**, incomplete. Clean entry was
`d3ef5e5d5adc89baf13f9697e5c38b165bba0dcc`: **284/343**. Implementation through
`a8cc241e67cec028d8e56cd65fe600876968c9ca` passes **301/343**: **17 original
failures fixed**, no new failures or reduced coverage. PA1–PA16 pass
**2266/2266**; through PA17 is **2567/2609**. File audit passes with the same
three inherited header warnings. All **205 personal controls** pass, including
47 new friend/access controls and four inherited LowIR ownership controls.
Tests, references, comparison rules and mandated limits are unchanged.

The completed group owns friend declaration identity, specialization access,
hidden lookup and friend body demand. It expanded from friend templates to
qualified member/namespace friends, existing function-template specializations,
non-template friends of class templates, constexpr friend evaluation, duplicate
candidate identity, and stable access for member class/variable specializations.

| Owner and data flow | Complexity / validation |
|---|---|
| Friend/entity graph: source class and independent head → namespace/qualified template → explicit friendship edge | Canonical typed signatures and parameter slices; expected O(head width), indexed declaration lookup. Renamed/qualified heads, namespace identity, explicit specializations, hidden visibility, access isolation and invalid redeclaration controls. |
| Access: declaring entity and friendship → selected specialization and lexical context | O(lexical/base edges); access comes from the template declaration, never the enclosing class's final access label. Private/protected, nested/friend, non-transitive/non-inherited and positive/negative member specialization controls. |
| Demand: retained ordinary friend → indexed entity demand → checked body → ordinary LowIR | One queue insertion and body computation per demanded entity; source binding is separate from instantiation. Dormant invalid bodies, constexpr evaluation, multi-specialization and native operator-chain controls. |
| Calls/operators/queries: ordinary and associated declarations → concrete candidate identity → conversions/selection | Call-local flat deduplication before conversions, no global marks; allocate only for multiple declarations. Native and decltype duplicate-specialization controls. |

No grammar replay, alternate semantic graph, output delegation or optional
optimization was introduced. Shared heads and TU-owned edges feed ordinary
substitution and typed lowering. Source friend-template checks wait for their
complete class; out-of-class bodies retain their function access context.

[Performance evidence](friend-performance.md) freezes entry/final binaries,
flags and inputs; includes A/A+ABBA latency/RSS, native runtime/text size and
all raw observations. Common LowIR/executable hashes match. Common frontend
paired medians span −6.63% to +0.87%, with disclosed timing outliers; compiler
text grows 4,928 bytes (0.28%). New-only friend-access inputs record entry
rejection and 4× demand scaling (4.22× time, 3.68× RSS), with exactly linear
candidate/substitution work. These are semantic cost observations, not an
optimization claim. PA17/O0 has no mandated numerical ceiling. Historical
[head](head-performance.md) and [audit](audit-performance.md) measurements and
their diagnostic +15%, +16 MiB and 5.5× targets remain intact under spec.md.
Evaluator limits and existing lowering growth policies are unchanged.

| Remaining implementation group | Cases | Next owning decision |
|---|---:|---|
| Dependent `typename` / `template` obligations | 5 | Check source introducers against current/noncurrent-instantiation identity. |
| Alias/variable/member specialization and arguments | 7 | Complete partial/explicit selection, cv and empty/value-pack substitution. |
| Qualified grammar, lookup, deduction and queries | 15 | Resolve syntax and dependent candidates/query recursion at their required point. |
| Required LowIR differences | 15 | Complete storage/initialization, transfer, cleanup and emission behavior. |

The remaining reentrant ADL fixture now belongs to query/substitution: its
failure is `recursive type query` while checking a constrained candidate, not
friend publication or access. The remaining hidden-friend LowIR fixture emits
its friend bodies but still differs in static-member storage and empty-object
zero-initialization. Those require query failure-state and storage/value-action
algorithms; more friendship edges or head overlays cannot supply them. This
is the concrete handoff boundary after completing related friend/access work.
All **42 failures remain required implementation**, not audit questions or
waivers. Exact cases/diagnostics and owner/data-flow/complexity/validation are in
[handoff evidence](../student.tests/pa17/handoff.json).

| Handoff ledger | State / evidence |
|---|---|
| Previous goal turn | Progress: retained-head implementation and validated loop 49 evidence, preserved in `student.tests/pa17/handoff-loop49.json`. |
| Loop 50 friend/access group | Complete implementation increment in `4b821120` and `a8cc241e`, followed by evidence. Required reports and all controls bind to source/binary hashes. |
| Remaining implementation | 42 failures above; no stage advancement or waiver. |
| Independent review | Pending cumulative canonical-head/class-identity cache review from loop 49, plus friend scope/head identity, source-to-concrete definition ownership and access/demand validity across specialization. Implementation evidence does not replace audit. |

Reproduce evidence checks with `python3 student.tests/pa17/verify_handoff.py`.
The prior checkpoint audit remains in [audit.md](audit.md). Review markers above
are preserved. This returns control to Ralph; it does not certify the stage.
