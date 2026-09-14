# PA15 accumulated checkpoint audit

Stage base commit: `8000f3c8ef4647d57f2c0775192585f14cab33d8`
Last reviewed commit: `538cfcb00441f57c0629f6d27fddbad723539479`

Reviewed range: `8000f3c8..538cfcb0`, inclusive of all changes after the base.
Audit entry: `db0686a3b17c87aa9d8dbd82e30346c034fc89b2`. Both original markers
still named the stage base, so this first audit covers all three accepted
handoffs, not just the pack handoff. The preceding handoff was progress: its
committed changes and freshly reproduced 166/177 report establish that state.

Disposition: **checkpoint audit complete; full-stage implementation incomplete**.
The same 11 of 177 fixtures fail. No earlier test regresses, coverage is intact,
and the discovered defects in the accumulated implementation paths are fixed.
This records a review boundary, not a claim that PA15 passes or permission to
advance. The remaining required work is grouped below and in [the plan](plan.md).

## Commit and combined-change review

| Commit | Reviewed contribution and interactions |
|---|---|
| `053af5e3` | Entry plan, original 56/177 baseline and owner boundaries; markers correctly retained. |
| `cdbfeb8a` | Constant quotient/remainder and shift limits, scalar casts, language-mode multicharacters and function-only inline metadata; checked both ordinary and query evaluators and preserved PA2 token mode. |
| `01b543ed` | Canonical type/value arguments, default/head substitution, lookup and typed ABI arguments; checked renamed heads, narrowing, enum identity and constant storage consumers. |
| `d3475a79` | Value handoff, frozen scalar evidence and preserved fixtures; historical verification is a snapshot, not a current-binary gate. |
| `6c8e2259` | Selection plan and its broader constant/source obligations; no unsupported numerical exit gate introduced. |
| `767e0797` | Explicit class/function/member/variable selection, source scope, primary demand exclusion, linkage and storage. Reviewed selection versus later pack keys and body demand. |
| `93dd5e29` | Selection/initializer counters and frozen controls; counters observe existing transitions and do not request work. |
| `113b6907` | Specialized static declarations versus definitions, initializer reset and duplicate rejection; checked forward declarations, later definitions and retained primary definitions. |
| `19225b3d` | Specialization handoff and all preliminary/noisy/repeat measurements; earlier unresolved owners remain required. |
| `53f1afa9` | Pack partitions, expansion frames, retained source lists, sizeof queries, bases and lifetime ordering; exposed missing traversal and signature-selection interactions. |
| `768e31f0` | Cooked/raw/character-pack literals, string constants, explicit prefixes and f80 writer operands; found target deduction reused call-expression rules. |
| `992b1170` | Unchanged scalar occurrence edges and empty-index fast path; useful repair, but per-parameter vectors and body pack scans remained. |
| `db0686a3` | Pack handoff and 166/177 baseline; all original failure sets and evidence retained. |
| `e211b3f8` | Audit repairs to pack traversal, typed specialization/target matching, head kinds and hash removal, with source/native and index reducers. |
| `034e3b91` | Removes scalar expansion dispatch and redundant pack scans by consuming the existing expanded-list fact. |
| `538cfcb0` | Frozen cumulative/checkpoint benchmark and independent evidence verifier, including historic hashes and current progress/coverage checks. |

The combined review includes **every changed `dev` file**, registration in
`dev/frontend_source_sets.mk`, all new personal controls and all evidence/scripts.
[Machine-readable evidence](../student.tests/pa15/checkpoint-evidence.json) lists
exact commits, source paths, source-tree identity, binary hash and artifact hashes.
The measured implementation is `034e3b91`; `538cfcb0` adds only audit tooling and
has the identical compiler tree. No compiler or tooling code is changed in the
subsequent records commit.

## Findings and repairs

1. **Pack traversal stopped at its container.** `associated_type_lookup` omitted
   the namespaces of type arguments within packs. `pack_adl` was rejected before
   the fix and now executes successfully; the non-type enum companion verifies
   that non-type arguments do not add associated namespaces. The corresponding
   local ABI type visitor now descends pack elements as well. This follows
   N3485 3.4.2 [basic.lookup.argdep]; no unrelated namespace scan was added.
