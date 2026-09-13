# PA14 architecture audit

Status: **in progress**, not stage completion. Audit entry `78c2f13e`;
PA13 base `8af3c149454e4e43e441206e6978f4d1300e079b`. This review reads the
implementation and stage history independently of checkpoint conclusions.

## Reconstructed architecture and spec alignment

| Surface | Ownership and data flow |
|---|---|
| Source / tokens | Immutable buffers, interned spellings, streaming preprocessing and post-token/syntax cursors. `lowering/driver.cpp` connects parser and Analyzer directly. Dumps are explicit outputs. |
| Parsing / templates | Each source region is parsed once. `syntax/occurrence.cpp` retains source nodes and creates eight-byte source/context views, deferring body/default regions. No template grammar replay or second owning syntax tree. |
| Identity | TU-owned canonical types, argument packs and specializations; compact source/concrete IDs; immutable parent-linked substitution frames. Rendered names are output adapters. |
| Lookup | Scope indexes and explicit base/using/associated edges. Candidate filtering precedes conversion work; typed selections feed expression facts. Fixed source decisions are shared where complete. |
| Demand | Separate declaration, definition, default, member, lifetime, layout, vtable and emission states. Monotonic queues visit required edges; parameter scanning advances a cursor. Failed public demands must remain terminal. |
| Storage | Shared source properties, sparse fact slabs, expression use records and flat TU indexes. Parser/candidate/function-builder scratch has shorter lifetime; frontend data is released after each TU's typed LowIR construction. |
| Lowering / ABI | One typed entry per function/initializer/thunk/ABI identity. Builders consume declarations, conversions, object paths, cleanup and virtual slots. The LowIR writer is the requested output adapter. |
| Optimization / native | PA14 requires O0 LowIR. Native selection, allocation, ELF writing, optimization levels and self-hosting are later-stage owners. The supplied backend is used only by validation/benchmark harnesses. |

Reviewed the driver, source/preprocessor/cursors/parser, occurrence machinery,
template declaration/class/definition/instantiation and binding paths, type
facts, defaults, query calls/operators, member access, class values, statements
and typed symbol emission. PA5 lexical hints implement its documented parsing
contract rather than fixture recognition.

## Representative traces

A fixed class conversion inside `template<class T> int f(C& c,int x)` begins
with one parsed body and symbolic parameter identities. Binding establishes
fixed types and conversion-function selection in the definition environment.
Each return owns a shared recipe. A demanded `f<Tag>` has a canonical key and
an immutable frame mapping source parameters to concrete entities; its body
view reuses fixed expression properties and return selection. Applying the
recipe establishes that use's objects and cleanup. Body and jump/lifetime
completion precede publication to typed lowering, which emits calls, branches
and returns. The external backend executes this LowIR for observation; the new
runtime control checks eight million conversion effects and its checksum.

A dependent jump/cleanup error follows the same demand but fails after statement
construction. Previously, definition presence could make a repeated query
succeed; direct bodies also lacked a full failure interval. Functions now enter
Active before parameter cleanup/statements and publish body and lifetime
Success separately. Failures remain terminal in function/member/specialization
owners; emission requires completed facts. Eight cases repeat finish/body
queries 10,000 times with stable node/entity/lifetime counts. A valid control
completes once.

## Findings and changes

- Added complete body/lifetime failure intervals and protected TU consume/finish
  publication. Entity remains 112 bytes; lowering requires completed facts.
- Fixed definition-time return/condition checks, loop/switch placement,
  duplicate labels and unbraced scope isolation. The 64 controls include 54
  invalid programs accepted at entry and ten checked native programs.
- Shared fixed return conversion selection; concrete consumers retain their
  own materialization, implicit-move and NRVO/local identity.
- Corrected symbolic implicit-object cv/fixed-base relationships and callable
  object/surrogate candidates in retained queries. Remaining reuse is open below.
- Fixed the personal Perl adapter's sort-variable shadowing. The course
  comparator is unchanged; one permitted top-level presentation difference
  follows earlier fixed return-type completion. Exact text is not a PA14 gate.

