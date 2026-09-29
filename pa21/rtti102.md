# RTTI implementation and evidence (loop 102)

## Ownership and design

Formation queries own operand type/category, overload selection, completeness,
access and cv checks. Static `typeid` keeps its operand unevaluated; polymorphic
glvalues promote only that operand to evaluated work. Explicit function template
names demand signatures, and evaluated calls demand bodies. `typeid` is an
lvalue of the declared `const std::type_info`; comparison overloads are checked
before using the standard library intrinsic. No substitute declaration is made.

`RttiExpression` retains canonical types, dynamic/static selection and the cast
hint. Lowering consumes those facts and the existing object/conversion records.
It emits typed LowIR globals, addresses, loads, calls and branches directly.
The ABI graph owns mangled type/name identities. TU caches own RTTI records and
linkage/incompleteness properties; the program linkage table merges external
identities. Nested pointer qualifiers and member-function signatures survive;
top-level object/array cv and reference qualifications do not change identity.

Incomplete RTTI records and their pointer chains have internal binding, while
external type names retain weak binding. A complete definition in another TU
cannot be preempted by a placeholder. General `type_info` reference comparisons
use these canonical name addresses; direct queries in one TU share the actual
RTTI object. Inline-function local types retain their external ODR identity;
anonymous-namespace and static-function types stay local.

Dynamic casts use checked single-inheritance reachability, null propagation and
ABI source/destination RTTI with the offset hint. A statically nonpublic base
path evaluates its operand and fails: in single inheritance neither success
condition in N3485 [expr.dynamic.cast]/8 can hold. PA22's multiple-inheritance
owner must replace that shortcut with a crosscast search. Private base metadata
uses VMI RTTI even at offset zero; SI is only valid for a public base. The
[Itanium RTTI contract](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#rtti-layout)
also defines the incomplete-record and pointer-chain requirements.

The adjacent allocated-copy fixture now retains and calls its selected complete
copy-constructor entry. The decision is stored on that allocation; lowering uses
the existing construction recipe. Parser type-id prediction admits function and
member-pointer declarators without speculative parsing. Shared unqualification
now treats array element cv consistently with qualification.

## Work bounds

One source parse; indexed query/specialization facts; no source or IR text replay.
RTTI records emit once per canonical type and external ABI identity. Cached
linkage and incomplete-pointer properties visit type/dependency edges once per
TU. Class casts traverse the existing inheritance path, and each dynamic query
adds a constant number of LowIR operations/blocks. Mangling and emitted name
bytes are charged to output size: a chain of N pointer RTTI records necessarily
contains quadratic total name bytes, though it has only N pointer records.
No optional optimization or new numerical acceptance gate is introduced.
Compiler latency/RSS and checked native runtime/payload evidence is recorded by
`student.tests/pa21/benchmark102.py`; the final measurements accompany the plan.

## Validation boundary

`rtti102.py` exercises formation, rejection, demand, identity, qualifiers,
conversions and execution. `host_rtti102.py` independently feeds the same
student-generated LowIR into the supplied object backend and links the host C++
runtime. It also checks separate and combined translation units, incomplete vs
complete RTTI, and the reduced ABI name. The reference tool never compiles the
source for these execution checks.

One supplied freestanding-runtime limitation remains observable: with
`B` public in `C`, and `C` private in `D`, a `B*` obtained from `D` must downcast
to its `C` subobject. N3485 [expr.dynamic.cast]/8's first condition succeeds;
accessibility between `C` and the most-derived `D` does not enter that condition.
Our VMI RTTI and call pass unchanged through the supplied object backend and host
runtime. The supplied freestanding runtime returns null. Both results, with the
same LowIR hash, remain in the validation evidence. This is a runtime observation,
not a waived compiler test or a reason to emit incorrect SI metadata. Our native
runtime implementation belongs to PA24 and host-object emission to PA25 onward.

The separately documented [oracle correction](reference-corrections102.md)
changes only a proved wrong substitution digit and matching symbol metadata.
Capturing-lambda RTTI compositions still stop at `unsupported value capture`;
they require the closure-construction owner. Initializer-list storage and EH
cleanup continuations remain distinct unfinished groups, not audit questions.
