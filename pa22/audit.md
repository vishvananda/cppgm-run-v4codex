# PA22 checkpoint audit 117

Target: **PA22 full-stage**. Checkpoint audit complete; stage implementation
remains **94/99**, with the same five failures as entry. No advancement to PA23.
Stage base commit: `a8482d768bd2dcede42ea63ef39e39cf3245c380`.
Last reviewed commit: `e90fa3fa514e2990bbe5ea4716252d8c42ae782a`.

Reviewed the complete accumulated range `a8482d7..e90fa3fa`, including all
three accepted handoffs and their interactions. Entry was clean at `853c4493`;
there was no live build/test process. The previous goal turn was progress:
handoff 116 committed conversion/initialization repairs and their evidence.
The entry tool status `2` was make's exit code, not the number of failing cases.

## Range and findings

The [commit/path manifest and first audit evidence](../student.tests/pa22/audit117-validation.json)
list all 15 entry-range commits. The combined review covers all **63** affected implementation/build files, including representation,
conversion, lookup, access, template identity/substitution/queries, constant
values, lifetime actions, LowIR adapters and every lowering consumer:

- 114: `cfcbacab` establishes ownership; `87ee0e07` implements scalar member
  pointers, null/adjustment/truth handling, local value proofs, pointer projection
  and single-vptr void casts; `e6316d05` excludes overloaded address calls from
  the local proof; `5a21daff` records controls and performance.
- 115: `d0c76b40` records ownership; `277c1c44` adds canonical dependent owners,
  deduction, NTTPs, callable queries and immediate selected targets; `7336cd8d`
  completes injected-name lookup, owner ADL and access-sensitive substitution;
  `2fd83db0` preserves parenthesized complete addresses and owner-only packs;
  `7b3685fc` records validation/performance.
- 116: `ca6b0cbb` records entry; `c016a259` visits sibling conversion bases and
  ranks class-value targets; `a5247137` adds required omitted-element zeroing;
  `7b6d62b9` counts lookup work and exercises inherited conversion templates;
  `810a9ee1` tests a nontrivial base constructor; `853c4493` records the handoff.
- Audit fixes: `015feb8d` repairs wrapper-aware adjustment proofs, hierarchy
  ranking and completed base-path reuse. `e90fa3fa` gives member constants
  their own canonical target/displacement owner and checks its consumers.

Four related findings were resolved:

1. The adjustment-elision proof stripped a parenthesized initializer before
   inspecting its incoming base conversion. Lowering applied the conversion but
   then omitted its receiver displacement. The proof now consumes the recorded
   conversion before every wrapper edge. Copy/direct/list initialization,
   assignment and a demanded dependent body reproduce the original wrong call.
2. Conversion ranking omitted `A::* → B::*` versus `A::* → C::*`, and omitted
   source hierarchy ranking for different conversion-function results with the
   same target. The selected-conversion owners now handle both directions,
   including the opposite member-pointer direction. C++11 [over.ics.rank]/4
   states these rules; the checked text is [N3485](../doc/n3485.txt), lines
   16692–16731. Function/data members, dependent call queries and pointer/reference
   conversion-function results are covered.
3. The affected hierarchy query repeatedly searched completed base graphs.
   Completed classes now consume the existing canonical path/miss fact; open
   classes retain uncached queries. The cache has no new global generation or
   invalidation scheme and introduces no new body/layout demand.
4. `Constant.bits` kept only a member declaration. Lowering's constant adapter
   reconstructed the displacement by searching between the declaration and
   destination classes. This lost conversions through repeated or sibling bases:
   distinct members of repeated subobjects collapsed, constexpr application
   failed, and a constexpr global could incorrectly need dynamic initialization.
   Constants now intern **(member entity, signed displacement)** with a separate
   type identity. Conversion composes the selected source-to-target displacement
   in semantics. Equality, NTTP validation/demand, ABI arguments, static data and
   lowering consume that identity. Constexpr receiver facts use the typed object
   address plus constant identity, distinguish repeated subobjects and cache
   completed results. They cache address selection, never mutable object values.
   The rules are [conv.mem]/2, [expr.static.cast]/12, [expr.mptr.oper]/4–6,
   [expr.eq]/2 and [basic.start.init]/2 in [N3485](../doc/n3485.txt). The equality
   section itself gives the repeated-base member-function example. All four
   [constant reducers](../student.tests/pa22/constants) pass, versus one at entry;
   each also requires static initialization and checked runtime behavior.