N3485 [stmt.select]/4, [stmt.switch]/2–4, [stmt.break]/1, [stmt.cont]/1,
[stmt.return]/2–3, [basic.scope.block]/2, [class.copy]/31–32 and [temp.res]/8
ground the controls. The standard permits some uninstantiated invalid templates
without a diagnostic; PA14 explicitly requires definition-time checking of its
supported unused bodies. No reference output was corrected.

## Remaining findings and handoffs

The initial three initializer reproducers are now rejected. Source initialization
checks retain scalar/reference conversions, constructor/default recipes,
narrowing facts and source aggregate shape. Static initializer dependence binds
immediately; nonstatic member initializers wait for complete class context.
Local const/constexpr values keep their qualifiers and constants. Query-only
constructor operands use the common typed selector without source temporaries.
Concrete list materialization projects operands and lifetime scopes into the
use's occurrence context. The 82 controls include 71 entry-accepted invalid
programs and eleven native programs; one valid constexpr case failed at entry.

This continuation independently found more work, rather than treating the new
pass as completion. Four frozen source-owner reproducers still succeed:

```cpp
template<class T> void f(){int& r;} int main(){}
struct D{D()=delete;}; template<class T> void f(){D d;} int main(){}
struct C{int operator()()=delete;}; template<class T> void f(){C c;c();} int main(){}
struct C{int operator()(){return 42;}};
template<class T> int* f(){return {C()()};} int main(){}
```

They concern default object initialization and publication of query-only source
expressions. A variable's runtime identity also currently makes its callable
type query dependent even when its class type is fixed; `F v; D d(v());` can
therefore miss a deleted `F::operator()` in an unused template. Fixed condition
recipes and query-only calls still need reuse and temporary/destructor review.

Three reduced mode/mapping defects were confirmed after the initializer
campaign ([handoff manifest](../student.tests/pa14/initializer-mode-handoff.json),
which also records the variable-callable case):

- With `explicit E(int)` and `E(double)`, `E e=1` inside an unused template was
  wrongly rejected. Ordinary copy initialization must exclude explicit
  constructors from its candidates; copy-list initialization considers them and
  rejects a selected explicit constructor. The common selector/consumer path
  now preserves this distinction.
- `struct A{E e;}; A a{{1}};` wrongly accepted an explicit-only `E(int)`:
  aggregate elements are copy-initialized ([dcl.init.aggr]/2).
- For `struct A{T x;int* p;}; A a{{1},{42}};`, mapping stopped at dependent `T`
  even though the explicit braces delimit that field and expose the fixed
  invalid pointer initializer.

The three mode/mapping defects are corrected. Five default/query outcomes above
remain implementation work, with no external blocker. The next owner is source
default initialization and query-only expression publication/reuse.

## Copy/direct/list ownership correction

`InitializationMode` passes the source use's context through ordinary and
retained initialization. Ordinary copy initialization consumes a conversion
recipe, including the common converting-constructor/conversion-function choice.
Direct and copy-list construction retain all candidates, with the latter
rejecting an explicit winner. Same/derived class transfer selection excludes
explicit copy/move constructors in implicit conversions and returns; direct
subobject transfer and ABI queries retain their own policy. Selection itself has
no negative cache to acquire an incomplete mode key.

Aggregate elements and omitted elements use copy initialization. Explicit braces
around a dependent field delimit one clause, so the source checker continues to
known later fields; unknown brace-elided widths still defer to the concrete
aggregate. A new native control also exposed an expression ownership collision:
a scalar clause's expression had been overwritten by its class construction.
The copy-conversion record now owns that operation separately, preserving the
scalar operand through typed LowIR. No synthetic syntax node was introduced.

Source conversion recipes remain keyed by source use, with target verification;
that source use uniquely fixes its initialization mode and declaration context.
Concrete materializations map operands through the occurrence frame. Empty-list
facts now have separate direct/copy indexes, each keyed by canonical target and
access scope. Success and failure remain terminal in their respective plan and
validation owners; adding a completed fact does not invalidate unrelated caches.
The inspection probe alternates both query orders 10,000 times, verifies the
qualified conversion target and checks stable node/entity/plan/conversion counts.