2. **Selection and packs had incompatible signature paths.** Fixed arity filters
   rejected explicit and deduced pack specializations. Function-pointer matching
   constructed expression records and applied call decay, losing reference and
   return-type deductions. Selection now uses the ordinary canonical prefix and
   completed-specialization indexes with a typed target-signature matcher.
   Explicit value/type packs, omitted arguments, empty packs, reference targets
   and deduced return types execute correctly. See N3485 14.8.2.2
   [temp.deduct.funcaddr], 14.8.2.5 [temp.deduct.type], 14.8.2.6
   [temp.deduct.decl] and 14.7.3 [temp.expl.spec].
3. **An empty target could discard an explicit pack prefix during the repair.**
   Prefix length checking lived inside the per-argument loop. It now runs before
   that loop, including when the target has no parameters. Short-prefix and
   wrong-reference/return rejection companions pass; no invalid target is
   accepted to enable a positive control.
4. **Class head matching omitted pack kind.** Scalar/pack redeclarations with
   otherwise identical kinds and counts were accepted. Parameter-pack kind is
   now part of the existing head compatibility check. Both directions reject.
5. **Clearing bindings broke the shared hash index.** `deduce_expansion` resets
   bindings with zero, but `IdIndex::put` left an empty probe-chain hole. The
   reducer lost key 81 when clearing key 6. Zero now removes the binding and
   shifts only its affected cluster, with no allocation or global invalidation.
   The explicit index control checks every surviving key through interleaved
   replacements, removals, growth and final empty state. This repairs semantic
   cache correctness, not merely a fixture output.
6. **Scalar parameters paid unnecessary pack costs.** Each substituted scalar
   allocated a temporary expansion vector, dispatched through the expansion
   helper, and each function body scanned scalar lists twice for pack bindings.
   Scalar substitution now appends directly; body binding consumes the published
   expanded-list identity, including explicit empty packs. No new cache, token
   replay, source copy or output transformation was introduced. Frozen repeats
   verify the affected cost reduction and disclose all other measurements.
7. **Historical evidence scripts pin old live-state assumptions.** Their binary
   and old-marker assertions are appropriate to the recorded handoffs, not the
   current tip. They remain unchanged. The independent checkpoint verifier
   verifies all their frozen artifacts and sequences, plus the current code,
   marker, failure-set and fixture-tree identities. This preserves evidence
   without making an old binary or unadvanced marker a permanent exit gate.

[Reducers](../student.tests/pa15/checkpoint_audit.py) supply 11 native and seven
rejection cases; nine fail against the frozen audit-entry compiler and all pass
now. [Index reducer](../student.tests/pa15/index_erase.cpp) independently exposed
the hash corruption. Source rules are available in [N3485](../doc/n3485.txt).
There are **no reference corrections**: no fixture, reference, bundle revision,
comparison rule or required behavior was altered, so no reference exception or
compiler-agreement proof is being used.

## Architecture and optimization trace

A representative nontrivial declaration is the explicit `forward<int>` definition
in the frozen selected-pack runtime control. `emit_lowir` owns a preprocessor,
post-token cursor, parser and semantic analyzer for each translation unit. Source
buffers and interned identifiers remain owned there; parser callbacks build the
shared semantic/source graph. Explicit arguments become typed canonical values
or types and interned pack slices. Selection publishes the same complete entity
used by subsequent calls; explicit definitions do not demand primary bodies.
The emitter consumes its signature, selected body, conversions and linkage to
construct typed LowIR, then the text writer serves the PA15 output contract.

For the demanded `forward(T... t)` template, the retained source signature/body
is parsed once. Complete pack boundaries identify each specialization; incomplete
explicit prefixes have a separate index. Substitution frames are immutable,
parent-linked and keyed by their full bindings, parent and expansion lane.
Source/typed pack discovery is cached at its pattern owner. Only dependent facts
and changed occurrence-list edges are produced. The body consumes published
parameter identities and expanded lists. Nested ellipses select the appropriate
whole pack; a lane does not replace an unrelated pack's size or environment.

Source regions, immutable queries, declaration/layout/initializer/body facts and
emission have distinct owners. Active/success/failure states prevent duplicate
recursive work. Late explicit declarations update an uncompleted selected entity;
they neither reinterpret a completed primary nor retry all specializations.
Expected candidate mismatches return compact failure; final invalid declarations
still diagnose normally. Existing reverse default/definition/vtable edges remain
precise. The hash removal repair preserves unrelated bindings instead of clearing
a cache. Hot identity comparisons use flat indexes, never rendered types/names.

