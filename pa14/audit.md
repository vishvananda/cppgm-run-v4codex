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

These fixed-invalid inputs still succeed:

```cpp
template<class T> void f(){int* p=42;} int main(){}
template<class T> struct C{int* p=42;}; int main(){}
template<class T> int* g(){return {42};} int main(){}
```

Binding initializer operands does not validate initialization. The source owner
and concrete consumers need checked initialization/list recipes; a default's
parameter-slot cache cannot own these. Fixed condition conversions and query-only
calls also need reuse, complete-context keys and temporary/destructor obligations
reviewed. These are remaining implementation work, not a waiver or blocker.

## Performance, acceptance and validation

[Body performance](../student.tests/pa14/body-audit-performance.md) preserves
532 new observations with paired spread, A/A calibration, RSS, native runtime/text,
compiler growth and layouts. All 15,848 earlier observations remain. No optional
runtime transform is added and no runtime-profit claim is made. Source checks
and concrete object/lifetime work have separate budgets; generated growth is
zero on comparable inputs. Historical diagnostic targets do not create extra
PA14/O0 exit gates.

The body change passed 51 checks: file audit/root report, 349 release/sanitizer
and 349 entry/current comparisons, 35 existing native programs, both builds'
64 statement and nine repeated body controls, inherited failure/rejection/demand
suites and PA13 ABI/lifetime controls. All 1,266 fixture/reference hashes are
unchanged. Root count: 1935/1935, all 14 stages. Remaining findings prevent closure.
