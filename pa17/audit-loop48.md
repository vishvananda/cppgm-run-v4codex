# PA17 checkpoint audit — Ralph loop 48

Stage base commit: `21748547a9e5befaae65e4fae120a63b3f9fcafb`
Last reviewed commit: `58789b00e19ae2aff032c8b04980b7d43265617f`

Target: **PA17 full-stage**, still incomplete. This first audit starts at the
stage base, which was also the previous review marker. Entry was the clean
`232f4a933613ef719a45d1e8f056fa191a03e41c` checkpoint: **248/343**, **95 failures**.
The previous implementation turn was progress: its commits and preserved loop-47
logs establish 242→248. This audit reviewed the thirteen accumulated commits,
their combined changes in all 32 implementation files and the source-set list,
and both subsequent audit fixes. It does not advance to PA18.

The assignment README, `spec.md`, testing/reference rules and all handoffs were
read as part of the review. Contract fixtures, reference bundle, comparison rules,
compiler invocation and required coverage are unchanged. No reference correction
was made. Historical handoff verifiers describe their frozen implementation tips;
the current audit has its own [verifier](../student.tests/pa17/verify_audit.py).

| Accumulated commits | Review and interaction findings |
|---|---|
| `a3fda6b0` | Baseline and ownership plan; recovered the full stage boundary rather than using the latest handoff. |
| `622e486a` | Template entities, aliases and structural deduction; checked parameter kinds, function/array/cv forms, pack reconstruction and template application identity. Audit found the nested-head context defect described below. |
| `8deaa931` | Retained source arguments and structural coverage; compared symbolic and concrete packs, nested default omissions, array element qualifiers and unparenthesized member `decltype`. Coverage belongs to each actual match, while context-independent ordering uses candidate IDs. |
| `da8594b7` | Alias namespace ownership, definition dependence and use-site template-argument access. Reviewed with later alias redeclarations and definition access recipes; found ordinary-alias merging. |
| `f6dbacde` | Canonical implicit cast identity and coverage only for competing matches. Verified the earlier coverage traversal was removed from the single-match path without changing selection. |
| `4bbb712a` | Entity handoff, controls and all three performance campaigns. Kept historical measurements and the full unfinished scope. |
| `fabfe92e` | Concrete member-template ownership, constructor selection, explicit instantiation, retained access obligations, body/storage demand and LowIR roots. Found crossed constructor ranking and caller privilege leaking into definition access. |
| `ca67b17d` | Renamed defaults, alias equivalence, nested extern suppression, static relocation demand and completed member selection. Checked publication after completion, independent member-template demand and source head retention. |
| `5ea7f7f6` | Definition access outside explicit-instantiation naming exemptions. The naming flag was reset, but qualified declaration privilege still crossed the boundary; fixed during this audit. |
| `ae5087da` | Member handoff and frozen controls/performance; matched claims to implementation and retained the unresolved multi-head and LowIR groups. |
| `aea1f09b` | Structural primary/partial definition-owner selection, distinct nested paths and current-owner aliases. Reviewed matching, renamed parameters, signature validation and late definition application. |
| `3bef03e0` | Indexed selected tuples for renamed partial heads. Its complete tuple identity was correct, but alias and pack-match paths still built per-parameter chains and frame interning used a parent collision chain. |
| `232f4a93` | Partial-owner handoff; verified its six fixes, unchanged 343-case contract and unreviewed accumulated range. |

Audit fixes are committed in `ebb0e5b2` and `58789b00`:

- Constructor ranking now checks each argument before using a template tie-break.
  `C(int,long)` versus `template<class T>C(long,T)` called with `(1,1)` must
  be ambiguous in either declaration order. N3485 13.3.3
  [over.match.best]/1–2 requires every conversion to be no worse before the
  non-template preference applies. Equal conversions and strictly better
  template/non-template alternatives retain their required selection.
- Alias-template and ordinary-alias declarations cannot share a namespace entity,
  even when both denote `int`; equivalent template redeclarations and ordinary
  alias redeclarations remain accepted. This follows N3485 14 [temp]/5 and
  3.3.1 [basic.scope.declarative]/4.
- Alias access recipes and class completion restore definition-owned access
  contexts. A qualified member declarator cannot lend access to a namespace
  alias's private dependent type. Both ordinary definitions and explicit
  instantiations are covered. Direct private names in explicit instantiations
  remain allowed by 14.7.2 [temp.explicit]/12; definition checks still follow
  11 [class.access]/1 and 14.6 [temp.res]/8. The alias fact key no longer depends
  on an unrecorded caller privilege.
- Nested template-head normalization retains enclosing bindings and distinguishes
  nesting depth as well as ordinal. `template<T>` and `template<U>` heads under
  distinct outer parameters cannot share identity. Normalization uses one
  growing scratch map over the head graph; matching sees established enclosing
  arguments and rejects missing type facts instead of equating two zero IDs.
  N3485 14.1 [temp.param], 14.3.3 [temp.arg.template]/3 and 14.5.6.1
  [temp.over.link]/5 supply the parameter-list equivalence rules. Renamed valid
  heads and a concrete dependent non-type head have positive controls.
- Alias and pack-match substitution use a retained parameter slice plus one
  canonical argument tuple. Frame interning hashes specialization, slice, count,
  parent and explicit tuple together. This removes unbounded parent collision
  chains and quadratic parameter walks within a wide head. Alias facts now have
  explicit active/success/failure states in a TU-owned compact vector. No syntax
  graph is copied and no broader declaration/body demand is introduced.

