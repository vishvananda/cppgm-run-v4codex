# PA29 accumulated checkpoint audit — loop 186

Target: **PA29 full-stage**. Phase: **checkpoint audit complete; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `52070178897f5894edaf2f35d03a734b781979d4`.
Audit entry: `152396e2` (**392/403**, **11 failures**).
Last reviewed commit: `2df00585bd10d4e2e068934394dffc8adb0a47ed`.
Reviewed range: **`52070178..2df00585`**, including all three accepted
handoffs, their records, the combined implementation and this audit's fixes.

The [range record](../student.tests/pa29/evidence186/range.json) enumerates all ten
entry commits and 56 combined implementation/build paths; the audit adds three
affected shared implementation paths. The previous handoff made concrete progress
through committed bit-integer implementation and validation. Entry state was clean,
and no predecessor build/test process was still running. The reported stage check
status 2 was a process exit status, not the failure count; the actual baseline was
11 failures among 403 tests. [Audit182](audit182.md) and all prior evidence remain
preserved. This audit does not claim PA29 completion or authorize advancing to PA30.

## Findings and repairs

1. **Prototype inquiries still repeated semantic lookup during default demand.**
   Handoff183 retained `sizeof(parameter)` but skipped `decltype` and nonpolymorphic
   `typeid` inquiries. A default is instantiated before runtime parameter objects
   exist. The reduced [decltype default](../student.tests/pa29/source186/decltype-default.cpp)
   and [typeid default](../student.tests/pa29/source186/default-typeid-value.cpp)
   therefore failed at entry with an unbound type-query name.

   The shared source-default owner now retains canonical operand queries and type
   recipes, including inquiries reached through a functional or named cast. The
   query owner substitutes them using the default's complete frame and memoizes
   the projected occurrence. It does not manufacture a runtime parameter, replay
   parsing or search by a rendered name. Source validation tracks potentially
   evaluated contexts; a dependent `typeid` keeps that decision deferred until its
   type is known. Polymorphic parameter operands are explicitly rejected, while
   nested `sizeof` operands remain unevaluated and genuine dynamic operand effects
   still run. [Default inquiry/effect controls](../student.tests/pa29/source186/default-inquiries.cpp)
   cover multiple types, references, arrays, enclosing/member template frames and
   both cast forms; [effects](../student.tests/pa29/source186/default-typeid-effects.cpp)
   additionally checks a dynamically evaluated call containing `sizeof(parameter)`.

   The rules are C++11 [expr.typeid/2–4](https://timsong-cpp.github.io/cppwp/n3337/expr.typeid)
   and the adopted [CWG2082](https://cplusplus.github.io/CWG/issues/2082.html)
   correction to default arguments: unevaluated parameter uses are permitted,
   potentially evaluated ones are not. Clang accepts the personal polymorphic
   negative reducer; GCC rejects it. The rule, not compiler agreement, determines
   its expected rejection. No course oracle is changed.

2. **Overflow intrinsics used carrier width rather than bit-integer precision.**
   Handoff185 integrated widths into ordinary operations, but the inherited generic
   overflow owner truncated to a LowIR carrier before deciding overflow. For
   unsigned 7-bit `127 + 1`, the carrier can represent 128 even though the language
   type cannot. [The reducer](../student.tests/pa29/source186/overflow.cpp) returned
   failure because the intrinsic incorrectly reported no overflow. The same
   problem affected 93-bit results carried in `i128a8`.

   The shared intrinsic owner now invokes exact-width normalization before the
   store and representability comparison. The normalized value carries the existing
   type-qualified proof; widening it for comparison consumes the correct signed or
   unsigned result. All three operations, signed and unsigned targets, mixed inputs,
   volatile destinations and widths 1/2/7/9/65/93/127/128 are checked in the
   [narrow matrix](../student.tests/pa29/source186/overflow-matrix.cpp) and
   [wide boundaries](../student.tests/pa29/source186/overflow-wide.cpp).
   The narrow matrix uses independent ordinary-long arithmetic; the wide checks
   cover mathematical boundary results. This follows the
   [GNU overflow builtin contract](https://gcc.gnu.org/onlinedocs/gcc/Integer-Overflow-Builtins.html):
   store the destination conversion and report whether it equals the mathematical
   result. At most two shifts are added per affected intrinsic. Argument evaluation,
   volatile storage and exception behavior are retained.

Entry fails **9 of 15** new controls; final passes all **15**. The
[combined control](../student.tests/pa29/source186/integrated.cpp) exercises a
width-deducing guide, demanded `step<7>`/`step<93>` defaults, RTTI, overflow and a
cv-removing reference base cast with an **8-byte** adjustment. Cast ownership from
handoff184 is preserved through its complete inherited controls and this combined
use. Initial exploratory observations and preliminary checks remain in evidence.
An assembly return with manually dirty padding was not accepted as an ABI proof;
it fails with both compilers and motivates no implementation or oracle change.

## Every commit reviewed

| Commit | Content and interactions reviewed |
|---|---|
| `a2ce2670` | Audit182 records and immutable reviewed baseline; inherited budgets and reference proofs. |
| `054d162c` | Distinct guide syntax/registry, deducibility graph, prototype defaults, pack rejection and shared member/declaration paths. Missing inquiry retention repaired here. |
| `68ee6f8d` | Guide/default handoff, full validation, source binding, first and confirmation performance runs. |
| `f375d9d7` | Explicit-cast ownership plan; no implementation or acceptance change. |
| `52c00f4d` | cv-only selection, reference categories, composed base adjustments and member constant identity. |
| `c5f594fc` | Cast handoff, lifetime and rejection controls, source/adapter/ABI inspection, frozen performance and retained preliminary evidence. |
| `c6d7a46a` | Deterministic manifest ordering only; no source semantics, contract or benchmark changes. |
| `8ee20450` | Parsed dependent bit-widths; canonical types/queries, constants, ranks, conversions, template deduction/packs, ABI naming and typed lowering. |
| `27012cf5` | Bit-integer storage/padding, normalization proof, ABI/variadic alignment and serialization, RTTI and atomic/vector boundaries. Overflow interaction repaired here. |
| `152396e2` | Bit-integer handoff, coverage/failure ledger, source binding and all performance/inspection records. |
| `2df00585` | Both shared-owner repairs, cross-handoff reducers and audit validation/performance tools. |

No contract fixture, reference sidecar, discovery rule or comparison rule changed
anywhere in this range. The reference bundle remains pinned to source
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`. No reference correction is requested or
made. Earlier reference proofs and all original measurements remain available.

## Architecture and optimization trace

The nontrivial declaration is `Derived : Pad, Base` in the combined control;
its 93-bit member preserves precision, 16-byte size and 8-byte alignment through
layout and ELF. The demanded templates are `step<7>` and `step<93>`, whose retained
body uses a typed default, RTTI and overflow operations. The guide declaration
has its own identity; it creates no callable body or guide symbol.

Immutable source buffers feed the streaming preprocessor/posttoken/parser cursor.
Delimiter lookahead retains compact positions and does not replay grammar. The
Parser/Analyzer cooperate on source nodes and facts; template projections retain
source identity and share fixed decisions. Identifier/type/query identities are
interned, not rendered signatures. The guide registry uses primary plus canonical
signature shape; declaration deducibility visits only reachable typed edges with
a visited index. It does not instantiate a class merely to inspect a signature.

Concrete bit-width/signedness participates in type hashing/equality, signature
shape, substitution, pack traversal and deduction. Width queries use existing
complete-frame memoization, including failure. Cast facts retain reference
category, constant restrictions, member identity and base adjustments. Defaults
have separate source-binding, formation and demand states. Their retained query
identities survive before body parameter publication; repeated use of a completed
default does not reparse, rebind or globally retry work. There is no new global
invalidation or process-global cache. The explicit supported scalar limit remains
signed 2–128 / unsigned 1–128 bits; unsupported concrete widths are diagnosed,
not silently narrowed. This does not claim arbitrary-precision support or add a
PA29 performance gate.

Typed lowering consumes those facts directly. Wide bit integers use the serialized
`i128a8` carrier so stack/variadic alignment survives both production and explicit
adapters; ordinary `i128` retains its ABI alignment. Type-family tests select
operations, while full type equality includes the ABI decoration. Existing
call/register placement, variadic selection and RTTI emission remain shared.
The native variadic owner admits scalar types only, so its >8 alignment case still
rounds to the supported 16-byte boundary. Host ABI controls cover both call
directions, stack/register exhaustion, varargs, aggregate storage and exceptions.

The useful optimization fact is a value's exact precision and signedness, plus
its transient normalization proof. A new operation/conversion clears that proof;
normalization is omitted only when the value already satisfies the same type.
The corrected overflow owner uses this fact before comparing the bounded wide
arithmetic result. Legal wrapping/sign extension is required regardless of level;
there is no optional profitability choice or iterative pass. Each normalization
has constant work and at most two shifts, with no added call, branch or source
multiplication. Optional transform allowances remain unchanged. The inherited
single native mandatory-inlining owner still checks recursion/depth/frame legality,
reserves caller/program budgets before mutation, and retains valid calls on a
missing proof or exhausted budget. Debug/unwind facts and source locations are
not discarded by these changes.

Source/type/query/default storage belongs to the TU; work vectors and flat visited
indexes die after their bounded check. Source templates/fixed facts are shared;
only occurrence-dependent facts and immutable substitution frames grow with actual
demand. No hot per-node owning pointer, deep tree copy or recursive destruction is
introduced. LowIR is built in memory; text is only an explicit inspection adapter.
The selector/encoder releases function-local MIR after direct ELF emission and
retains required linkage/data/relocation/unwind facts. No host compiler, reference
compiler or assembler supplies the implementation's required output.

[Machine traces](../student.tests/pa29/evidence186/machine-traces.json) retain source
AST, original/prepared LowIR and selected MIR. Integrated frames are **304 bytes**
for `main`, **256** for `step<7>` and **592** for `step<93>`, including actual homes.
All named native sections, symbolic relocations and complete symbol sets agree
between direct and serialized-adapter emission; objects with stats on/off agree.
ELF sections, symbols and unwind frames are inspected. PA5/6 debug views do not
activate the hosted/template semantic mode: the new full-phase inspection uses
the production Parser/Analyzer configuration and build-time host environment.
The preliminary PA6-view rejection is retained, not reported as a code regression
or used to weaken any course check. Later DWARF/allocator requirements retain their
owning stages.

## Validation and performance acceptance

[Validation](../student.tests/pa29/evidence186/validation.json) and
[source binding](../student.tests/pa29/evidence186/source-binding.json) bind the
475 implementation/build paths and tested binary to `2df00585`. Validation and
measurements preceded its commit; the committed bytes match. The following commit
contains audit records only, with no subsequent code edits.

- `make test-pa29`: **392/403**, exit 2, exactly the entry's **11 failures**.
- Exact required prior-through command: **4538/4538**, exit 0.
- `make test-report-through-pa29`: **4930/4941**, exit 2; only PA29 fails.
- File audit: exit 0, with the same four inherited substantial-header warnings.
- Explicit controls183/184/185/186: **53/42/31/15**, all pass (**141** total).
- Inspections183/184/185/186: **291/194/169/182 commands**, all pass (**836** total).

[Failure identity](../student.tests/pa29/evidence186/stage-delta.json) contains no
new failure or offsetting extra passes. [Coverage](../student.tests/pa29/evidence186/coverage.json)
preserves **403 inputs and 1,707 contract/harness paths** against both entry and
previous review. Earlier-PA contracts and harnesses are unchanged and fully tested.

[Performance186](performance186.md) records all four dimensions, **496 new
observations plus eight launchers**, eight byte-identical object/executable pairs,
24 corrected-only scaling checks, and verification of **1,144 inherited observations
plus 24 launchers**. Compiler paired medians are **0.9953–1.0189**, with every
compiler paired range crossing unity; RSS increases and all noise/outliers are
preserved. No repeatable avoidable regression or speedup is established. Required
default/overflow costs are bounded and measured. Mandatory constexpr, storage,
frame, inline work/growth and precision limits remain unchanged. Historical blanket
15% latency/RSS and zero-growth targets remain **diagnostic under spec §9**;
necessary semantics and later-stage constraints do not create extra exit gates.
Correctness, mandated limits and coverage are not reclassified.

## Remaining work and handoff quality

The [remaining ledger](../student.tests/pa29/evidence186/remaining.json) retains all
**11 failures**: numeric representation/vendor syntax **7**, hosted template/demand
and ABI **3**, legacy trait contract **1**. Eight are unfinished implementation;
three remain independent contract questions (nothrow shorthand, default-sensitive
invocability and nested ABI tags). All remain counted. Continue broadly through
floating/complex representation and ABI, vector/contextual syntax, and hosted
template behavior. A reducer plus cited standard/contract proof remains necessary
before any future reference correction. Full PA29/root-through success is still
required before PA30.

The three handoffs reduced course failures **15 → 11** since review182. They
represent distinct useful owners, but inquiry handling stopped after `sizeof`
and integer integration stopped before generic overflow consumers. Those gaps
caused avoidable follow-up fragmentation. Complete each owner across source,
query/default demand, conversion, constant/runtime, storage/ABI and explicit
adapter boundaries before handoff; keep remaining work in the broad groups above.

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
