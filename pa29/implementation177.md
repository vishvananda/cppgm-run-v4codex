# Implementation177 — selection scope, lifetime and branch demand

Entry: `d69d57fc28bfc308f8223e3090fb9e9ab37c4823`, **380/403** course tests.
Stage base and Last reviewed commit remain unchanged in [plan.md](plan.md).
This is implementation evidence, not an independent review.

## Contract and ownership

The course control-flow fixture requires declaration and alias initializers
before selection conditions. These are hosted extensions to C++11. Their scope
and execution follow [N4868 stmt.if/2–3](https://timsong-cpp.github.io/cppwp/n4868/stmt.if)
and [stmt.switch/7](https://timsong-cpp.github.io/cppwp/n4868/stmt.switch).
Alias initializers are explicitly exercised by the PA29 fixture; they are a
further extension beyond that draft's initializer grammar.

The parser consumes each initializer/condition once. The delimiter after the
shared declaration/expression prefix decides whether it is a selection
initializer or the condition. A `SelectionInit` source wrapper retains its
actual child declaration/expression; a condition uses the established condition
declaration shape. No token replay or synthetic expression is introduced.
`using` aliases, typedefs, multiple declarators, direct/list/copy initialization,
empty statements and expression statements share the selection's lexical scope.
A condition keeps the separate restrictions from
[stmt.pre/4,8](https://timsong-cpp.github.io/cppwp/n4868/stmt.pre): one initialized
variable, no array/function or class/enum definition, and only type/constexpr
specifiers with brace/equal initialization.

Template definition binding retains declarations, dependent names and condition
conversion recipes. Concrete semantic checking resolves initialization before
the condition, records its selected conversion and publishes constexpr branch
selection once. Discarded dependent substatements are not instantiated. Ordinary
non-template discarded substatements are still checked; their return statements
do not contribute to placeholder-return deduction. The discarded context is
owned by the current function, so a nested lambda's return deduction is separate.

A discarded substatement suppresses emission-only function/member/storage demand,
including deferred use edges. It is not an unevaluated operand: ordinary lookup,
access, deleted-call checks and lambda validation still run. Deduced return types
can still require a callee's body; that declaration-only demand retains dormant
callee-use edges and does not activate runtime emission.

## Lifetimes and data flow to ELF

Selection declarations publish ordinary canonical object entities. The existing
lifetime visitor consumes those entities in source order. Its persistent active
initialization prefixes also prohibit entry into constexpr substatements, even
empty ones. Source jump validation checks non-template discarded branches without
publishing executable cleanup facts for them. Selected branches alone supply
lifetime actions to lowering.

The lowerer initializes the header before evaluating the condition or emitting
the selected constexpr arm. Compile-time condition declarations still create
their required runtime object. Switch fallthrough and an unmatched dispatch
route through one normal cleanup block; `break` targets the already-clean end.
Existing return, goto, continue and exception paths consume recorded lifetime
prefixes. Constructors/destructors are not rediscovered by name during lowering.
Typed LowIR, native MIR and ELF emission remain the existing shared pipeline.

Constant execution uses the condition's recorded conversion, including a class
conversion function. Selection proof runs in manifest constant-evaluation mode.
Selection/loop local storage is retired on scope exit, including early return;
a pointer to an expired header local cannot remain readable in constant execution.

## Complexity and bounds

All new records belong to the existing translation-unit arena or current
function's control state. One source wrapper and constant-size scalar context
records suffice; lexical bindings use the existing compact indexes. Work is
linear in parsed statements, selected specialization statements and produced
operations. Jump checking retains its single traversal plus prefix numbering;
it does not introduce a second whole-body pass. Condition declaration adaptation
reuses the parsed declaration's nodes (two bounded list wrappers become unused),
without a checkpoint or abandoned alternate parse. Constant execution retains
its 1,000,000-step and 512-call-depth limits. No optional optimization, fixed-point
transform, process-global cache, text transport or new owning graph was added.

The explicit controls, inspections, unchanged-coverage record and performance
report are linked from the compact plan. The PA6 declaration-only view is checked
on concrete selection forms. Its inherited inability to inspect a deliberately
incomplete dependent alias is not a PA29 oracle; the same input is checked through
`--emit-semantics`, `--emit-lowir` and native execution. `-g` is unsupported by the
current object driver; this group does not add or claim DWARF line support. Native
unwind tables and exception execution are inspected through the supported path.