The original [18 composition controls](../student.tests/pa22/audit117.py) improve
**8/18→18/18**. Conditional/comma/assignment writes, address/reference exposure,
reference capture, sibling user conversions and dependent instantiation remain
covered. The final accumulated [validation](../student.tests/pa22/audit117-final-validation.json)
records **22/22 audit controls**, **36/36 inherited controls**, and source/binary
hashes. The analysis-disabled final lane also passes all 22 audit controls.
No reference, fixture, status sidecar or comparator was changed; no reference
exception or unproved oracle waiver is used.

## Architecture trace and ownership

For a nontrivial declaration, follow `constants/converted.cpp` from immutable
`SourceBuffer` through `Preprocessor`, `PostTokenCursor`, `syntax::Cursor` and
`Parser::translation_unit(&sem)` in `lowering/driver.cpp`. Parsing publishes into
one TU-owned node pool while semantic facts attach by NodeId. Identifier/type
interning supplies compact keys; member-pointer types retain a canonical owner
TypeId in the existing slot. The declared bases provide typed layout paths.
Constant conversion composes those paths once into the interned value, and
`constant_static_value` now only reads its member/adjustment and completed field
layout. It performs no member hierarchy reconstruction. Typed LowIR globals and
instructions reach the ordinary writer; there is no production IR text reparse.

For a demanded template, follow `dependent/nttp.cpp`'s `call<A,&A::f>` through
`form_member_pointer`, owner/child substitution, the context-sensitive member
argument query and canonical specialization key. The address cache includes
query identity, target type, access override and explicit-instantiation context;
incomplete results are not cached as completed successes. Query identities retain
lexical/naming context. The selected member's definition demand is separate from
class completion, access checking, layout and emission. `instantiate_function`
uses monotonic body states, immutable substitution frames and occurrences of
retained parsed regions, with fixed nodes/facts reused. Packs visit owner as well
as member types. The specialization receives its established constant identity;
`ObjectUse.member_target` retains the completed static fact for lowering. The
call emitter and ABI encoder read that selected entity; neither resolves it by
name or signature text.

`Analyzer::finish` drains indexed demand queues with cursors and fact states;
new facts do not retry all declarations. Conversion collection follows explicit
base edges and local conversion lists, restores hiding at sibling boundaries,
retains cv overloads, then checks the selected access/ambiguity path. The required
subobject traversal is proportional to actual inheritance paths, not unrelated
registries. Candidate/query rejection remains a compact failure; selected-program
errors retain diagnostics. Cached member receiver addresses have complete
object/value keys; lifetime/readability checks still belong to constant reads.

Both traces end in well-formed LowIR, then the supplied native test backend and
ELF execution, as PA22 requires. The hosted benchmark lane additionally records
ELF `.text` and disassembly. This backend is a validation boundary, never used to
implement student frontend/lowering output. Student native selection, allocation,
ELF emission and self-hosting remain PA24+ obligations.

Source nodes, semantic records, member constants, cache indexes and substitution
frames use TU-owned contiguous pools/open-addressed indexes. Candidate and
receiver traversal vectors have local lifetimes. No hot `shared_ptr`, per-node
ownership, cloned syntax graph or process-global mutable cache is introduced.
The frontend/lowerer die at the TU boundary; the minimal typed LowIR program
remains for this stage's required serialized output. Rendering/mangling are
output views, never semantic primary keys.

## Legality, profitability and budgets

