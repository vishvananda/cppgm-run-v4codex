# PA17 query/candidate implementation — loop 58

Entry `411ad00e37b421dc1d3d85b5438e010bd099e025`: 330/343.
First code increment `7d04e8ce`: 333/343. No course fixture, reference,
comparison rule, or bundle revision changed. This is an implementation handoff;
independent stage review and the ten remaining course failures are still open.

## Ownership and flow

| Owner | Fact flow | Work and lifetime |
|---|---|---|
| Candidate substitution | Deduction preserves every deduced argument, including positions after a default hole. `deduced_specialization` uses the canonical template head and full incomplete argument tuple to mark active default/signature substitution. Recursive demand observes that mark and discards only that active candidate. A completed tuple proceeds to the existing specialization fact. | One average O(1) flat-index operation per default-bearing candidate, plus O(head width) argument interning; fully deduced tuples need no active index. Scope exit restores only the current key. Marks are not cached negative results and therefore do not outlive the active deduction. Index capacity and interned tuples belong to the TU. No global generation or retry scan. |
| Type queries | Canonical substituted query nodes retain compact invalid-operand, no-viable, ambiguity, and deleted-selection outcomes. Child failures propagate without evaluating consumers or unwinding C++ exceptions. `decltype` returns the substitution failure sentinel without qualifying it into a false concrete type. Completed query/type facts cache their result at the existing query/environment owner. | One computation per canonical query; candidates and associated namespaces/classes use local sequences and flat deduplication. No token replay, AST clone, or LowIR adapter. |
| Immediate context | Candidate signatures/defaults and partial-pattern comparison enable a separate immediate-query mode. This differs from the existing partial type-construction mode at a source definition. Class-definition demand suspends both modes; its partial matcher establishes its own local probe, while a selected class body's errors remain hard errors. | Stack-scoped booleans, restored on all exits. No exception catch converts arbitrary class/body errors into candidate failure. |
| Deleted functions | Source declarations record deletion on the function entity; concrete template declarations inherit it. Operator/call selection checks the chosen entity, retaining deleted candidates for ranking. Ordinary calls and addresses consume the same deletion fact. Deleted definitions must be first, and cannot acquire a later body. Explicit specialization replaces the primary deletion fact along with the primary definition owner. | One entity flag within existing padding; existing member transfer deletion remains available for implicitly deleted special members. No semantic names or emitted strings are keys. |

The recursive fixtures' comments describe the course implementation's shared
ADL scratch/generation bugs. This compiler already has per-call ADL worklists and
candidate deduplication. Its actual reducer stack enters default substitution,
class partial matching, a `decltype` operator query, and the same candidate's
default substitution again. The new active state belongs before defaults, where
no complete specialization key exists yet. It does not skip distinct argument
bindings, subsequent candidates, or later declarations adding a default.

N3485 §14.8.2/8 [temp.deduct] limits substitution failure to the immediate
context and distinguishes class-instantiation side effects. §8.4.3/2,4
[dcl.fct.def.delete] includes selected deleted functions in unevaluated uses,
requires deletion on the first declaration, and makes deleted definitions inline.
See the checked-in [standard](../doc/n3485.txt:20470) and
[deleted definitions](../doc/n3485.txt:10964). No reference correction is needed.

## Validation and boundary

Three original course failures close together: candidate collection reentrancy,
nested ADL, and ambiguous-operator substitution. Personal controls extend those
cases to missing/deleted operators and calls, callable/member queries, cv and
namespace separation, repeated queries, ordinary and hidden-friend paths,
default holes, declaration insertion, and hard definition/body errors. The
accumulated replay retains all 493 previous inputs and adds 37 controls, all
executed explicitly. Entry results distinguish the 22 newly fixed controls from
15 controls already passing; those additions do not replace course progress.

The remaining closure case fails at `resolve_expression` because there is no
semantic lambda implementation: only the parser recognizes it. Completing it
requires source-occurrence closure entity identity, a synthesized call operator,
body/parameter binding and temporary lifetime/lowering facts. Query failure
handling cannot supply those facts. Seven storage cases require publication,
static/dynamic initialization and relocation owners; two cleanup cases require
exception-region scheduling. These are concrete independent state machines,
not additional variants of the now-completed query/default owner.

Independent review remains open for the active candidate key across retained
member heads and partial explicit arguments, query failure-cache validity at
instantiation boundaries, and probe-mode restoration on side effects. Earlier
base-graph/qualified-receiver/performance review questions are preserved. These
questions do not replace or waive any of the ten unfinished implementations.
