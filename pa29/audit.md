# PA29 checkpoint audit174

Target: **PA29 full-stage**; accumulated checkpoint audit, not stage completion.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous review: `221d6d0e4930da05db2913bdf5f50d808f89c744`.
Audit entry: `914e1a0a07e40884c91c0b967b0421eea9ef0d48`.
Last reviewed commit: `7139ceb5769eea12c6e2e00b54932f09d59c79b5`.

The review covers all sixteen entry commits across implementation171–173 and the
cohesive audit fix, including combined changes and interactions with earlier
owners. [Range evidence](../student.tests/pa29/evidence174/range.json) records every
commit, path and diff hash; the entry range touches 59 implementation/build-list
paths; the reviewed tip covers 60 combined source paths. The boundary was recovered from the existing review marker, not the latest
handoff. Committed implementation173 and its evidence establish inherited
progress; entry inspection found no inherited live build/test. Code was validated
and committed before this records-only update. The previous audit is preserved
verbatim in [audit170.md](audit170.md).

## Findings and fixes

1. **Fold reduction lost discarded-value facts and lifetime actions.** The source
   wrapper cleared the final operation's discarded form. Builtin comma lowering
   loaded every volatile result, including reference-returning calls, but omitted
   the selected class conversion and its temporary. Early fold-specific cleanup
   and exception traversal also bypassed common incoming/discarded conversions.
   The fix retains the final/sole operand's form and attaches the query-selected
   discarded conversion to each concrete FoldStep. Normal temporary construction,
   activation, default arguments, exception facts and cleanup consume that fact.
   Root cleanup runs before the iterative reduction traversal. Volatile scalar
   loads require both the retained source form and volatile lvalue category.

   C++11 [expr]/11 and [conv.lval]/2 in [N3485](../doc/n3485.txt) require conversion
   for the enumerated discarded volatile lvalues, including class copy creation;
   function-call lvalues are excluded. The hosted fold extension expands to the
   specified nested operations ([temp.variadic]/9 in
   [N4659](https://timsong-cpp.github.io/cppwp/n4659/temp.variadic#9)). Controls check
   three copies/destructions, sole-operand copying, final-value use, deleted-copy
   rejection, noexcept, throwing-copy unwind, scalar loads and call-result
   exclusions. A volatile access remains rejected in constant evaluation.
   [Host observations](../student.tests/pa29/evidence174/host-volatile-observation.json)
   show that Clang also omits the class copies: the language proof, rather than
   compiler agreement, determines these personal expected results.

2. **Capture recipes entered declaration-form unevaluated type queries.** The
   source walk skipped expression-form decltype but not a DeclSpecifier carrying
   decltype/typeof. Merely declaring `decltype(shape) local` in a generic closure
   captured `shape` by copy, including through nested capture propagation. Skip
   that unevaluated declaration form at the recipe owner. The selected type and
   local object's actual construction remain. N3485 [dcl.type.simple]/4 makes
   decltype's operand unevaluated; [basic.def.odr]/2–3 and [expr.prim.lambda]/11–12
   tie implicit capture to the relevant evaluated uses. The explicit-template
   extension retains the same distinction in the
   [capture rules](https://timsong-cpp.github.io/cppwp/n4868/expr.prim.lambda.capture).
   Three reduced direct/nested/typeof cases and the integrated case now avoid
   observable extra copies, with host execution agreeing on these cases.

3. **Combined template/closure ABI paths dropped canonical argument facts.** A
   literal selection index retained `int` instead of the builtin head's size_t
   conversion; dependent class types flattened matched argument packs; and the
   ABI graph merged pack and ordinary type parameters with the same ordinal.
   Convert the index at query formation, retain argument-pack identity in type
   lowering, and include parameter pack-kind in the canonical ABI node key. The
   explicit fact reader/writer preserve the new key bit. Ordinary renamed
   parameters at the same ordinal still share ABI identity. See the
   [builtin alias contract](https://clang.llvm.org/docs/LanguageExtensions.html#builtin-type-aliases)
   and [ABI template arguments/substitutions](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangle.template-args).

   A preliminary all-pack-grouping change exposed PA28's unresolved qualifier
   case. Its failure and symbols are retained in
   [preliminary validation](../student.tests/pa29/evidence174/preliminary-abi-validation.json)
   and [reducer evidence](../student.tests/pa29/evidence174/preliminary-abi-regression.json).
   The final encoder uses the source argument sequence for unresolved qualifiers,
   while resolved types keep pack grouping; this follows the distinct
   [expression grammar](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling-expressions).
   No rendered-name repair or semantic re-resolution occurs. Reduced type/qualifier,
   parameter-kind and index cases plus integrated host declarations/symbols/linking
   pass. The NTTP pack reducer uses GCC's existing target grammar; modern Clang's
   additional head encoding differs, as documented in
   [host ABI observations](../student.tests/pa29/evidence174/pack-host-abi.json).
   No unrelated ABI revision is imposed as an audit exit gate.

No required input, reference, sidecar, discovery or comparison rule changed.
The reference manifest still pins source `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`;
no bundle correction is claimed. Audit170's two contract questions remain counted,
unwaived failures, with their reserved-name limitations and char-traits discussion
preserved in the archived audit and its linked evidence.

## Every commit reviewed

| Commit | Reviewed content and interaction |
|---|---|
| `6525c1af` | Previous audit, baseline and retained performance/coverage evidence. |
| `4cdc6280` | Template intrinsic owner, stage boundary and validation plan. |
| `e029840a` | Canonical pack selection, generators, builtin registry, parser/query/type/ABI paths and negative controls. |
| `7545cda7` | First-class builtin alias heads, lazy lookup, template matching, host peer checks and measurement tools. |
| `846ef3fb` | Integral argument type preservation during deduction; no value-only equivalence. |
| `1bc21c57` | Intrinsic handoff, final validation, larger generator evidence and remaining work. |
| `05363f0a` | Retained fold semantic owner, validation and traversal design. |
| `fd565072` | Fold parsing, canonical queries, substitution, runtime/constant reduction, lifetime and ABI integration. |
| `827b4c7c` | Bounded constant scheduling, callable operands, query ABI and member-function facts across reduction. |
| `51fde102` | Fold handoff, final controls, scaling and residual failures. |
| `dab1dc09` | Explicit-template closure owner and validation plan. |
| `06d6fae8` | Retained heads, lexical frames, capture recipes, concrete storage, call specializations and pointer adapters. |
| `a34c7a65` | Capture telemetry, typed graph/host ABI peers and performance/validation tooling. |
| `909d6141` | Template scope retained for dependent trailing-return queries and substitution failures. |
| `128e7e39` | Direct decltype identity propagated to graph, encoder and explicit adapters; host peer proof. |
| `914e1a0a` | Closure handoff, preliminary/final measurements, validation and all 26 residuals. |
| `7139ceb5` | Discarded-fold, unevaluated capture and combined ABI owner repairs; explicit integrated controls and reproducible audit tools. |

## Architecture and generated-code trace

`lowering/driver.cpp` connects immutable source buffers and streaming pre/post/
syntax cursors to the integrated Parser/Analyzer. Syntax occurrence projections
reference retained source nodes and compact contexts; they do not copy or reparse
function bodies. Builtin alias heads are fixed-size lazy declarations. Template
arguments, query child lists, environments, types, layouts and specializations
use canonical IDs and flat indexes, never rendered signatures as primary keys.
Complete specialization keys and immutable parent-linked frames retain enclosing
and deduced arguments. Declaration/body/layout/exception/emission states remain
separate; repeated calls reuse completed specializations and dormant bodies stay
undemanded. Query prerequisites use existing precise dependency edges; these fixes
add no retry loop, global invalidation or process-global mutable cache.

The nontrivial declaration is the explicit-template closure in
[controls174/integrated.cpp](../student.tests/pa29/controls174/integrated.cpp).
`first<int,int,int>` combines selection alias identity, integer-sequence arguments,
constant and runtime folds, an unevaluated Shape query and a lambda head containing
both type and non-type packs. Capture recipes inspect the source once; nested
recipes propagate edges, and each concrete closure finalizes storage before
layout/calls. The fixed query creates a local Shape but no capture copy. Call
resolution publishes the operator specialization, parameter conversions, lifetime
and exception facts. Lowering consumes these records and typed ABI entries
without new lookup, fake source nodes, grammar replay or serialized transport.

The useful effect fact traced in `volatile-comma`/`volatile-scalar` is the retained
source form plus selected volatile conversion. It reaches LowIR as constructor
calls/temporaries or exactly three volatile i32 loads. The reference-returning
call case retains three calls and no invented loads. `volatile-throw` destroys the
first constructed temporary when the second copy throws. Constant evaluation
cannot fold away these effects. The existing O0 policy adds no optional transform:
legality is language-mandated, profitability does not authorize removing required
effects, and optional work/code-growth budgets are zero. Unknown aliases/call
clobbers/unwind remain conservative. No stale cache fact is reused or invalidated
by a global generation change.

[Machine evidence](../student.tests/pa29/evidence174/machine-trace.json) retains
LowIR/MIR and effect counts; [inspection evidence](../student.tests/pa29/evidence174/inspection174.json)
includes AST, typed graph roundtrip, telemetry-on/off object equality, ELF symbols
and final disassembly. Integrated `first` has a 48-byte frame and preserves
rbx/r12/r13; its closure operator has a 32-byte frame and three preserves. The
scalar reference-call case preserves its pointer across calls and has a 32-byte
frame. No allocator-quality claim is inferred from IR counts alone. Host linking
for runtime support occurs only in validation at the assignment's allowed
boundary. The explicit LowIR object adapter handles exception runtime references;
production never invokes it or another compiler.

Fold reduction/query/constant traversal is iterative and tracks expanded operand
and conversion records, not the unexpanded source pattern. Capture scanning
tracks source recipes and dependency edges, not calls. ABI encoding walks typed
arguments and emits the required grammar, with flat substitution keys. TU-owned
source/semantic arenas release with the TU; traversal vectors end with their
operation. Direct LowIR is lowered once per emission identity. `native/driver.cpp`
consumes one compact Function MIR at a time, releasing selection/allocation state
before the next function. Cross-function ELF/COMDAT/relocation summaries remain
for required object emission; no hot per-node ownership or textual phase buffer
was added. Generator/evaluator/native and pipeline budgets remain as detailed in
[performance174](performance174.md).

## Validation, evidence and disposition

[Final validation](../student.tests/pa29/evidence174/validation.json) and
[source binding](../student.tests/pa29/evidence174/validated-source.json) record
commands, statuses, hashes and exact tested compiler/source state:

- `make test-pa29`: **377/403**, exit 2; exactly **26** entry failures remain.
- Required prior-through shell command: **4538/4538**, exit 0.
- `make test-report-through-pa29`: **4915/4941**, exit 2; PA29 alone fails.
- File audit: pass, four unchanged inherited substantial-header warnings.
- Controls170/171/172/173/174: **28/45/51/59/35**, all **218** pass.
- Inspections171/172/173/174: **13/13/29/89**, all **144** pass.

The new entry comparison is **26/35**; nine failing checks become passing without
removing any input. Additional personal passes do not compensate for new course
failures. [Failure delta](../student.tests/pa29/evidence174/stage-delta.json) checks
the exact failure set. [Coverage](../student.tests/pa29/evidence174/coverage.json)
compares all **1,707** contract/harness paths to both entry and previous review,
retaining all **403** stage inputs. Preliminary correctness and performance
observations remain alongside final evidence, including the rejected PA28 ABI
revision and the corrected test's fact-ID allocation assumption.

[Performance174](performance174.md) records compiler latency/RSS, checked executable
runtime/text, frozen binary/input identities, A/A noise and ABBA paired spread,
work bounds and necessary semantic costs for the accumulated range and audit.
It also reviews inherited plans and preserves their measurements. Acceptance is
PA29/O0 under spec §9; unsupported blanket 15%/zero-growth targets are diagnostic,
not gates. Mandated limits, behavior and comparison coverage are unchanged.
There is no optional optimization to justify by noisy timing gains.

The [remaining ledger](../student.tests/pa29/evidence174/remaining.json) keeps broad
owners: extended syntax/types/layout **17**, template demand/hosted ABI **7**,
legacy trait **1**, structured source-invocation operands **1**. The two unresolved
contract questions remain counted. Full through-PA29 success is required before
advancing. The three distinct owner handoffs made progress from 42 to 26 failures,
but separate query/ABI followups caused avoidable fragmentation. End-to-end checks
of discarded contexts, unevaluated declarations and integrated host linkage
should accompany each owner before handoff. This audit covers every increment
and leaves one reviewed code baseline; it does not declare PA29 complete.

## Audit ledger

| Checkpoint | Reviewed range | Findings / disposition | Evidence / result |
|---|---|---|---|
| 158 | `2734e5c6..1ab3499d` (three handoffs) | Alias/expression/template storage fixed; deleted-copy reference corrected with clause proof; historical performance targets classified under spec §9. | PA29 317/403, no new failures; PA1–28 4538/4538; file audit/controls pass; four performance dimensions retained. |
| 162 | `1ab3499d..cce8634c` (entry `9662716b`, three handoffs) | Fixed atomic bool RMW, reference snapshots, alignment/native fallback and cv/identity conversions; reviewed complete traits/invocation/atomic ownership range; no reference changes. | PA29 338/403, identical 65 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 221/221 explicit controls; native/LowIR/cache checks and 1,088 performance observations retained. |
| 166 | `cce8634c..f07f7823` (entry `3036bd4e`, three handoffs) | Reviewed all assembly, function-context and evaluation/storage increments; fixed effect invalidation, runtime extents, prvalue materialization and complete query receiver keys/lifetimes; no reference changes. | PA29 350/403, identical 53 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 222 behavioral checks and 222 inspection checks/groups; 832 final performance observations plus 24 launchers, with preliminary evidence retained. |
| 170 | `f07f7823..221d6d0e` (entry `ecb69d94`, three handoffs) | Reviewed all vector/inline, aggregate and block-pointer increments; fixed aggregate pointer dependency ownership and vector query initialization; no reference changes. | PA29 361/403, identical 42 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 376 behavioral and 377 inspection checks; 1,384 final performance observations plus six launchers, with historical evidence preserved. |
| 174 | `221d6d0e..7139ceb5` (entry `914e1a0a`, three handoffs) | Reviewed all intrinsic/fold/closure increments; fixed discarded conversions/lifetimes, unevaluated capture recipes and combined typed ABI identities/grammar; no reference changes. | PA29 377/403, identical 26 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 218 behavioral and 144 inspection checks; four-dimensional final/historical performance evidence retained. |
