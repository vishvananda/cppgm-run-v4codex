# PA29 checkpoint audit190

Target: **PA29 full-stage**. Checkpoint audit complete; **stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `2df00585bd10d4e2e068934394dffc8adb0a47ed`.
Entry HEAD: `bac893d42bba792ab2ce3c868a69cd183acb73bc`, clean, **398/403**.
Last reviewed commit: `5aaf16d15f8e50925c0b75a5485893a958501b85`.
Reviewed range: **`2df00585..5aaf16d1`**, including all three accepted handoffs,
all intervening record commits and the audit repair. The prior audit is preserved
as [audit186](audit186.md). The entry status's `fail (2)` is command exit status 2;
the preservation baseline is **five failing tests**, not two.

## Range and combined review

Read `spec.md`, the assignment README, compact plan, testing/reference policy,
project layout, handoffs187–189 and their performance/proof records. Review starts
at the recorded last review, not at handoff189. The [range manifest](../student.tests/pa29/evidence190/range.json)
lists every commit and path: **78 implementation/build paths** changed before
entry, **82** across the final combined range. Each source increment and its
combined consumers were inspected, including interactions with inherited owners.

| Commit | Review and disposition |
|---|---|
| `5ef7f89a` | Prior audit records; checked marker, source binding, performance policy and retained ledger. |
| `5c93ddef` | GNU complex canonical types, constants, conversions, component expressions, builtin signatures, typed LowIR, SysV/native carriers and adapters. Static-data identity and exceptional-value propagation had gaps, repaired here. |
| `f714397c` | Complex MIR ABI reporting; checked component carriers against placement/encoding and explicit inspection. |
| `c1caa0fc` | Handoff187 evidence, failures and performance; verified original hashes and observations. |
| `e52f4586` | Contextual coroutine grammar, ordinary identifier lookup, parenthesized throw/comma and dormant dependent bodies. Checked scope/shadowing, declaration prediction and unsupported demanded-coroutine boundary. |
| `bf8bc00f` | Interned contextual-name keys and single class-predeclaration ownership. Checked complete-class lookup without duplicate declaration publication or grammar replay. |
| `54ddacb1` | Handoff188 evidence; checked scaling, noise and combined parser/semantic controls. |
| `d0df835d` | Definition/demand ownership plan; reviewed inherited targets under current spec §9. |
| `32e2430a` | Three reference exit-status corrections; independently reviewed reducers, source definitions, clauses and bundle revision, as detailed below. |
| `4ed34a87` | Actual static-assert contextual conversion, access/deletion, constant execution, message validation and dependent facts. Fixed source-list recipe execution was missing. |
| `e08707fc` | Noexcept, conditional explicit and deduction-guide access context; checked lexical/friend contexts through substitution. |
| `383fae65` | Conditional-explicit constructor-template substitution failure versus demanded-definition errors; checked fallback candidates and failed fact ownership. |
| `bac893d4` | Handoff189 validation/performance and manifest; checked historical contents at that commit. |
| `5aaf16d1` | Cohesive audit repairs plus personal reducers and reproducible controls/inspection/performance scripts; final validated source and evidence described below. |

## Findings and owner repairs

1. **Incomplete static constant-data key (spec §§2/5/6).** Complex constants pack
   two component identities into 64 bits; `key(type,bits)` narrowed the payload
   to 32 bits. Two namespace/static values with equal real and different imaginary
   parts reused the wrong emitted initializer. The TU-owned flat index now hashes
   the full typed payload and points into contiguous collision records; equality
   compares type, all bits and validity. Static vptr/construction publication uses
   the same object-ID owner. Repeated identical values share one fact, while
   different imaginary components and signed zeros remain distinct. Telemetry
   reports existing fact/key storage, without triggering work.
2. **Complex exceptional constants.** Scalar evaluation already allows admitted
   infinity/NaN inputs to propagate. The new complex arithmetic lost that permission
   when publishing its two result components. It now carries the input permission;
   finite overflow and a zero divisor still fail constant evaluation. Reducers
   cover all three component formats, exceptional multiply/divide recovery,
   static storage and runtime observations. This does not promise bitwise equality
   between arbitrary floating constant and runtime intermediate evaluations:
   C++11 N3485 §5/12 permits excess intermediate precision. The exploratory
   4097/4096 complex-float probe is retained with that disposition, not added as
   an unsupported exact-rounding gate.