The audit also follows constant argument `N` through canonical type/bits, checked
integral conversion/evaluation, semantic constant facts and typed LowIR constants.
Signed overflow, invalid shifts and narrowing reject; short-circuit evaluation
retains its defined operand semantics. Unknown values do not acquire constant
promises. Raw numeric spelling is retained only for the language-required raw
literal operand, not as a semantic key or phase transport. Cooked long-double
calls carry the selected operand type through the text adapter. Literal calls
use ordinary call/lifetime facts, including reference and class results.

At O0 these are required semantic construction steps, **not optional optimization
passes**. There is no inlining/unrolling/fixed-point search or code-growth policy
to justify. Work is bounded by source, candidates, dependent nodes, actual pack
lanes and produced IR; scalar bypasses remove work without changing the IR. The
conservative behavior for an unavailable required fact is rejection, never an
invented constant or textual recovery. Later O1–O3 profitability, invalidation,
allocator, spill, encoding and self-hosting budgets keep their owning stages.

TU-owned flat records/arenas release with the frontend after typed lowering;
argument and candidate vectors are local scratch. The LowIR program and minimal
linkage/ABI facts survive to the requested output. No new per-node owning pointer,
process-global cache, duplicate syntax/semantic graph or production text roundtrip
appears in the reviewed changes. Native ELF checks consume the student's LowIR
through the **supplied backend**, as permitted for these controls; PA15 does not
own ELF encoding. Common controls have identical executable bytes, and runtime
checks cover loops, calls, memory and floating point. This cannot certify a
student allocator/encoder that is not introduced until later stages.

The frozen counters corroborate the trace at N=1,000 and 4,000: nested packs have
nine source regions, 18 discovery visits, 7N lanes, 13N+11 frames and 3N body
transitions; target signatures have three source regions, one discovery visit,
2N lanes and N body transitions. Reused values have N bodies; defaults have N
class completions and zero member bodies; explicit classes have N selections
and zero primary completions. Explicit value-pack selection has distinct prefix
and completed records (2N records, N selections), not duplicate work for one key.
These measurements supplement source review; they do not prove the unfinished
constant-object or ordinary-body groups correct.

## Validation, performance and remaining work

Fresh required checks at the final compiler source:
`make test-pa15` **166/177**, exit **2**;
`make test-report-through-pa14` **1935/1935**, exit **0**;
`perl scripts/cppgm_file_audit.pl --stage pa15 --paths dev/src` **pass** with the
same three inherited header-ownership warnings. The precise stage failure set is
identical to entry. All earlier fixture trees, all 177 PA15 cases and the harness
are unchanged. No current-stage debug/inspection gate is omitted: the handout
requires LowIR validation, not the later debug/native suites. Personal successful
native controls additionally request `--validate-lowir`.

Explicitly run personal suites: 27 pack/literal native + 8 rejections; 24 selection
native + 13 rejections; 26 value groups; 10 constant groups; 11 audit native + 7
rejections; the index collision/removal control. [Performance](audit-performance.md)
reports all four dimensions, frozen A/A/ABBA observations, spreads, parity,
work scaling, compiler text and stage-scoped acceptance. All 2,380 invocations
across 11 campaigns are hash-verified, including preliminary and noisy results.

Three broad implementation groups remain: **dependent matching/aliases** (four
fixtures), **constant objects/initialization/storage** (six), and **ordinary body
validation independent of emission** (one). Their exact names are in the evidence
JSON and their owner requirements in the plan. Checked fixtures remain required
even where they exercise handout-excluded general language. None was relabeled
an optional test or an audit uncertainty to hide incomplete behavior.

The handoffs should have tested selection, packs, ADL and signature targets
together when pack identities were added. The scalar-list performance repairs
also belong to that same parameter owner. Keep further work broadly grouped
through related consumers, with positive/rejection/native controls, rather than
creating additional handoffs at arbitrary passing-test counts.

| Audit | Reviewed range | Findings / evidence | Progress and remaining work |
|---|---|---|---|
| 2026-09-14 checkpoint 32 | `8000f3c8..538cfcb0` (16 commits) | Pack traversal/selection/target/head fixes; probe-chain removal; scalar owner cost repair; 2,380 verified invocations and complete source/fixture ledger | 166/177, identical 11 failures; prior 1935/1935; file audit pass; matching/aliases, constant objects/storage, ordinary source validation remain |
