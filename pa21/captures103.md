# Closure environments and construction (loop 103)

## Owner and data flow

The semantic closure record owns the declaration, call operator, enclosing
function, lexical parent and capture default. `(closure ID, object ID)` indexes
capture edges. Each edge retains its storage field, source type, forwarding
edge, capture mode and checked initialization conversion. Identifiers and types
remain interned; neither names nor rendered LowIR are lookup keys.

Explicit and implicit value captures share this path. A reference-to-object
capture copies the referred object; a function reference remains a reference.
Nested captures select either the outer closure field or the original referred
object, preserving the outer operator's cv qualification. Evaluated uses retain
the selected capture edge and field type, so `mutable` controls modifiability.
An ordinary implicit member access through captured `this` retains the receiver
projection separately from its selected field projection in O0 LowIR.

Parenthesized `decltype` applies hypothetical capture cv without creating a field
or demanding storage. Retained template lambda bodies keep indexed default and
explicit capture modes on their semantic function identity. Substitution consumes
the qualified type recipe; it does not replay lambda grammar. Unparenthesized
`decltype` still names the original declaration's type. These rules follow the
checked-in N3485 [expr.prim.lambda]/14–18, 20–21 (`doc/n3485.txt`).

Copy initialization is prepared after checking the retained lambda body, under
the enclosing expression's demand owner. Thus evaluating `[object] {}` invokes
its selected copy even when the call operator is never used; merely forming an
undemanded enclosing operator does not emit its nested copies. Constructor
selection, accessibility/deletion, default arguments and destructor dependencies
are recorded before lowering. The shared generated-copy path now considers
constructor templates and retains selected default conversions. Implicit copy
signature constness is determined by subobject **copy constructors**, separately
from the larger constructor candidate set used by memberwise initialization.

Lowering projects the recorded source and destination and consumes the existing
typed conversion plan. It creates no replacement source expressions and performs
no overload selection. Scalar, class and array captures use the same checked
conversion machinery; array expansion stops at eight elements, then uses a
counted loop. Class-copy default arguments are applied per element, including
ending their temporary lifetimes before the next element.

## Ownership on exceptional exits

A successfully constructed capture adds a function-local partial-object lifetime
state. Arrays additionally retain the count of completed elements, incremented
only after successful construction. An unwind destroys that prefix in reverse
order and retains the caller's earlier live objects. When construction succeeds,
retirement removes only the partial-object states; an indexed traversal preserves
other full-expression temporaries and branch choices. Old states remain immutable
because previously emitted unwind edges still refer to them. Ordinary closure
object ownership then passes through the existing object/argument lifetime path.

Generated closure copies also install cleanup for completed member subobjects,
including empty subobjects with observable destructors. Array copies retain a
single completed-element count and reverse cleanup loop. These changes extend
the existing generated transfer owner; they also apply to equivalent ordinary
class transfers. No source `try`/handler state or exception-object semantics is
invented by this work.

## Work and storage bounds

One capture edge per closure/object pair; expected O(1) indexed lookup. Nested
capture forwarding and hypothetical cv inspect required lexical lambda edges.
Each initialization conversion is prepared once for its capture; emitted work
tracks fields and initialization/default dependencies. Closure numbering retains
the existing final source-order sort. Call operator syntax is parsed once.

Partial construction states, conversion records and capture arrays use dense
TU/function-owned vectors. Lowering retirement visits each reachable state/edge
once using a local index, and never mutates existing unwind snapshots. Array
construction/copy/destruction uses bounded expansion or constant-size loops.
Function-local cleanup states are reset at the function boundary; semantic
capture/query indices die with the TU. No process-global cache or textual
transport was added. Performance evidence is in [performance103.md](performance103.md).

## Validation and boundary

[Required checks and personal evidence](../student.tests/pa21/validation103.json):
64 capture execution/rejection controls, 58 inherited reference-capture controls,
and 13 host-unwind controls pass. `capture_eh103.py` host-compiles only the external
throwing test harness. Student-generated, validated LowIR passes through the
supplied object backend; that backend never compiles the student source.

The completed behavior group covers closure environment formation, direct capture
initialization, generated copies, lifetime transfer and partial construction.
All standalone capture fixtures and their RTTI/template compositions pass.
Two source-handler lambda compositions remain in the **unfinished EH group**:
`200-default-reference-capture-local` and
`200-function-template-lambda-decltype-eh-fallback`. Source `try` statements still
reach the semantic checker's unsupported-statement path. Completing them requires
source handler/exception-object facts and typed continuation contexts, including
catch-parameter binding; closure field or copy changes cannot supply those facts.
The other EH/lifetime and initializer-list fixtures remain required work.
Independent whole-stage review is still owed; this document does not certify it.