3. **Complex RTTI and typed continuations.** Hosted exception/typeid demand
   previously assumed runtime-supplied fundamental RTTI for these extension types,
   leaving `_ZTICf`, `_ZTICd` and `_ZTICe` undefined. They now use ordinary weak
   COMDAT definitions, sharing the existing extension-type RTTI owner. Function
   completion and RTTI-failure continuations use `exception_fallback`, which
   supplies valid typed complex storage rather than an integer-zero complex
   operand. The all-handlers-return template path previously reached invalid IR.
   Bidirectional GCC/Clang exception transfer and typeinfo coalescing cover all
   formats and pointer RTTI; host runtime symbols remain external.
4. **Fixed source-list constant demand (spec §§4/6).** An assertion containing a
   fixed `flag{3}` in a template retained a ListPlan, but constant conversion only
   knew its query-list form or runtime materialization. Both now execute the same
   typed recipe with the proper source/query validator and recorded conversions.
   No initializer/overload replay or fake runtime temporary is introduced.
   Field evaluation applies the existing bitfield conversion. Controls cover
   aggregate/constructor/default/nested array/string/reference members, dependent
   lists, and false/narrowing/nonliteral/nonconstant/private rejection. Negative
   template controls demand `f<int>` because modern Clang defers a fixed false
   assertion in an unused template; that host policy is not the course oracle.
5. **Explicit LowIR special-value reader.** The writer's existing format-suffixed
   infinity spelling was rejected by its reader. Recognize optional f/F/l/L only
   after matching a complete special word; do not strip the `f` in unsuffixed
   `inf`. The writer and references are unchanged. An initial writer-only approach
   caused one PA11 spelling failure; it was reverted before final validation.
   Canonical text is checked for idempotence, with stronger direct/adapter object
   section and symbol equality. Original spelling need not equal first canonical
   spelling. Both preliminary failures are retained in evidence.

[Entry controls](../student.tests/pa29/evidence190/entry-controls.json) reproduce
rejection, incorrect execution or link failure with the frozen entry compiler.
[Final controls](../student.tests/pa29/evidence190/controls190.json) pass. The initial
RTTI probe omitted `<typeinfo>`; that preliminary observation is retained separately
and is not used as proof. Corrected source-hashed probes include it.
No fixture, harness, reference or comparison rule changed in this audit.

## Source-to-ELF architecture and optimization trace

The [integrated input](../student.tests/pa29/controls190/integrated.cpp) crosses
all three handoffs: a canonical complex namespace constant feeds a template
boolean conversion, assertion and conditional explicit/noexcept specifiers;
`box<C>` calls a later ordinary member named `co_await`; demanded `roundtrip<C>`
throws, catches and returns a complex value. An unused invalid member stays
undemanded. [Inspection](../student.tests/pa29/evidence190/inspection.json) and
[full machine traces](../student.tests/pa29/evidence190/machine-traces.json)
retain production AST/facts, original/prepared LowIR, MIR, host symbols,
disassembly, relocations, sections and unwind views.

Immutable source buffers feed the streaming cursor; identifier IDs are interned.
Complete-class predeclaration uses a bounded category/delimiter lookahead, not
re-parsing grammar or cloning a syntax tree. Lexical contextual lookup is indexed;
ordinary names retain ordinary meaning. Parsed dependent bodies are retained,
with substituted facts in immutable parent-linked frames. Actual coroutine-frame
execution is explicitly unsupported here; dormant syntax acceptance does not
claim full coroutine implementation. The integrated counters show eight deferred
regions, seven demanded regions and one template body transition; the dormant
member has no emitted symbol. No global retry or cache-wide generation flush
was introduced.

Complex types have three canonical component formats; constant pairs retain
compact component identities. Builtin signatures are keyed by equal unqualified
component type (at most three); runtime complex helper identities are bounded by
six per LowIR program. Selected conversions/access/defaults and failed specifier
specializations retain their existing complete fact keys and monotonic states.
Source-list and query-list demand use the same validated recipe without conflating
validation with runtime emission. Assertion-ready facts are not retried on class
completion. The repaired constant-data cache owns one record per complete key,
with TU lifetime and geometrically grown contiguous storage; it never uses
rendered spellings for equality. Temporary argument/field work vectors die with
their operation. No new hot per-node owning allocation or process-global cache
was introduced.

The useful fact is **complete constant identity**, from canonical components
through static-data publication, component offsets and final ELF bytes. Sharing
is legal only after full equality; signed-zero/imaginary differences cannot be
lost. Required constant evaluation and typed ABI handling have no optional
profitability decision. The work is proportional to demanded distinct values,
fields and interpreter steps; ownership-local caches need no unrelated invalidation.
Runtime complex operations retain scalar floating semantics and the SysV carriers,
including x87 for long-double returns. No fast-math or reassociation is added.

