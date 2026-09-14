# PA17 checkpoint audit — Ralph loop 52

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `c43e8eb68db7e9b3f1dd0bbb18f14c92e4fc4b30`

Target: **PA17 full-stage**, implementation still incomplete. This audit starts
at the previous reviewed code tip `58789b00e19ae2aff032c8b04980b7d43265617f`,
not the latest handoff. Entry was clean `b34c1be9`, **306/343**, **37 failures**.
The preceding goal turn was progress: loop 51's code, controls and frozen
measurements establish 301→306. The [previous audit](audit-loop48.md) remains
preserved, along with its measurements and evidence.

The review covers every one of the thirteen accumulated commits, their combined
changes in 43 implementation paths, and both audit fix commits. Including fixes,
that is **15 commits, 44 combined implementation paths and 45 touched paths**
(the intermediate conversion checks were removed). The
[range record](../student.tests/pa17/checkpoint52-range.json) lists every commit,
all touched paths and each implementation patch hash. `spec.md`, PA17's README,
its compact plan, testing/reference rules and all three handoffs were reviewed.
Contract fixtures, references, bundle revision, comparison rules and coverage
are unchanged. No reference correction was made.

| Commit | Review and interaction findings |
|---|---|
| `df0904de` | Previous audit records, not a new source baseline. Preserved its full-stage boundary, ownership findings and stage-scoped performance classification. |
| `9bba16a6` | Retained-head plan. Kept unresolved stage obligations instead of narrowing review to the next handoff. |
| `6a8454d8` | Lexical member heads, parent frames, source class identities, defaults, retained definitions and parser receiver categories. Checked declaration publication before substitution and concrete-owner binding rather than name recovery. |
| `3811277b` | Multiple nested definition heads, positional signatures, selected tuples, converting constructors and receiver-subtree caching. Checked distinct outer/inner parameter identity, constructor ranking and late nested definitions. Found source head obligations were not all checked at their proper owner. |
| `807e0989` | Raw source signatures through enclosing packs, body parameter expansion and current versus other nested template-ids. Reviewed with head defaults and later source-name facts; each final function parameter list expands at its own context. |
| `d3ef5e5d` | Head handoff, controls and frozen measurements. Preserved the superseded overlapping campaign and the final independent campaign; neither creates a numerical exit gate. |
| `4b821120` | Friend template grants, hidden namespace entities, ordinary friend recipes and body queue, specialization access, candidate identity deduplication and constant execution. Found missing address demand and deferred fixed qualified-friend checks; fixed below. |
| `a8cc241e` | Class/variable specialization access and static identity. Checked public/private stability, hidden base names and independent specialization grants. |
| `0fe13f16` | Friend handoff and compiler/runtime evidence. Reviewed its retained head/access questions cumulatively; no new scope waiver. |
| `3ce88dd7` | Current-instantiation identity, dependent introducers, declaration ambiguity and independent definition access. Checked renamed primary/partial/pack heads, non-type alias equivalence, leading returns, parameters, current/noncurrent receivers and source view publication. |
| `d7f0055d` | Parser ambiguity flag and pointer/ordinary declaration discrimination. Flag only selects the source check; it does not replace the semantic decision. Fixed, elaborated and instantiated controls preserve their categories. |
| `6afed48e` | Shared AST view fallback. Both source and projected views consume the same six-role interpretation; source-region traversal sees the resolved topology. No cloned subtree or grammar replay. |
| `b34c1be9` | Source-name handoff, exact 37-failure set, 261 controls and performance records. All three pending review groups were carried into this audit. |
| `1b137ed1` | Initial audit fixes and 44 controls. Its complete and interrupted performance campaigns remain preserved. The follow-up use trace found discarded function-name expressions shared the address-demand defect. |
| `c43e8eb6` | Evaluated ordinary function expressions now own demand, including discarded values and reused facts. Removed the extra conversion-path checks; retained qualified-address handling. Final reports, all 313 controls and measurements bind to this binary. |

The audit fixed these ownership defects:

