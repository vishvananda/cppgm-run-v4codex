# PA29 checkpoint audit166

Target: PA29 full-stage; this is an accumulated checkpoint audit, not stage completion.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous review: `cce8634c3c835cf6d5e8f4fa5fea0db213959718`.
Audit entry: `3036bd4ec4b1f8d315cf826437244de5236955bf`.
Last reviewed commit: `f07f78236eb475648834ed78afbca6c864f64408`.

The entire prior-review-to-code-tip range is reviewed, including eleven entry
commits, three accepted implementation handoffs, and the audit correction.
[Range evidence](../student.tests/pa29/evidence166/range.json) retains each
commit's paths and full/source diff hashes. Audits158/162 remain in history;
this records-only update follows the validated code tip.
The previous goal turn is classified as progress from the committed
implementation165 changes/evidence (`db0d0889`, `3036bd4e`). Entry process
inspection found no inherited live build or test. This turn adds independent
evidence and fixes. No earlier boundary was narrowed.

## Findings and disposition

1. **Assembly writes left stale private-local proofs.** `resolve_assembly` did
   not report output writes or memory exposure to `observe_scalar`. A later
   conditional involving a class temporary could use the local's original
   initializer to select the wrong branch. The fix uses the existing semantic
   observation owner; it neither rescans functions nor invents an IR-only effect.
   Register, read/write, memory, parenthesized and dependent-template cases are
   covered by `controls166/assembly-observations.cpp`. The GNU
   [output-operand contract](https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html#OutputOperands)
   defines these operands as modified by the assembly. N3485 [expr.cond]/1
   requires the condition's actual value to select the evaluated operand.

2. **The first new[] bound used manifest evaluation.** The declaration type
   builder evaluated an ordinary allocation extent as a required constant. The
   reducer allocated two elements instead of five. Bound expression checking
   now retains manifest contexts for template arguments/types, while its value
   proof uses runtime mode; later array dimensions remain required constants.
   `PlacementNew::dynamic_extent` carries the accepted decision into finish and
   lowering. The runtime fact accessor consumes runtime facts even when the TU
   has not yet encountered the builtin. No phase reconstructs this decision
   from the mutable availability of another cache. The
   [GNU builtin contract](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html)
   distinguishes manifest constant evaluation, while N3485 [expr.new]/6–10
   distinguishes first expressions, later constant dimensions and allocation
   sizes. Controls check nested dimensions, template arguments, runtime
   evaluate-once bounds, constructors/destructors, and a zero bound.

3. **Value parameters acquired nonexistent storage.** The constant-address
   evaluator treated a prvalue enumerator/substituted non-type argument as a
   real object. Context-reference lowering then allocated an uninitialized
   `$N` slot. A host run could accidentally pass when stack garbage was nonzero;
   standalone LowIR execution exposed the error. Parsed-expression and query
   evaluators now materialize prvalues, preserving glvalue object identities.
   N3485 [expr.prim.general]/8, [temp.param]/6 and [dcl.init.ref]/5 distinguish
   these values and the required temporary binding. The independent enum/value
   argument controls and combined string/query/reference/assembly reducer pass
   with explicit initialized storage in LowIR and both native emission paths.

4. **Query receiver cache context/lifetime was incomplete.** A materialized
   receiver derives from a query value but was keyed only by query identity.
   Its key now includes evaluation mode, cached hits propagate that value's
   mode dependence, and the existing reverse completion edges retire both mode
   receivers alongside the query values. This is a code-proven cache-validity
   defect; it is not represented as a separately observed course-test failure.
   The cache stays TU-owned, sparse and bounded to two variants per query.

No reference, fixture, sidecar, discovery rule or comparison rule changed.
Compiler agreement is supplementary; the reducers, language/extension rules,
accepted semantic facts and emitted code establish the corrections. Clause
references are to the checked-in `doc/n3485.txt`. No reference bundle revision
was needed. The known std-trait oracle questions remain unresolved failures.

## Every commit reviewed

| Commit | Reviewed content and interaction |
|---|---|
| `3bc61ecb` | Prior audit evidence, complete review marker, preserved coverage and stage-scoped performance classification. |
| `c3b29a60` | Assembly grammar/recipes, dependent operand binding, semantic constraints, effects/lifetimes, shared LowIR and native encoding. |
| `a6f3d6a2` | Hint instruction shape validation, external IR/native/debug controls, frozen performance inputs and handoff limits. |
| `e5690337` | Assembly handoff evidence, seven resolved fixtures, resource bounds and unfinished owner groups. |
| `22036430` | Function-context work plan and preserved full-stage scope. |
| `e16108af` | Scope/name query identity, dependent scope substitution, output-only signature rendering, semantic helper moves and telemetry. |
| `61e023f3` | String/query validation, performance and required source-invocation/evaluation residuals. |
| `12893184` | Evaluation-context scope and retained earlier review boundary. |
| `f42ab562` | Intrinsic registry, mode propagation across evaluator/query/static/activation/array owners, initialized storage lowering and controls. |
| `db0d0889` | Reference plans, actual activation storage, aliases/mutation, frame lifetime retirement and expanded final evidence. |
| `3036bd4e` | Full 403-fixture checkpoint, 53 failures, final versus preliminary performance evidence, remaining aggregate/source-context work. |
| `f07f7823` | Shared effect, allocation, materialization and query cache corrections; independent integrated controls and audit evidence tooling. |

## Architecture trace

`lowering/driver.cpp` creates immutable preprocessor source buffers, streaming
post/syntax cursors, and the integrated Parser/Analyzer. Assembly strings are
explicit source inputs parsed once into typed recipes; they are never emitted
and reparsed between production phases. The source-owned recipe is shared
across substitutions. Operand expression edges use the same graph and scope
indexes as ordinary expressions. Thirty operands bound constraint/name matching;
instruction work follows source bytes and the recipe's actual instructions.

The nontrivial declaration trace is `mode` in `integrated-context.cpp`.
`work<3>` selects the true initialization branch in manifest mode. The prvalue
argument is materialized with a typed initial value and temporary entity,
rather than an address for a template parameter. Its context-reference plan
records real storage; lowering emits the store before binding the reference.
The assembly consumes that location and records the write to `flag`. The
conditional therefore retains the actual runtime branch and both appropriate
cleanup paths. No name lookup, fake AST node or semantic reconstruction repairs
this in lowering. The array-allocation reducer separately demonstrates the
recorded runtime extent reaching the allocator's byte count and element loops.

The demanded-template trace is the same `work<N>`, plus inherited query controls.
One parsed pattern retains `FunctionName` queries keyed by lexical function
scope and interned name. `substitute_query` projects that scope through the
canonical specialization frame, visiting dependent nodes only. `length` selects
its overload and conversions once; concrete string storage has one entity per
scope/name pair. Pretty text is an output payload, never a semantic lookup key.
The immutable parent-linked environments and existing completion/demand edges
remain the owners. No global retries, generation flush or parse replay appears.

Expression/runtime values, activation keys and static/array proofs retain their
mode split. Activations additionally include body, typed arguments, receiver,
zero-initialization and reachable-storage snapshots. Reference rebasing visits
its recorded subobject path; mutable scalar writes update the owning frame and
storage version. Scope/activation exit retires the storage and clears frame
back-pointers, including exceptional exit. Aggregate-subobject mutation remains
the documented inherited positive-behavior gap, not an expected rejection.

TU arenas, interning pools, flat indexes and geometrically grown recipe/fact
vectors own retained data. Renderer/operand/activation scratch has bounded local
lifetime; no per-node owning shared pointer, global mutable cache or duplicate
syntax graph was introduced. The typed LowIR Program is the source/object
boundary. `compile_object` calls `native::compile_image`, then per-function
Selector/encoder and HostElf; native function scratch dies at each iteration.
External LowIR roundtrips are test adapters only. Required output uses this
compiler; host linking is the PA27/28 ABI boundary.

## Legality, profitability and bounds

The useful optimization fact is the private local's unmodified initializer.
Assembly invalidates it at the semantic write/exposure site, before
`prepare_scalar_consumption` can select a conditional arm. With no valid proof,
ordinary branching and cleanup remain. Existing profitability policy is
unchanged; no new transform or search was added. The allocation proof likewise
uses the appropriate evaluation mode and otherwise preserves conservative
64-bit extent arithmetic. Mode-sensitive initialized storage is required
semantics, not an optional speedup. Receiver completion invalidation follows
only explicit query dependency edges.

New optional work/growth budgets are zero. Source recipes and emitted operations
have linear bounds; each supported locked operation emits a fixed primitive or
one fixed-size CAS loop. Runtime contention is not compiler search. Reference
plans are constant-sized plus an existing subobject path; mode caches have at
most two variants. Pipeline work scales with demanded facts and emitted bytes,
with no new fixed-point pass. The 1,000,000-step/512-depth evaluator bounds,
native frame/data 0x70000000, 4096 alignment limits and course timeouts remain.
ABI, nonthrowing intrinsic facts, full-expression cleanup and debug metadata
transport are checked. PA8 permits the inherited absence of host DWARF; PA29
adds no contrary requirement. Hint debug locations survive explicit LowIR/MIR.

Actual frame/branch/call and text costs are inspected rather than inferred from
IR counts. Each integrated `work<N>` has a 96-byte O0 frame, eight call sites
(including alternative cleanup paths), nine loads and nine stores in its MIR;
allocation control `main` has a 160-byte frame. The reference now has an explicit
constant store before its address is bound, with no uninitialized `$N` slot. [Performance evidence](performance166.md)
reports final compiler latency/RSS and checked runtime/text together: 832 final
observations, 24 launcher samples, A/A calibration, ABBA paired spreads and all
preliminary measurements. Equivalent common images are byte-identical; audit-only
paired compiler ratios are 0.9966–1.0013. The cumulative memory/floating and
affected assembly timings have substantial scheduling variation, disclosed in
full. Counters bound required demand growth; no repeatable avoidable regression
is established and no speedup is claimed.
Necessary correctness costs are disclosed; invalid entry behavior is never an
affected performance oracle. Historical blanket 15% and zero-growth targets
remain diagnostics under spec §9. No mandate or correctness/coverage requirement
is weakened. Broader hosted runtime, optional optimizer/allocation and
self-hosting ownership remains PA30–34.

## Validation and remaining work

[Validation](../student.tests/pa29/evidence166/validation.json) is for code
`f07f7823` and the final compiler hash, including fresh required checks:

- `make test-pa29`: **350/403**, exit 2, the exact same **53 failures** as entry.
- `make test-report-through-pa28`: **4538/4538**, exit 0.
- `make test-report-through-pa29`: **4888/4941**, exit 2; only PA29 fails.
- File audit: pass, the four inherited substantial-header warnings, no new waiver.
- Explicit controls162/163/164/165/166: **49/53/52/47/21**, all passing
  (**222 total**). New controls are **21/21**, versus **17/21** at entry;
  [entry observations](../student.tests/pa29/evidence166/entry-regressions.json)
  preserve the four failing runtimes.
- Inspection161/163/165/166: **7/5/112/98** passing checks/groups. These cover
  source stats/image equality, validated and byte-roundtripped LowIR, MIR,
  standalone execution, and the current-source IR-to-host-object adapter.

[Coverage](../student.tests/pa29/evidence166/coverage.json) hashes every one of
the 403 inputs and checked sidecars against the previous review. The
[exact failure delta](../student.tests/pa29/evidence166/stage-delta.json)
proves no new failure and no reduced coverage; personal passes do not compensate
for a course regression. No harness/discovery/comparison rule changed.
The [inspection results](../student.tests/pa29/evidence166/inspection166.json)
preserve the integrated reducer's actual emitted evidence. All required checks
and performance observations use the final binary; preliminary evidence is
retained and labelled separately.

Remaining required work stays broadly grouped: extended syntax/types/layout
(36 failures), template demand/hosted ABI (15), source-invocation context (1),
and legacy trait contract/implementation (1). Preserve the source-invocation
and aggregate-mutation positive reducers, code-alignment placement, dependent
offsetof ABI signatures, class-convertible designator indices and extended
floating precision in their owning groups. Runtime vector lowering is outside
PA29; compile-time vector layout remains required. Full through-PA29 success
is still required before advancing.

The three broad owners justified separate handoffs, but their interactions were
under-tested. The evaluation alias followup and repeated records-only handoffs
added avoidable fragmentation. Complete each owner through dependent queries,
mode-sensitive storage, effect invalidation and both typed-IR/native paths before
handoff; host execution alone can hide an uninitialized stack read. This audit
reviews all those increments together and preserves one unambiguous code tip.

## Audit ledger

| Checkpoint | Reviewed range | Findings / disposition | Evidence / result |
|---|---|---|---|
| 158 | `2734e5c6..1ab3499d` (three handoffs) | Alias/expression/template storage fixed; deleted-copy reference corrected with clause proof; historical performance targets classified under spec §9. | PA29 317/403, no new failures; PA1–28 4538/4538; file audit/controls pass; four performance dimensions retained. |
| 162 | `1ab3499d..cce8634c` (entry `9662716b`, three handoffs) | Fixed atomic bool RMW, reference snapshots, alignment/native fallback and cv/identity conversions; reviewed complete traits/invocation/atomic ownership range; no reference changes. | PA29 338/403, identical 65 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 221/221 explicit controls; native/LowIR/cache checks and 1,088 performance observations retained. |
| 166 | `cce8634c..f07f7823` (entry `3036bd4e`, three handoffs) | Reviewed all assembly, function-context and evaluation/storage increments; fixed effect invalidation, runtime extents, prvalue materialization and complete query receiver keys/lifetimes; no reference changes. | PA29 350/403, identical 53 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 222 behavioral checks and 222 inspection checks/groups; 832 final performance observations plus 24 launchers, with preliminary evidence retained. |
