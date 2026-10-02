# PA30 implementation201 ownership

Entry `b0790726de6c67722826833b395b76853e051e2d`, frozen before edits.
Stage/review markers remain in plan.md. Entry is 148/153 (five failures);
Ralph's cached 148/154 (six) is retained in evidence201/entry.json.

| Group / owner | Data flow | Complexity / lifetime | Validation |
|---|---|---|---|
| Automatic object use / semantic closure_capture.cpp | Selected declaration and evaluated-use context → ordinary function boundary check or explicit closure capture chain → recorded object-use fact → existing storage lowering | O(lexical depth) per use, required enclosing scopes only; captures indexed by closure/entity, existing TU owner. No new cache, retry, parse or representation. | Illegal local-class reads, parameters, constant addresses/references, template bodies, captures crossing ordinary members; constants, sizeof/decltype, static storage and nested legal captures. |
| Fallthrough / typed LowIR function completion | Function-local blocks and selected call boundary facts → one reachability worklist → reject reachable fallback for non-void functions except global main | O(instructions + edges), one visited bit per function-local block; scratch released on return. No IR rewrite, semantic reconstruction or optimizer. | Conditional returns, literal loops, breaks, goto labels, switches, EH, template bodies and noreturn calls; full prior report. |
| Noreturn / parser native attributes, semantic entity, LowIR signature | Parse attribute once → declaration identity, inherited specialization attribute → signature return contract → reachability and backend | Constant work per attribute/publication, existing source/TU/signature owners. No spelling-based lowering or whole-program search. | Both standard and GNU spellings, declarations/definitions, specialized/member calls, preserved exceptional exits. |

Initial CFG checking exposed an inherited missing noreturn fact:
the standard/GNU attributes were parsed but discarded. The completed group includes
that repair to preserve control convergence and hosted regex. Independent review remains separate
from implementation and is not waived.

The group extends through defaults: checking a deferred default uses a dormant
evaluation context, not the requesting function's automatic storage. Local
reads/captures are rejected there, while constants and unevaluated queries
remain valid. A lambda in a default is potentially evaluated; its body owns a
new function context, and nested captures of that body's locals remain valid.
A closure records its default-argument boundary once. Actual sizeof/decltype
operands remain unevaluated and reject C++11 lambda expressions.

The CFG proof does not rewrite code or run constexpr evaluation. A bounded
single-definition integer proof handles literal arithmetic/comparisons/casts;
unknown values retain both edges. Parent-linked EH region identities assign
only actual throwing-call/resume edges, preserving noexcept and unreachable
handlers. Noreturn kills normal continuation, independently of throwing.
Isolated joins require no further analysis. All work is function-local and
linear in emitted instructions/edges (a fixed number of passes, no fixed point).
Existing telemetry now records fallthrough functions, instruction visits and
edge visits without triggering analysis. Source/LowIR/object controls and
scaling evidence exercise the exact facts used by object emission.

The allocation exception expectation is resolved by the documented
[reference correction](reference-correction201.md), without changing compiler
compatibility rules. Only that exit-status sidecar changes; its source, negative
companion, inventory and comparison rules remain. Positive hosted replacement
new and dynamic-set redeclarations have explicit compile/runtime controls.


## Trace and final handoff boundary

`local-template-valid.cpp` traces demanded `f<int>` through its retained body,
concrete local-class method, bound constant/sizeof uses and nested capture
edges. Selected declaration identities and capture storage reach the existing
typed LowIR object/call path and direct ELF emission. `flow-noreturn.cpp`
traces both parsed attribute spellings and a demanded template through the
semantic no-return flag, function-signature return mode, selected call and
normal/exceptional CFG edges. The throw/catch runtime controls establish that
noreturn does not imply noexcept. `local-default-nested-lambda.cpp` traces a
completed default fact through a default-owned closure and a legal nested
capture of that closure body's local object. Actual unevaluated operands and
invalid enclosing automatic/this captures remain rejected.

The 101-command trace includes ten LowIR validation/roundtrips, object rebuilds
from serialized IR and linked executions, plus symbols, disassembly and unwind
frames. Telemetry changes no emitted object. All 111 current and 245 inherited
control commands pass, as do 4941 earlier required cases and file audit. Current
PA30 is 151/153; the through30 report is 5092/5094. The verifier binds all evidence
to the final implementation. This is a validated implementation boundary, not
whole-stage certification.

The initial local-use/return work expanded through capture-chain boundaries,
default contexts, nested default lambdas, noreturn publication, integer CFG
proofs and exceptional reachability; all known defects in these owners have
focused controls and final reports. Remaining random failures need packed
intrinsic signatures, saturation/lane semantics and subsequent header operations;
general vector subscripting additionally needs lvalue/storage lowering. Those
are separate operation and representation owners. More changes to the completed
function/context/CFG facts cannot supply their missing semantics. Opening that
new vector implementation group here would not be further related work. Both
remaining requirements stay explicit implementation obligations in plan.md and
pending.json. Independent review of implementations199–201 remains for Ralph's
scheduled audit; it is separate and is not waived.