The local proof has one shared **64-expression-node** budget per root and
active/proven/unknown states. It freezes after writes and exposure are recorded;
unknown, volatile, adjusted, escaped or cyclic values keep generic lowering.
The repaired incoming-conversion check closes the violated legality condition.
The final proof/no-analysis experiment establishes a repeatable live-loop benefit
with less text and stack traffic; [performance117.md](performance117.md) preserves
all observations, noise and regressions. The separate `Addr` non-null fact only
removes redundant null projection around established object addresses.

Immediate NTTP calls consume a selected semantic target with constant work.
The earlier campaign did not establish a repeatable runtime win, and no such
claim is accepted: its direct form is the required O0 contract. Omitted aggregate
zeroing is required initialization, carried by an immutable action and cached
canonical zero plan. The eight-element expansion cap and loop fallback remain.
Member conversion/application growth is constant per operation; the proof adds
no generated growth. The constant pool grows with unique member/displacement
pairs, and receiver selection is computed once per complete address/value key.
All these caches die with their TU. Unknown facts retain conservative behavior;
there is no new fixed-point pass, body cloning, inlining or broader optimizer.

Spec §9 applies to PA22/O0. Historical +15%, +16 MiB and 5.5× diagnostics do not
become exit gates through inherited plans. All historical evidence and misses
remain in performance114–117; mandated bounds, correctness and coverage are
unchanged. Necessary semantic work is measured on common correct inputs, and
later native/backend constraints are documented without adding stage gates.

## Checks, remaining work and ledger

Final commands: `make test-pa22` is **94/99**, exit 2, with exactly the same five
entry failures. `make test-report-through-pa21` is **3712/3712**, exit 0.
`perl scripts/cppgm_file_audit.pl --stage pa22 --paths dev/src` passes with the same
three inherited header-organization warnings. All **95 accepted** PA22 outputs
validate and roundtrip stably; all **four rejection** statuses are preserved.
The [gate record](../student.tests/pa22/audit117-final-validation.json) checks the
unchanged 99-case contract inventory, all earlier test/reference/comparison paths,
complete reviewed source hashes and frozen binaries. Raw logs/artifacts remain
under `/tmp/pa22-117/`; structured evidence is committed.

Remaining PA22 implementation work stays in two broad groups:

- Runtime member-value provenance and required lowering: the two general cases
  `300-const-member-function-pointer-address-call` and
  `300-repeated-nested-owner-member-template-address`, plus spec cases
  `300-member-pointer-parameter-variadic-deduction` and
  `300-overloaded-member-pointer-function-template-deduction`. Unknown parameters
  and indirect storage cannot inherit a zero-adjustment proof from owner layout.
  Preserve inverse/assigned/escaped controls; establish a sound value/exposure
  contract or prove a reference correction using the authorized evidence rule.
- Constant-condition materialization and demand:
  `300-structured-bool-conditional-member-pointer-dead-branch`. Receiver effects,
  temporary lifetimes and static constant emission need one coordinated owner.

These are retained implementation failures, not unreviewed handoffs or waived
correctness. The broader virtual/multi-vptr cases remain explicitly PA23 work.
114–116 made real progress, but tiny follow-up commits for address guarding,
parentheses, telemetry and one constructor control fragmented otherwise related
ownership groups. Future handoffs should carry a whole group with its interaction
controls and evidence. This audit retains all commits and reviews their combined
behavior; its own two fixes were validated together before the baseline was set.

| Audit | Accumulated range | Findings/fixes | Validation and disposition |
|---|---|---|---|
| 117 | `a8482d7..e90fa3fa` (three handoffs plus both audit fixes) | Wrapper conversion proof; source/target hierarchy ranking; canonical completed paths; member constant/receiver ownership | Prior 3712/3712; PA22 same 5/99 failures; file audit pass; personal 58/58; 95 roundtrips + 4 rejections; performance accepted at PA22/O0. Audit complete, stage incomplete. |
