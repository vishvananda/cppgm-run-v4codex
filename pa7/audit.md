# PA7 implementation audit

Implementation reviewed locally at `6690406f2`; the independent-review marker
in plan.md remains the stage base. This records the current implementation,
not a claim of an independent Ralph review.

## Contract and ownership

The driver adds `--emit-semantics` to the existing shared frontend. PA5 and PA6
remain adapters on that frontend. Each input is a separate TU: preprocessing,
identifier interning, parser scopes, source graph, canonical types/entities and
semantic side arrays are released at TU end. No reference compiler, host
compiler or serialized intermediate representation implements required output.
The only host compilation is building/testing this compiler itself.

The parser streams tokens from the inherited cursor, retains source locations
and literal data in its existing arena, and calls the semantic consumer once
per declaration region. Expressions, statements and template declarations
extend that same source graph by NodeId. No syntax-to-semantic tree copy,
source-token replay or abandoned speculative graph was added.

| Owner | Facts and data flow | Complexity / validity |
| --- | --- | --- |
| `type_builder`, `model` | Source declarator -> canonical source/function/member-pointer type; top-level parameter cv adjusts only the function signature. | Interned TypeIds and cached signature facts; structural work proportional to the declarator and distinct types. |
| `lookup`, `overload` | Scope/name indexes and explicit using edges -> immutable overload-union edges -> candidate EntityIds. Family/signature index finds redeclarations without scanning unrelated functions. | Visit lexical/required imported scopes and candidates; scratch deduplication removes repeated paths. Each new declaration creates a new overload binding; using declarations retain their original binding. |
| `conversion`, `operators`, `expression` | NodeId -> reference-free language type/category, original dump type, constant facts, selected declaration and conversion sequences. | Analyze each node once. Qualification walks only type depth. Arity filters precede conversion classification; a linear tournament plus verification finds a candidate better than every other viable candidate. Expected failure is a compact Conversion result. |
| `statement` | Source-order declarations -> block/condition/substatement scopes; explicit loop/switch context and return target. | One statement traversal; no global declaration scan or scope snapshots. |
| `member` | Class/member identities -> canonical implicit-object call type; object -> constructor action containing object, constructor and address TypeId. | Actions are typed records, not fabricated frontend nodes. Selected member bodies enter a deduplicated queue; processing a body may append its constructor dependencies. No unrelated body is analyzed for a PA7 class completion. |
| `template_call` | Retained function declaration and parameter environment -> interned TypeId argument pack -> specialization identity. | Key includes pattern identity (which owns its environment) and canonical arguments. Each declaration is substituted once; success/failure is retained, emission demand is separate. A TU-owned dependence fact skips closed type subgraphs; a local TypeId cache shares repeated dependent type structure. |
| `resolved_output` | Source order plus recorded semantic facts/demand order -> deterministic dump. | Output-only view; no semantic lookup, overload selection or parsing while printing. |

The function-template support here is the contract's declaration/address and
call-type selection intake. General template bodies, class-aware call semantics,
ABI lowering and native emission remain later assignment work; this stage does
not claim to implement them. The retained patterns and separate demand records
leave those phases explicit.

## Traces and audit findings

For `int (*fp)(int)=choose`, the source declarator creates a canonical pointer
whose child is the normalized function type. Lookup returns the visible
immutable overload binding. Conversion classification selects the matching
function EntityId; committing that conversion records the selected function on
the existing name expression. Printing consumes those facts. No spelling is a
semantic key and no second parse occurs.

For a two-argument call used as a return value, the call owns a contiguous range
of argument conversions. Its parent return conversion has a separate incoming
index. The API check verifies that attaching the latter preserves both argument
facts. This audit caught and fixed their previous shared-field ownership.

For `consume(static_cast<void(*)(int)>(&target<int>))`, the explicit type argument
becomes a canonical argument pack for `target`. Its function type is substituted
once from retained typed parameter structure. `consume` deduces its argument
from that resolved function-pointer expression and gets its own specialization.
Repeated uses return the same two specialization identities. The API and fixed
benchmark check reuse; declaration success/failure and emission demand have
separate states. No grammar is replayed and no template body is copied.

For `&C::used`, target selection records the cv-correct member function and its
implicit-object call type. Demand analyzes the retained selected body once and
can enqueue a default constructor required by a local object. `C::unused` is
left dormant. The API includes an invalid name in that undemanded body and
verifies that it is neither analyzed nor emitted. A queue cursor makes repeated
finish calls idempotent without rescanning completed demand.

Anonymous types and storage have stable typed identities; their synthetic
labels are output views and are not inserted as source lookup bindings. Member
pointer equality includes the owning class and function cv qualifiers. Ordinary
source expressions keep their source spelling even where using directives or
namespace aliases resolve them to a differently qualified declaration.

## Verification and performance boundary

- All 186 unchanged PA7 fixtures pass, including successful dump equality and
  required rejection exit statuses; the through-PA7 report passes 684/684.
- 29 personal cases and a multi-primary isolation case run explicitly.
- ASan/UBSan/leak checks pass all PA7 fixtures, the typed PA7 API and the inherited
  PA5/6 APIs. Inherited personal checks pass (PA5: 10 core, 15 extended, 19 audit;
  PA6: 57 cases), and all 105 PA6 fixtures also pass under sanitizers.
- File audit passes across 79 files. No course test, reference, harness,
  discovery rule, stage handout or root specification was edited.
- Compiler observations and budgets are tracked by `student.tests/pa7/benchmark.py`
  and the performance record. No optimizer or executable is produced at PA7;
  generated runtime/text and native optimization profitability are N/A.

The final audit also added a canonical type-dependence cache: substitution now
returns a closed type directly and transforms only dependent structure. The
cache is valid for the TU lifetime because types and template-parameter identities
are immutable. A work counter bounds first-time dependence analysis by distinct
canonical types.

Performance completion remains subject to the frozen campaign verifier; see
performance.md for its final outcome rather than inferring speed from test
counts or IR size.
