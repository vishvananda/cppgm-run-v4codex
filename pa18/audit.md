# PA18 full-stage final audit 90

Stage base: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Entry: `f5c942ff` (audit 89's compiler code was `f2558ff4`).
Reviewed implementation: `fe4a11f0`, including `34b49cab`.
Target: **PA18 full-stage / O0 LowIR**; PA19 has not been started.
Disposition: **PA18 full-stage passes; Spec Alignment: aligned for PA18/O0**.

The prior turn was progress: its implementation and recorded checks exist in
history. Entry inspection found a clean tree and no inherited live job. The
entry binary hash matches audit 89's frozen final binary. Its 65-check explicit
validation was rerun successfully before repairs. [Audit 89](audit89.md) is
preserved verbatim. Its conclusions did not substitute for this source review.

## Findings and changes

**Nonfinal function parameter packs had the wrong deduction positions.**
`template<class... T> int f(T... v, int x)` rejected both `f(3)` and
`f<int,long>(1,2,3)`, and taking its address as `int(*)(long,int)` with `f<long>`.
The source pattern counted a symbolic pack as one fixed parameter. The same
assumption affected ordinary calls, expression queries, constructors, call
operators and function types used as deduction targets.

`deduction_parameters.cpp` now supplies a candidate-local typed view. A nonfinal
pack contributes only explicitly supplied lanes, each marked non-deduced; the
remaining parameters retain source ordinals. The ordinary argument conversion
and exact target-type check still validate the completed specialization. Both
function-type deduction and call deduction consume this rule. Trailing packs
retain extensible-prefix deduction. No token replay, synthetic declaration or
name-based recovery was introduced. The new source is registered for `cppgm++`
in `dev/frontend_source_sets.mk`.

**Default slots did not follow concrete pack expansion.** Each completed
specialization now owns an immutable slice mapping concrete parameter positions
to the original default-expression nodes. A default is still checked and demanded
only by its existing fact owner; expansion neither evaluates it nor instantiates
unrelated bodies. Signatures without packs share their original slots; signatures
without actual defaults allocate no replacement slice. Declaration checking also
now permits a pack after a defaulted parameter and rejects a default on the pack.

**A nonfinal template argument expansion must suppress deduction for its whole
list.** `f(L<T...,int>)` could not accept either `f<long>(L<long,int>())` or
`f(L<int>())`. `deduce_sequence` now recognizes that non-deduced context before
matching arity or binding any prefix/suffix parameters. Later conversion/target
matching still rejects inconsistent arguments; ordinary trailing pack deduction
is retained. This follow-up was found while checking the first repair's full
ownership path, after its first 71-check validation. The initial timing run was
explicitly stopped, its incomplete observations preserved, and final code was
validated and frozen again.

The proof is in the supplied [N3485](../doc/n3485.txt):

- §14.8.2.1 [temp.deduct.call]/1, lines 20603–20622: only a trailing function
  parameter pack deduces from remaining call arguments; its `g1` example shows
  explicitly supplying arguments for a nonfinal pack.
- §14.8.2.5 [temp.deduct.type]/5, line 20914: nonfinal function parameter packs
  are non-deduced; /9, lines 21001–21010, makes the entire template argument list
  non-deduced when its expansion is nonfinal.
- §14.8.1 [temp.arg.explicit]/9 and §14.8.2 [temp.deduct]/2–5: explicit arguments
  substitute into the type, and a deducible pack prefix may still extend.
- §8.3.6 [dcl.fct.default]/3–4, lines 10676–10684: a pack cannot have a default,
  but may follow a parameter with one. §14.7.1 [temp.inst]/10 retains lazy demand.

[49 controls](../student.tests/pa18/audit90_controls.py) improve **13/49 → 49/49**
on identical inputs. Accepted cases execute student-generated, validated LowIR;
rejection cases require failure. They cover explicit/empty/repeated/distinct and
non-type packs, array/reference prefixes, nested function types, template lists,
SFINAE, defaults/effects, queries and constexpr use. A nested explicit function-
type example is rejected by the installed GCC and accepted by Clang; its expected
acceptance follows explicit substitution and the cited non-deduced rule, not a
compiler vote. No course reference was changed.

**The audit harness depended on an old scratch executable.** The current
`validate89.py` now builds the ABI API check from checked-in sources into its own
work directory, runs its direct graph checks and uses that executable for fact
roundtrips. Validation no longer needs `/tmp/pa18-loop75/check-api`.

## Independent architecture reconstruction

| Requirement | Actual ownership and data flow reviewed |
|---|---|
| §1 source/streaming/parsing | `lowering/driver.cpp` connects `Preprocessor` → `PostTokenCursor` → syntax cursor/parser → `Analyzer`. `SourceBuffer` owns immutable bytes; tokens borrow spellings and retain interned identities. Cursor ring storage releases consumed tokens. The parser calls semantic construction on the same source graph; there is no syntax-to-semantic tree copy. `syntax/occurrence.cpp` projects source/context IDs and defers bodies/defaults/nested class regions without replaying grammar. |
| §§2–3 identity/lookup/candidates | Types, queries, arguments, declarations, scopes and specialization records have TU-owned compact IDs. `IdIndex` uses flat open addressing. Scope/name/kind indexes and explicit namespace/base/ADL edges limit lookup. `select_call`, deduction, ordering, conversion deduction and address-argument checking retain selected entities and conversions. Ordering keys include participating types, call arity, member owners and conversion mode. Address keys include target and access context. Expected candidate failure is a compact result; demanded body failures remain hard errors. |
| §§4–5 demand/cache validity | `template_instantiation.cpp`, `candidate_substitution.cpp` and `template_type_facts.cpp` distinguish active/completed/failed facts and intern complete parent-linked frames. Fixed types and expression properties are shared; dependent facts and concrete object uses belong to the specialization. `query_dependencies.cpp` records class/query reverse edges and per-query revisions, so one completion revises only dependent consumers. `Analyzer::finish` drains monotonic queues/cursors; it does not retry every pending entity. Defaults, class completion, body demand and emission retain distinct owners. |
| §6 direct typed lowering | Semantic expressions carry selected conversion recipes, materialized object/lifetime identities, layouts and canonical class-result classification. `values`, `user_conversions`, `construction`, `branch_lifetimes`, `function_declaration` and typed ABI projection consume those facts. Missing prepared conversions are invariants. `FunctionBuilder` writes typed LowIR directly; the writer is the explicit PA18 output adapter. There is no production serialize/reparse boundary. |
| §§7,9 transformations/budgets | `conversion_result.cpp` inspects one requested completed scalar conversion body, one return and at most eight wrappers. Only an established nonvolatile named constant is forwarded. Unknown/effectful/reference/virtual cases retain calls. Lowering retains receiver effects, second conversions, temporaries and actual function uses. Empty aggregate helper omission checks one action head and adds no code. Neither transform has fixed-point rescans, cloning or positive code growth. Constant evaluation retains one-million-step and 512-depth bounds; inherited array expansion retains its eight-lane policy. The new deduction views and default slices do linear work in consumed source/expanded parameters, without an optimizer or extra growth allowance. |
| §8 allocation/release | `NodePool` stores each parsed node once and compact occurrence records separately. `FactStore` uses slabs; expression properties are shared with compact per-use records. Semantic indexes and argument/frame arenas are TU-owned. Candidate parameter views die with candidate deduction; completed default slices die with the TU. Function builders and transient cleanup state reset between functions. Each driver's input scope destroys the frontend/lowering owners before the next input; typed linkage and the explicit LowIR output program survive until writing. There are no per-node owning smart-pointer graphs or accumulated process-global semantic caches. |
| §10 self-containment | Required output is produced by shared implementation phases in `dev/`. Source inspection finds no external compiler/reference invocation or textual IR parser in the production source-to-LowIR path. The supplied backend is invoked by explicit validation/performance harnesses only, as permitted by the course boundary. No filename/source-snippet/expected-answer dispatch was introduced. |

The review includes the accumulated stage changes, not only the new pack code:
partial ordering, immediate substitution, completion revisions, conversion and
address deduction, lexical/signature identity, aliases and correlated packs,
callable/prototype queries, list/cast queries, inherited/nested class demand,
constant arrays, class result/ellipsis boundaries, discarded values and lifetimes.
The last checkpoint's handoffs 87–89 were reviewed in their current combined
source form. Audit 89's closing commit contains documentation/evidence only;
there was no additional unreviewed implementation between its marker and entry.

[The new trace](../student.tests/pa18/audit90_trace.cpp) follows ordinary `Packet`
and demanded `dispatch<int,long>` through a nonfinal pack, one lazy default,
selected class copy, potentially throwing call cleanup, indirect call and an
alias-spelled class result. The direct and pointer calls share one completed
`dispatch` declaration/body. `next()` is emitted only at the defaulted direct
call; the pointer supplies its third argument explicitly. `make<Result>` and
its indirect caller share `obj<4x4>` result classification. LowIR has one
`dispatch` definition and preserves the selected copy/destructor calls.

The trace records **303 tokens, 525 parsed nodes, 147 occurrence records, two
body transitions, one default fact/check/demand, zero completion invalidations,
148 instructions and 225 operands**. Its supplied-backend ELF exits zero,
checking both results are 15, one default evaluation, two copies and four
destructions. All nine inherited traces also pass. The bounded result-summary
inspection suite separately checks legality, fallback, effects and retained
emission; the expanded performance corpus checks its actual emitted executable.

PA18 ends at unoptimized LowIR. Native selection, allocation, ELF writing and
optimized/debug code ownership start in later assignments. This audit traces
execution through the explicit supplied-backend boundary and makes no claim
that this compiler already implements those later phases or self-hosting.

## References, coverage and performance acceptance

The 29 accumulated changed fixture paths retain their reducers, bundle revision
and standard/contract proofs in corrections [65](reference-correction65.md),
[67](reference-correction67.md), [69](reference-correction69.md),
[79](reference-correction79.md), [83](reference-correction83.md),
[84](reference-correction84.md), [85](reference-correction85.md) and
[87](reference-correction87.md). Proof 87 distinguishes a valid isolated direct
call from the inconsistent exact-type pointer boundary; C++ does not mandate a
particular isolated IR spelling. Corrections 83/84 also rely on the explicit
inherited PA16/PA11 representation contract. The discarded-reference proof uses
its defined volatile reducer, not the original null-reference fixture's runtime.
Current reconstruction checks preserve the composed 84/87 oracle and prior
required zeroing. The bundle remains `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.

All **420 PA18 sources and 1,686 fixture paths** remain. Earlier fixture paths,
required behavior, success/rejection coverage, harnesses and comparison rules
are preserved. This audit makes no new reference correction.

[Performance 90](performance90.md) covers the union of audits 82 and 89 plus
nonfinal-pack scaling/runtime inputs. It reports frozen compiler latency/RSS,
checked executable runtime and code-size evidence together, with A/A calibration,
ABBA blocks, all observations and startup limitations. The interrupted initial
record is retained, explicitly superseded by the final corrected implementation.
Original optional-transform profitability remains in performance 82/86; current
code/output equivalence is checked for the named-result, retained-call and empty-
helper workloads, as well as loops, memory, floating point and lifetime controls.

Acceptance is spec §9's **PA18/O0** policy. No numerical compiler latency/RSS
ceiling is mandated here. Historical +15%, +16 MiB and 5.5× targets remain
preserved diagnostics, including historical misses; inherited plans cannot turn
them into exit gates. Necessary semantics and later backend costs are disclosed.
This does not relax correctness, coverage, mandated limits, work/growth bounds
or measured profitability of optional transformations.

## Final validation and ledger

[The final evidence manifest](../student.tests/pa18/loop90-evidence.json) binds
current source/binary hashes, all commands/results, controls, coverage, traces
and performance records. `validate89.py` passes **71 explicit checks**, including
**1,605 cases in 37 result-bearing suites**, ten source-to-native traces,
scaling/bounds/representation inspections, current-source ABI/API checks and
oracle reconstruction. Required validation on reviewed code:

- `make test-pa18`: **420/420**, exit 0.
- `make test-report-through-pa17`: **2609/2609**, exit 0.
- `make test-report-through-pa18`: **3029/3029**, **18/18 stages**, exit 0;
  PA10–PA12 additionally report **22** focused properties.
- `perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src`: exit 0,
  the same three inherited substantial-header advisories, no errors.

The supplied 3053 tally includes 24 tracked inputs outside default course roots
(18 PA8 later native-debug inputs and six PA9 top-level inputs); the authoritative
root course report is 3029. No fixture or count was changed to obtain that result.
Two preserved exploratory PA21 controls remain later-stage work, not PA18 passes.

The complete performance run retains **2,563 observations over 99 workloads**:
**89 exact-output comparisons and ten newly accepted inputs**. The one-element
nonfinal-pack/default runtime control already worked at entry; the larger and
empty packs exposed the defect. Seven targeted repeat runs retain **308** more
observations; the interrupted initial record retains **330** completed observations.
All 89 comparable LowIR outputs and applicable executable bytes are identical.
Compiler text grows **7,296 bytes (0.366%)**; new 2400-case compilation medians are
**373 / 488 / 326 ms**, peak RSS **63,472 / 78,360 / 61,868 KiB** for call/default,
function-target and template-list families. Work counters scale proportionally.
The noisy apparent compiler slowdowns do not persist consistently in repeats;
the report discloses calibration, paired spreads and the additional cast repeat.
No precise compiler speedup is claimed. Optional summary/helper profitability
remains supported by its original measurements and current output equivalence.

No unresolved PA18 correctness, architecture, self-containment, timeout or
file-audit finding remains. The compact plan records stage-scoped acceptance.
Implementation and final evidence are committed; a clean worktree is verified
after the closing documentation commit.

| Ledger | Disposition |
|---|---|
| Entry audit 89 | Preserved verbatim; all 65 explicit checks reproduced on the frozen entry binary. |
| `34b49cab` | Nonfinal function-pack views, concrete default positions, declaration rules, portable ABI validation and representative trace. First final validation: 71 checks passed. |
| `fe4a11f0` | Whole-list non-deduced template expansion rule; 49/49 focused controls, versus entry 13/49. Required full validation repeated after this change. |
| Initial performance | Intentionally stopped after the related template-list finding; raw observations and frozen binary retained, no final acceptance claim based on the partial run. |
| Final disposition | All required checks pass on `fe4a11f0`; full/repeat performance evidence accepted under PA18/O0 rules; no remaining handoff or audit obligation before PA19. |