Production lowering builds typed LowIR directly. External text is an explicit
adapter: five direct/adapter cases agree in every named disassembled section,
symbolic relocation shown there and complete symbol set, and all adapted objects
link and run. Stats on/off objects are identical. Both paths enter native
preparation once. Forced inlining remains bounded by legality checks, recursion
and depth, 262,144 caller work and 4,194,304 total work; it reserves before mutation
and retains calls if proof/budget is unavailable. No new optional pass, enlarged
allowance or optimistic profitability claim is present. Constexpr retains one
million steps/depth 512; native frame/data/alignment limits and timeouts remain.

Native selection/encoding releases each function's MIR and temporaries at the
loop boundary in `native/driver.cpp`; retained TU data is linkage, object and
relocation/unwind state or the shared typed source/LowIR graph. No assembler,
host compiler or reference tool implements required output. Host disassembly
shows integrated `main` frame **160 bytes** and `roundtrip<C>` **176 bytes**, with
component homes and exception bookkeeping visible. The course standalone MIR
view has **160/160 bytes**, reflecting its distinct exception-runtime boundary;
it is not misreported as hosted MIR. Host CFI and symbolic relocations were
inspected separately. These O0 homes/copies are recorded costs, not claims of
spill elimination; the measured executable loop/call traffic is retained.

## Reference correction review

The three inherited corrections are accepted on their
[reduced proofs and cited C++11/contract rules](reference-corrections189.md), not
compiler agreement. Undefined primary class templates cannot supply members:
N3485 [temp.inst]/1, /5, /7 require definitions for the demanded member lists.
The third fixture explicitly defines a false primary; its negations require
rejection by [expr.unary.op]/9 and [dcl.dcl]/4. The reserved `std`/double-underscore
qualification is explicit: the course spec §10 prohibits synthesizing hidden
library definitions or overriding the declared primary by spelling. Positive
`-include` definitions exercise the original assertions unchanged. Reducers,
original inputs, host/reference observations and bundle revision were inspected.

Bundle source revision is `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, bundle SHA-256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`, compiler SHA-256
`e32741e2504f7c75a91abdf5cea0d5fc0b56eb43deb503d2ffbddbcc2f854f70`.
The bundle is unchanged. Exactly three exit-status sidecars differ since review;
all 403 inputs and 1,704 other contract/harness paths remain identical. This
exception does not authorize changing the still-unresolved nested ABI-tag oracle.

## Validation, evidence and stage acceptance

[Validation](../student.tests/pa29/evidence190/validation.json),
[failure identity](../student.tests/pa29/evidence190/stage-delta.json),
[coverage](../student.tests/pa29/evidence190/coverage.json) and
[source binding](../student.tests/pa29/evidence190/source-binding.json) pin all
477 tracked dev paths and tested binary to the committed code tip. Validation
and measurements preceded its commit; committed bytes match. This following
commit contains only audit records, with no intervening code/test edits.

- `make test-pa29`: **398/403**, exit 2, exactly the entry's five failures.
- Exact required prior-through command: **4538/4538**, exit 0.
- `make test-report-through-pa29`: **4936/4941**, exit 2; only PA29 fails.
- File audit: exit 0; same four substantial-header warnings (procedural/model/analyzer headers).
- Explicit controls187/188/189/190: **99/61/221/89**, all pass (**470 commands**).
- Full-phase source/LowIR/native/ELF inspection: **82 commands**, all pass.

`stageProgressPreserved` passes with **no new failure and unchanged coverage**;
extra personal passing tests are not used to offset course failures.
[Performance190](performance190.md) reports all four dimensions, **656 new
observations plus 16 launchers**, ten byte-identical equivalent image pairs and
corrected-only scaling. **2,936 historical observations plus 56 launchers** and
manifests were verified. Every new compiler paired range crosses unity; measured
RSS changes and all timing outliers remain. No repeatable avoidable regression
or speedup is established. Required key/list/RTTI costs are bounded and measured.
Inherited blanket 15% latency/RSS and zero-growth gates remain diagnostic under
spec §9; later-stage hosted/optimizer/allocator/inception obligations are not
extra PA29 gates. Correctness, mandated budgets, coverage and full-stage progress
requirements remain intact.

## Remaining work and handoff quality

The [five-case ledger](../student.tests/pa29/evidence190/remaining.json) retains
three extended binary128/half representation/operation/ABI failures, one vector
expression/type-operand intrinsic failure, and one independent nested-template
ABI-tag contract question. Continue in these broad owners. No remaining case
is waived; PA29 and root-through must pass fully before PA30.

