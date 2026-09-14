# PA17 compact plan — checkpoint audited, loop 48

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `58789b00e19ae2aff032c8b04980b7d43265617f`

Target: **PA17 full-stage**, incomplete. The first checkpoint audit covers the
entire stage-base→reviewed-tip range: thirteen accumulated commits through
`232f4a93`, plus audit fixes `ebb0e5b2` and `58789b00`. The prior implementation
turn made progress (242→248); this audit fixed ownership, identity and complexity
defects and added explicit regression controls. Detailed findings and the single
review ledger row are in [audit.md](audit.md).

PA17 remains **248/343**, with the **same 95 failures** as audit entry and no lost
passing cases. PA1–PA16 pass **2266/2266**. The through-PA17 report is
**2514/2609**, with every failure confined to PA17. Source file audit passes with
the same three inherited header-division warnings. All **115 personal controls**
pass, including **25 audit controls**. Fixtures, references and comparison rules
are unchanged. No reference correction or stage-advancement waiver was made.

The reviewed implementation retains canonical template arguments, structural
primary/partial owner shapes, member signatures, renamed source heads, defaults,
access obligations and separate declaration/body/storage demand. The audit fixed
crossed constructor ranking, alias/template declaration collisions, definition
access leaking from callers, and lost enclosing parameter identity in nested
heads. Alias and pack-match frames now use indexed tuples; full frame keys and
explicit alias fact states bound lookup and repeated demand. Source parsing and
typed LowIR construction remain shared; no output delegation or optional
optimization was added.

[Final performance evidence](audit-performance.md) includes cumulative and
checkpoint A/A+ABBA compiler timing/RSS and checked native runtime/text size.
Wide-head compilation improves 7–17% with less memory. Common cumulative partial
compilation costs 2.31% for required semantics; native outputs remain identical.
PA17/O0 has no mandated numerical ceiling. Inherited +15%, +16 MiB and 5.5×
targets remain diagnostics under spec.md, with every measurement preserved.
Evaluator limits and existing lowering growth policies remain intact; native
optimization, MIR and self-hosting belong to later stages.

| Remaining broad work | Owning implementation |
|---|---|
| Nested/member/alias declarations and late definitions | Retain all enclosing and nested heads in the source declaration/prototype graph, then compose selected-owner frames and precise definition demand. Includes remaining enclosing non-type/template-head substitution. |
| Friend templates, access and ADL | Publish explicit friend entity/namespace relationships and consume them in indexed lookup, hiding and access checks. |
| Definition-time dependent-name obligations | Preserve `typename`/`template` introducers and current-instantiation context in dormant and nested declarations/bodies. |
| Pack, variable-template and dependent lookup interactions | Complete argument/substitution and specialization ownership while retaining source scope and structural identity. |
| Required LowIR behavior and representation | Finish cleanup, scalar/constant storage, static initialization and emission ownership. |

All 95 failing cases remain required implementation work. Passing this checkpoint
audit does not complete the stage. Keep the next work grouped by these ownership
boundaries, with the shared head/frame/access interaction controls run together.
The three prior handoffs fragmented related access and environment work; this
audit exposed defects across those splits. Avoid repeating fixture-sized handoffs.

Reproduce evidence with `python3 student.tests/pa17/verify_audit.py`. The record
binds the reviewed source, unchanged contract, exact failure sets, required logs,
controls and frozen performance artifacts. The audit-record commit contains no
subsequent compiler edits, leaving the marker above as the next review baseline.
