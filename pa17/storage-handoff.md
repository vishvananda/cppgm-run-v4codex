# PA17 static storage implementation — loop 59

Entry `7cc89281`: 333/343. Implementation `6a1a51a8` and documented oracle
corrections `3c7eeea2`: 340/343. Seven original failures close; no course input,
exit-status expectation, coverage or comparison rule changes. The six reference
corrections have [standard proofs and reduced observations](storage-references.md).
This is an implementation handoff; independent stage audit remains required.

| Owner | Fact flow | Complexity and lifetime |
|---|---|---|
| Static member definitions | Retained source head and declaration prototype → canonical type/head signature → selected definition link → entity-keyed storage demand → instantiated definition. Static definitions now use the same declaration signature index as functions. Duplicates, mismatched types and nonstatic out-of-class definitions are diagnosed, including undemanded templates. | Source signature computed once per immutable prototype bucket. Definition application keeps the existing `(specialization, source)` monotonic state and precise demand queue. One static member cannot accumulate multiple valid definitions. TU-owned flat indexes and compact records. |
| Dependent signature queries | Parameter substitution → canonical head ordinals → signature-only query identity → declaration match. Access environments are omitted only from the signature representation: the bound operand entities and canonical types remain. Concrete type checking substitutes the original query with its original context. | Memoized traversal of query/type edges per signature; no text key, parsing, global retry or negative cross-context semantic cache. The interned signature graph belongs to the TU and is not an instantiation input. |
| Read facts | Ordinary O0 reads retain constants already published at that source use, before their storage demand attaches a definition. A substituted expression establishes its fact at instantiation after required demand. Both ordinary names and object-member reads record the chosen constant; member lowering consumes this snapshot instead of consulting the eventual entity value. Explicit constant evaluation retains its existing demand path. | Constant-time snapshot publication and existing entity-keyed demand. No later whole-function scan, mutation of earlier expression facts or repeated lowering-time evaluation. Preserves PA14's instantiated unqualified constant read and PA17's ordinary first read. |
| Function addresses | An address query retains an unresolved overload family for its target conversion, rather than manufacturing a pointer to unknown type. A single non-template declaration yields a typed ordinary or member function pointer; access and deletion are checked. Target conversion records the selected function, and static initialization emits its relocation through the ordinary LowIR path. | Single-declaration fast path is O(1); unresolved families use the existing candidate sequence and conversion machinery. Source recipes remain shared; selected function demand is established for concrete uses. |

The neighboring defects were exercised together: template-local aggregates,
overloaded and template addresses, `decltype` function addresses, nested heads,
renamed parameters, dependent arrays, constexpr redeclarations, mutable and
volatile reads, independent local guards, and source-order definition changes.
All 37 added controls pass, together with all 530 inherited controls. The entry
binary's results for the same sources distinguish new fixes from existing
behavior. A preliminary 25-case entry result remains preserved.

N3485 §3.2/1 forbids multiple definitions in one translation unit; §9.4.2
separates an in-class declaration from a namespace-scope static member definition;
§14.5.1.3 governs template static member definitions. Function-address typing
follows §5.3.1/3 and §13.4. The reference proof supplies the initialization and
unused-instantiation rules. No compiler is used to implement compiler output.

The [trace](../student.tests/pa17/storage-trace.cpp) combines a nested template
static member, dependent `decltype`, a constant function address, and two
template-local tables. Source parsing retains the template graph; canonical
head identities match the out-of-class definition; original substitution frames
establish access and concrete pointer types. The declaration owns storage and
relocation demand. Typed lowering consumes those facts and emits each table and
target once. The explicit writer/backend adapters validate and execute the
output outside the compiler; own native emission remains a later-stage owner.
The evidence manifest retains source/LowIR/ELF hashes, telemetry and runtime.

The group is complete at the storage-definition, constant-initialization and
read-fact boundary. The remaining two cleanup cases require exceptional
call-region placement and temporary/automatic cleanup scheduling. The lambda
case requires closure entity identity, call operator/body binding, and class
value/lifetime lowering. Those owners cannot be supplied by further storage
definition or relocation changes; all three are unfinished implementation.

Independent review must still examine signature-only context normalization,
source versus instantiated read snapshots, and address-query access provenance.
The previous active candidate keys, query failure-cache validity, immediate
context restoration, base-graph keys, qualified receivers and O0 work/growth
questions remain open. These review questions do not waive the implementation
cases or certify whole-stage architecture.