- Taking an ordinary instantiated friend's address used only the function-template
  specialization demand path. The friend therefore had no body when LowIR
  referenced it. Evaluated ordinary function-name expressions and qualified address formation
  now use the existing selected-function demand owner, including discarded
  values and cached expression facts. General conversion paths retain their
  original selected-conversion handling. Its indexed friend queue and monotonic body
  states handle recursion, success and failure; unevaluated operands remain
  dormant. Address, decay, reference, argument-conversion, discarded/comma/boolean/void
  expressions, template body reuse, recursion and unevaluated controls cover this interaction. N3485 3.2 [basic.def.odr]/3–4
  and 14.5.4 [temp.friend]/4 require the definition when the function is odr-used.
- Fixed qualified and template-id friend declarations inside retained class
  templates were treated as deferred ordinary-friend patterns. Definition-time
  lookup now checks the existing entity, excludes undeduced template declarations
  from ordinary-function matching, and uses typed target deduction when required.
  The chosen specialization receives the grant. Dormant absent, mismatched and
  undeducible references reject; ordinary, deduced and explicit template-id
  neighbors execute. N3485 14.5.4 [temp.friend]/1 and 14.6 [temp.res]/10 establish
  these identity and definition-time binding requirements.
- Retained out-of-class member definitions accepted default template arguments,
  including newly added defaults and defaults on nested class definitions.
  Every source head is now checked before definition publication. In-class and
  namespace function defaults remain valid. N3485 14.1 [temp.param]/9 prohibits
  defaults on these out-of-class member-definition heads; /12 also forbids
  duplicate defaults in one scope.
- `check_template_parameters` repeatedly traversed projected member-template
  declarations to check unchanged source names, rebuilding scratch indexes.
  It also checked a retained multiple-head definition under only the outer head.
  Each source template now owns its lexical name check after its full head is
  available; nested templates own their checks and substitutions reuse them.
  New counters observe source work and reuse. A retained inner-parameter shadow
  now rejects, and nested/outer/inner positive and negative controls pass.
  This follows N3485 14.6.1 [temp.local]/6 and spec.md §§1, 4, 8–9. The
  specialization still performs its required dependent semantic checks.

The reducers and positive neighbors are in the
[audit controls](../student.tests/pa17/checkpoint52_controls.py). Citations refer
to the checked-in [N3485 text](../doc/n3485.txt); compiler agreement is not the
proof. Entry fails **17 of the 52 controls**; final passes all 52. All 261
inherited controls also pass. Every rejection has a normal nonzero exit status.

Architecture trace: `A<X>::B<Y>::f(Z)` with a private return alias and the
ordinary friend address both enter through immutable source buffers, the streaming
preprocessor/post-token cursor, and integrated parser/semantic construction in
`lowering/driver.cpp`. Names use interned identifiers. The class identity and
immutable source-parameter slice key current-instantiation types; named entities,
typed argument tuples, source heads and substitution frames determine member
signatures and specializations. Defaults do not change a fully supplied injected
type, and renamed heads have distinct slices. Lexical-frame caches are populated
after their parameter set and parent are established; new definitions own new
head/environment identities. No global invalidation is needed.

Partial selection follows the primary's candidate edges. Definition applications
are keyed by specialization/source; traversal results also include the current
source-definition head, preserving late attachment without retrying unrelated
members. Source prototype/signature indexes distinguish declarations before
body demand. Class completion, member-template declarations, body/constant
execution, static storage and emission remain separate facts. Ordinary friend
bodies use a deduplicated entity queue; constant execution uses the same body
state. Candidate deduplication occurs after specialization identity is known and
before conversions; declaration and arity filtering preserve required candidates.
Expected ordinary deduction rejection returns zero, without rendering diagnostics
or using exceptions as the normal candidate filter.