The [audit controls](../student.tests/pa17/audit_controls.py) retain reducers and
positive neighbors; they supplement the unchanged entity, member and definition
controls. Standard citations above refer to the checked-in
[N3485 text](../doc/n3485.txt), not compiler agreement or reference behavior.

Architecture trace: the nontrivial `Secret::f` explicit instantiation and the
`A<T*>` partial member call flow through the streaming source cursor and shared
parser/semantic construction in `lowering/driver.cpp`. Source names retain
interned IDs; template arguments and owner shapes become canonical typed tuples.
`class_pattern_selection` follows only the primary's candidate edges and records
the selected pattern and deduced tuple. `template_definition` attaches the member
to that pattern's indexed source/prototype path. Its application key is
specialization/source; its selected traversal also includes the current definition
head, so a late definition does not require a global retry or stale negative hit.

`syntax/occurrence.cpp` projects source/context IDs from one parsed region and
defers bodies/defaults; it does not replay grammar or clone source nodes.
`reuse_template_type`, substitution frames and access recipes reuse fixed facts
and establish dependent facts. Scope lookup uses flat name/kind indexes, lexical
parents and explicit base/using edges. Completed qualified-member selections are
published only after class completion; access remains a distinct recipe/frame
fact. Alias identity includes its concrete declaration owner and normalized
arguments. Renamed member heads retain their own parameter slices. Templates,
specializations, source regions and facts live in TU-owned pools; candidate,
normalization and traversal vectors are local scratch. Lowering temporaries end
with their function/TU; the typed program survives to the requested LowIR writer.

`demand_member`, `instantiate_function` and the definition application states keep
declarations, class completion, bodies and storage distinct. Extern suppression
preserves inline/defaulted demand and independent member-template specializations.
Static-local address constants demand their actual relocation targets. LowIR
lowering consumes selected entities, conversions, layouts and initializers;
`symbols.cpp` records explicit-instantiation emission roots and typed ABI facts.
Rendered names and manglings are output adapters, not specialization/cache keys.
There is no production token/IR text roundtrip, reference invocation, global
cache invalidation, whole-program search, or per-node owning allocation in the
new paths. The four added implementation sources are registered in the compiler
source set; earlier staged tools retain their existing shared phase boundaries.

For the optimization/fact trace, `sizeof(U)` in the retained partial member body
becomes the semantic constant 4 for `U=int`; `lowering/expression.cpp` consumes
the proven constant and emits typed LowIR `const`. The volatile loop bound and
repeated member calls remain executable work. The pinned supplied backend consumes
this explicit LowIR output and produces the checked ELF. This is PA17's required
validation boundary, not an assertion that the student has a PA24 encoder/MIR
allocator. No optional optimization pass changed. Existing constant evaluation
keeps its 1,000,000-work/512-depth limits; small array expansion and cleanup
thresholds remain 8, with counted-loop/shared-cleanup fallbacks. No ABI, exception,
debug, work-limit or growth policy was relaxed. Compiler and native measurements
are reported together in [audit-performance.md](audit-performance.md).

Final validation: `make test-pa17` is **248/343**, with exactly the entry's
**95-case failure set**. `make test-report-through-pa16` passes **2266/2266**;
`make test-report-through-pa17` reports **2514/2609**, every failure in PA17.
`perl scripts/cppgm_file_audit.pl --stage pa17 --paths dev/src` passes with the
same three inherited header-division warnings. These are stage-progress and
earlier-stage checks; PA17 itself remains incomplete. No passing case offsets
a new failure, because there are no new failures. Required reports ran serially.

All **115 personal controls** pass: the existing 34 entity, 34 member, 4 member
LowIR and 18 definition controls, plus 25 audit controls (14 native, 11 rejection).
The frozen entry fails eight of the added controls; the reviewed compiler passes
all of them. Rejection reducers exit normally with failure, not a signal.
The [evidence manifest](../student.tests/pa17/audit-evidence.json) binds source,
contract protection, complete commit list, exact failure sets, log hashes,
controls and performance records. The [source/LowIR/ELF trace](../student.tests/pa17/audit-trace.json)
records telemetry and a disassembly with explicit entry/function boundaries;
the selected member's native code contains the proven constant 4.

Performance acceptance follows spec.md §9 at PA17/O0. The new wide-head paths
show repeatable compiler benefit and lower RSS on equivalent outputs. Common
cumulative partial compilation costs 2.31%; no generated-code regression is
hidden, since corresponding executable bytes/text are identical and runtime is
measured independently. The inherited percentage, RSS and scaling targets are
diagnostics, not mandated exits; all old and new measurements remain preserved.
No required limit, correctness rule or coverage was weakened.

Remaining work stays in the broad groups in [plan.md](plan.md): nested declaration
and enclosing-head ownership; friends/access/ADL; dependent-name obligations;
pack/variable-template interactions; required LowIR cleanup/storage/emission.
These are implementation obligations, including all 95 failures, rather than
unanswered questions about the reviewed range. Avoidable handoff fragmentation
split alias access, explicit-instantiation access and selected-head environments
across three checkpoints. Their shared ownership needed this combined review;
future handoffs should close a broad owner and its interactions together.

| Loop / phase | Reviewed range | Findings and disposition | Validation / remaining work |
|---|---|---|---|
| 48 / checkpointAudit | `21748547..58789b00` (entry `232f4a93`, 15 commits total) | Four correctness groups fixed; tuple/frame complexity and alias fact states corrected. Full accumulated implementation and handoff interactions reviewed; final performance accepted at PA17/O0. | Earlier 2266/2266; PA17 248/343 with the same 95 failures; file audit pass; 115 controls pass. Full-stage implementation remains in the five broad groups above. |
