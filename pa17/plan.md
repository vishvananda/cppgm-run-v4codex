# PA17 compact plan — implementation handoff, loop 51

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `58789b00e19ae2aff032c8b04980b7d43265617f`

Target: **PA17 full-stage**, incomplete. Entry
`0fe13f160c921db3ca68bfefc857b3a6be40cf3f` passed **301/343**. Source through
`6afed48ec5ff231ef85df0d8c5b7980f50ef6500` passes **306/343**: **five original
failures fixed**, none introduced, with unchanged coverage and comparison.
PA1–PA16 pass **2266/2266**; through PA17 is **2572/2609**. File audit passes
with the same three inherited header warnings. All **261 personal controls**
pass: 205 inherited and 56 new (36 native, 20 rejection). The frozen entry fails
19 of the new controls. Tests, references and mandated limits are unchanged.

The completed group owns source dependent-name obligations and their
current-instantiation exceptions. It expanded into qualified calls and receiver
categories, nested/alias/partial identities, renamed ordinary and pack heads,
non-type parameter equivalence, leading return versus parameter/trailing scopes,
member access, rooted class categories, varargs and declaration ambiguity.

| Owner / data flow | Complexity and validation |
|---|---|
| Retained type/name binding: source introducers + canonical qualifier → required checks → shared type/access facts | O(name parts + lexical owner edges); indexed member lookup. Dormant and demanded missing-introducer cases, current/noncurrent owners, aliases, fixed/dependent bases and qualified calls. |
| Current-instantiation facts: class identity + immutable source parameter slice → injected type | Expected O(1) completed lookup; head/partial-shape work once per key. Renamed heads, partial packs and direct-value alias equivalence controls. |
| Declaration interpretation: parser ambiguity flag → source semantic decision → existing node views → substitution/lowering | O(flagged declarations), one three-ID record plus six flat role entries per resolved source. No grammar replay or cloned subtree. Native direct-initialization, function declaration, elaborated type and function-pointer controls. |
| Definition access: member access context + source lookup position → retained access recipe | Relevant lexical/base edges only; earlier instantiation-access controls and private return-type controls pass. |

The shared source graph, TU-owned facts and ordinary typed LowIR path remain
the production pipeline. Source binding establishes declaration interpretations
before region/fact publication. Cache keys use compact identity; no rendered
names, global invalidation, output delegation or optional optimization is added.
[Implementation evidence](name-implementation.md) records the C++11 clauses,
source-to-LowIR trace, owners, validity boundaries and controls.

[Performance evidence](name-performance.md) freezes entry/final binaries, flags
and inputs, with A/A and ABBA compiler latency/RSS, native runtime/text size,
work counters and all observations. Common LowIR/executable hashes match;
compiler paired medians range from −2.68% to +1.25%, with all spread/outliers
disclosed. New demand scales 4× input to 4.14× time and 3.38× RSS, with constant
source binding work. Both intermediate campaigns are preserved.
The declaration flag avoids ordinary-declaration inspection; sharing the view
fallback removes 57,408 bytes of avoidable compiler text. Final compiler text is
23,040 bytes (1.32%) above entry. PA17/O0 has no mandated numerical ceiling;
these are semantic cost observations. Existing evaluator and lowering work/growth
limits remain unchanged. Historical [friend](friend-performance.md),
[head](head-performance.md) and [audit](audit-performance.md) measurements and
their diagnostic +15%, +16 MiB and 5.5× targets are preserved under spec.md.

| Remaining implementation group | Cases | Next owning decision |
|---|---:|---|
| Alias/variable/member specialization and arguments | 7 | Partial/explicit selection, cv and empty/value-pack substitution. |
| Qualified lookup, deduction and expression queries | 14 | Candidate/query failure states, dependent candidates and remaining syntax. |
| Required LowIR differences | 16 | Storage/initialization, transfer, cleanup and emission. |

All **37 failures remain required implementation**, not independent-review
questions or waivers. The rooted static-member fixture now compiles and reaches
a LowIR storage-initialization difference. The remaining reentrant ADL fixture
still fails in recursive type queries. These require selection, query or
lowering algorithms; further source-introducer checks and current-owner facts
cannot supply them. This is the concrete boundary after completing the related
source-name behavior and its interactions. Exact diagnostics and ownership are
in [handoff.json](../student.tests/pa17/handoff.json).

| Handoff ledger | State / evidence |
|---|---|
| Previous goal turn | Progress: loop 50 friend/access implementation and evidence preserved in `student.tests/pa17/handoff-loop50.json`. |
| Loop 51 implementation | Source-name group in `3ce88dd7`; focused ambiguity dispatch in `d7f0055d`; shared view fallback in `6afed48e`. Required reports, controls and performance bind to final source/binary hashes. |
| Remaining implementation | 37 required failures above; no stage advancement. |
| Independent review | Cumulative head/class cache and friend ownership/access/demand review remains pending. Also review current-instantiation cache validity, source ambiguity publication/projection and access versus lookup contexts. Implementation evidence does not replace or waive audit. |

Reproduce evidence checks with `python3 student.tests/pa17/verify_handoff.py`.
The prior checkpoint audit remains in [audit.md](audit.md). Review markers are
preserved. This handoff returns control to Ralph; it does not certify PA17.