The declaration ambiguity has one source interpretation before fact/region
publication: the parsed name and delimiter nodes gain six indexed roles, shared
by source and projected views. `source_region` uses that topology and defers
bodies/defaults. Source checks traverse original parsed edges once per template; projected
declarations no longer repeat the lexical checks. Body/default demand stays
separate. Parameter packs expand only their concrete parameter children;
nondependent types, conversions and source recipes are shared. The TU owns source
buffers, interners, node slabs, flat indexes, fact vectors, parameter slices and
parent frames. Candidate/head work uses local scratch vectors; no per-node
owning pointers, deep copies, process-global cache or second syntax tree appears
in the reviewed paths. Function-local lowering state is released by its owner;
the typed LowIR program survives until the requested output writer.

Lowering consumes selected entities, layouts, conversions and lifetime facts.
Its entity-indexed symbol tables construct typed ABI records and emit each
required entry through the ordinary LowIR path. The corrected friend demand
supplies the missing body before lowering rather than recovering semantics from
names or serialized text. The two added implementation sources are registered
in `frontend_source_sets.mk`; earlier tools preserve their phase boundaries.

For the useful-fact trace, `sizeof(X)` in the selected member becomes a proven
integer constant, consumed directly by typed LowIR construction. Runtime tests
retain volatile loop bounds, calls, memory traffic and floating-point work with
checked results. The [trace record](../student.tests/pa17/checkpoint52-trace.json)
binds source, telemetry, LowIR and ELF/disassembly evidence. PA17 requires LowIR;
the pinned supplied backend is the explicit validation consumer. Own MIR,
allocation, encoding, ELF emission and self-hosting belong to later stages.
No production phase invokes a reference/host compiler or serializes and reparses
its internal IR.

No optional generated-code optimization changed. Existing constant evaluation
retains its 1,000,000-work and 512-depth limits. Array expansion stays bounded by
8 (including nested expansion), with counted-loop fallbacks; shared cleanup and
zero-initialization policies are unchanged. Unknown facts remain conservative.
There is no new fixed-point transform, growth policy or ABI/debug relaxation.
The source-check improvement removes duplicate required work rather than adding
an optimization level or speculative code growth.

[Performance evidence](checkpoint52-performance.md) reports frozen checkpoint
and cumulative A/B binaries, flags and sources, A/A calibration, ABBA blocks,
compiler latency/peak RSS, separate checked native runtime/text, counters and all
observations. PA17/O0 has no mandated numerical ceiling. Inherited +15%, +16 MiB
and 5.5× values remain diagnostic targets under spec.md §9; historical misses
are preserved and do not fail corrected code permanently. Correctness, coverage,
comparison rules and existing mandated work/growth limits remain binding.

Validation: `make test-pa17` is **306/343**, with exactly the entry's **37-case
failure set**. `make test-report-through-pa16` passes **2266/2266**;
`make test-report-through-pa17` reports **2572/2609**, all failures in PA17.
The file audit passes with the same three inherited header-division warnings.
Reports ran serially. The [manifest](../student.tests/pa17/checkpoint52-evidence.json)
binds source, range, logs, exact failure sets, 343 course inputs, controls and
performance. Run `python3 student.tests/pa17/verify_checkpoint52.py` to verify it.
Earlier handoff verifiers describe their frozen historical tips.

All **37 failures remain implementation obligations**, grouped in
[plan.md](plan.md): specialization/argument entities (7), qualified lookup and
expression queries (14), and required LowIR storage/transfer/cleanup/emission (16).
No independent-review question or performance target waives them. Avoidable
handoff fragmentation separated member heads, friend publication/demand and
source checks across three checkpoints, delaying these interaction findings.
Future handoffs should close one broad owner together with its declaration,
substitution, demand and lowering interactions.

| Loop / phase | Reviewed range | Findings and disposition | Validation / remaining work |
|---|---|---|---|
| 52 / checkpointAudit | `58789b00..c43e8eb6` (entry `b34c1be9`; 15 commits, 44 combined implementation paths) | Friend demand/binding, forbidden defaults and source-check ownership fixed; all three accumulated handoff groups reviewed; performance accepted at PA17/O0 with all observations preserved. | Earlier 2266/2266; PA17 306/343, same 37 failures; file audit pass; 313 controls pass. Three broad implementation groups remain; no stage advancement. |