The 75 controls include 49 rejections and 26 checked native programs. Against the
frozen initializer entry, 35 statuses and three native results were incorrect.
The proof cites N3485 [over.match.ctor]/1, [over.match.copy]/1,
[over.match.list]/1, [dcl.init.aggr]/2,7, [class.copy]/31–32 and [temp.res]/8;
compiler agreement is not used as the language proof. No reference was changed.
An interrupted validation is retained with its initial binaries: source review
caught a dropped cv-qualified target before any timing. Current validation and
measurements use the corrected binaries and separate paths.

## Performance, acceptance and validation

[Body performance](../student.tests/pa14/body-audit-performance.md) preserves
532 new observations with paired spread, A/A calibration, RSS, native runtime/text,
compiler growth and layouts. All 15,848 earlier observations remain. No optional
runtime transform is added and no runtime-profit claim is made. Source checks
and concrete object/lifetime work have separate budgets; generated growth is
zero on comparable inputs. Historical diagnostic targets do not create extra
PA14/O0 exit gates.

The [initializer campaign](../student.tests/pa14/initializer-performance.md)
adds 616 observations, bringing the verified history to 16,996. All ten native
executables are byte-identical. Source initialization recipes scale as 2M and
concrete uses as 2KM; candidate visits at K=128 fall 2176→144. No timing or
runtime gain is claimed amid the preserved A/A and paired variation. Compiler
text grows 1.466%; Analyzer grows 144 bytes, and other measured records do not
grow. The 67 release/sanitizer checks include current inherited default,
lifecycle, virtual, demand and ABI controls. An artifact-name collision in two
personal controls was corrected by rerunning both 82-case groups under the same
frozen binaries; the prior script/manifest and the other 65 results remain.

The body change passed 51 checks: file audit/root report, 349 release/sanitizer
and 349 entry/current comparisons, 35 existing native programs, both builds'
64 statement and nine repeated body controls, inherited failure/rejection/demand
suites and PA13 ABI/lifetime controls. All 1,266 fixture/reference hashes are
unchanged. Root count: 1935/1935, all 14 stages. Remaining findings prevent closure.

The [mode campaign](../student.tests/pa14/modes-performance.md) adds 728 frozen
observations and 71 passing validation groups, bringing the ledger to 17,724.
All 11 native executables are byte-identical. It also exposes avoidable copy
initialization metadata: 128 instances × 8 initializers add 1,024 temporary
entities/scopes despite an existing destination, and RSS rises 806–824 KiB.
Those diagnostic measurements are retained; the following correction closes
that avoidable ownership cost. Five default/query reproducers remain confirmed.

## Existing conversion destination ownership

The common copy path now records Recipe/Temporary/Destination explicitly in
conversion records without increasing their sizes. Destination preparation
checks the same selected constructor, conversions, destructor and virtual ABI
demands but does not allocate a second entity/scope for storage already owned
by the initialized object. A conversion-function source prvalue still owns its
required temporary and cleanup. Typed lowering rejects a destination record
without a destination; ordinary value conversion requires an actual temporary.
Source recipes remain immutable and per-use preparation remains terminal.

The [destination campaign](../student.tests/pa14/destination-performance.md)
adds 882 observations, bringing the ledger to 18,606. Both affected K=128/M=8
shapes remove exactly 1,024 entities and scopes and reduce measured RSS in both
campaigns. Selection and source/use budgets remain unchanged. All 12 executable
images are identical. Compiler text grows 1,152 bytes (0.0841%); all measured
record sizes are unchanged. Timing regressions, A/A spread and large native
outliers remain reported, with no compiler/native speedup claim. This resolves
the measured redundant allocations without waiving a correctness requirement
or inventing a numerical O0 gate.

The 73 validation groups pass both required gates, inherited controls and new
destination/lifetime probes. A storage-exhausted sanitizer link is preserved;
lossless compression and an identity-checked resume completed the remaining
checks before timing. The [handoff](../student.tests/pa14/destination-handoff.json)
retains five incorrect source default/query outcomes under the final binary.
Those owners remain the next required work; the full-stage audit is open.