The three accepted handoffs reduced failures **11 → 5**, including the three
proven reference corrections. Their broad owners are useful, but complex support
stopped before complete static-key, RTTI and continuation consumers, while
assertion work stopped before retained source-list evaluation. Those omissions
caused avoidable audit/handoff fragmentation. Before another handoff, exercise
each owner across definition/query/demand, constants, storage, runtime/ABI and
explicit adapters, including interactions with existing consumers. Do not split
remaining floating representation or vector operations into isolated spelling
patches with no complete typed path.

## Audit ledger

| Checkpoint | Reviewed range | Findings / disposition | Evidence / result |
|---|---|---|---|
| 158 | `2734e5c6..1ab3499d` (three handoffs) | Alias/expression/template storage fixed; deleted-copy reference corrected with clause proof; historical performance targets classified under spec §9. | PA29 317/403, no new failures; PA1–28 4538/4538; file audit/controls pass; four performance dimensions retained. |
| 162 | `1ab3499d..cce8634c` (entry `9662716b`, three handoffs) | Fixed atomic bool RMW, reference snapshots, alignment/native fallback and cv/identity conversions; reviewed complete traits/invocation/atomic ownership range; no reference changes. | PA29 338/403, identical 65 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 221/221 explicit controls; native/LowIR/cache checks and 1,088 performance observations retained. |
| 166 | `cce8634c..f07f7823` (entry `3036bd4e`, three handoffs) | Reviewed all assembly, function-context and evaluation/storage increments; fixed effect invalidation, runtime extents, prvalue materialization and complete query receiver keys/lifetimes; no reference changes. | PA29 350/403, identical 53 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 222 behavioral checks and 222 inspection checks/groups; 832 final performance observations plus 24 launchers, with preliminary evidence retained. |
| 170 | `f07f7823..221d6d0e` (entry `ecb69d94`, three handoffs) | Reviewed all vector/inline, aggregate and block-pointer increments; fixed aggregate pointer dependency ownership and vector query initialization; no reference changes. | PA29 361/403, identical 42 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 376 behavioral and 377 inspection checks; 1,384 final performance observations plus six launchers, with historical evidence preserved. |
| 174 | `221d6d0e..7139ceb5` (entry `914e1a0a`, three handoffs) | Reviewed all intrinsic/fold/closure increments; fixed discarded conversions/lifetimes, unevaluated capture recipes and combined typed ABI identities/grammar; no reference changes. | PA29 377/403, identical 26 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 218 behavioral and 144 inspection checks; four-dimensional final/historical performance evidence retained. |
| 178 | `7139ceb5..667edd80` (entry `e59dcaa1`, three handoffs) | Reviewed all source-invocation, declaration/demand and selection increments; fixed declaration-array bounds, initializer definition/dependency ownership and complete default frames; no reference changes. | PA29 381/403, identical 22 failures and 403 inputs; PA1–28 4538/4538; file audit and controls/inspection pass; 776 final observations plus eight launchers, 816 inherited observations reviewed; mandated limits and coverage preserved. |
| 182 | `667edd80..52070178` (entry `9211517d`, three handoffs) | Reviewed all decomposition, callable/inline and zero-extent increments; fixed array cv deduction and single native preparation ownership; no reference changes. | PA29 388/403, identical 15 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 141 controls and 653 inspection commands pass; 440 new observations plus eight launchers, 2,576 inherited observations verified; budgets and coverage preserved. |
| 186 | `52070178..2df00585` (entry `152396e2`, three handoffs) | Reviewed all guide/default, cast and bit-integer increments; fixed prototype inquiry reuse and exact-width overflow decisions; no reference changes. | PA29 392/403, identical 11 failures and 403 inputs; PA1–28 4538/4538; file audit pass; 141 controls and 836 inspection commands pass; 496 new observations plus eight launchers, 1,144 inherited observations verified; mandated limits and coverage preserved. |
| 190 | `2df00585..5aaf16d1` (entry `bac893d4`, three handoffs) | Reviewed every complex, contextual-syntax and constant-condition increment; fixed complete static constant keys, exceptional complex constants, RTTI/typed continuations, retained source-list evaluation and special-value reader. Accepted three inherited reference corrections on reduced clause/contract proof; no new reference changes. | PA29 398/403, identical five failures and 403 inputs; PA1–28 4538/4538; file audit pass; 470 controls and 82 inspection commands pass; 656 new observations plus 16 launchers, 2,936 inherited observations plus 56 launchers verified; mandated limits and coverage preserved. |
