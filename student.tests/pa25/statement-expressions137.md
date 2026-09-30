# PA25 statement-expression ownership and handoff

The three required GNU statement-expression failures are fixed without changing
fixtures, references, comparison rules or harnesses. The implemented behavior is
based on the [PA25 contract](../../pa25/README.md) and the
[GNU statement-expression documentation](https://gcc.gnu.org/onlinedocs/gcc/Statement-Exprs.html):
a scoped compound body, a final expression value (otherwise void), C++ value
results with decay, statement-local temporary destruction, and control transfers
to the enclosing function/loop. Incoming jumps into a statement body are rejected.

## Ownership and data flow

- `syntax::Kind::StatementExpression` owns one parsed compound region. The parser
  uses its existing block/name machinery; it does not replay tokens or introduce
  an artificial function, lambda, call, or cloned syntax graph.
- `Analyzer::statement_expression` checks the statements in one block scope,
  records the final operand and selected conversion, and creates the ordinary
  class-result temporary identity. Destination conversion recipes reuse the
  existing copy-elision policy. Declaration constructor/destructor effects are
  recorded by the semantic effect owner, including in unevaluated operands.
- The template binder binds source statements once. Fixed expressions retain
  their existing shared facts. A `StatementResult` query owns its source region
  and environment; substitution checks that region's canonical occurrence before
  consuming the result type. It never searches for a replacement declaration by
  spelling. Runtime lowering reuses those checked facts.
- Jump validation visits evaluated expression bodies as well as statements.
  Region identities distinguish entering a body, leaving it, and looping inside
  it. Loop header expressions retain the enclosing jump targets. Initialization
  has separate scope-entry and lifetime-start facts: a jump can leave an
  initializer before its destination starts to live.
- Lowering consumes those statement, conversion and lifetime facts directly.
  A function-local, parent-linked lifetime overlay inserts any surrounding
  expression temporaries below body-local objects. Complete `(overlay,state)`
  keys memoize translated prefixes; unchanged prefixes retain their identities.
  Return/break/continue/goto clean the appropriate prefix in reverse order.
  Sparse initialization guards exist only for destructible objects whose
  initialization can be bypassed by an outgoing jump, including references to
  temporaries. An ended instruction stream cannot receive more instructions.
- Typed LowIR continues through per-function MIR and direct native encoding.
  The object format, linker, native ABI and runtime-helper interfaces are unchanged.

Work is O(source nodes + required lifetime/control edges + emitted instructions),
with average O(1) fact/cache lookup. Overlays and their flat index are released at
the next function reset; semantic facts are TU-owned. No global mutable cache,
whole-program retry, serialization transport, optional pass or unbounded search
was added. Outward transfers visit the prefixes they actually exit. The ordinary
O0 optional optimization/growth budgets remain zero.

## Validation and trace

[statements.py](statements.py) runs 49 explicit controls: scalar/void results,
array/function decay, shadowing/nesting, enclosing and nested returns, all jump
forms, loop-header targets, class copies/result identities, constructor/destructor
ordering, enclosing temporaries, skipped construction, reference lifetime,
`noexcept`, templates, dependent type queries, diagnostics, and separate/direct/
both mixed object paths. The latter paths also compile with an empty tool PATH.
The two rejection controls for scalar lvalue assignment and constant evaluation
pin the documented value-result/nonconstant policy; they are not claims of full
GCC extension compatibility. Exploratory host comparisons found GCC accepts
additional cases there. No required fixture depends on those additional cases.
The initial invalid address-of-rvalue control was corrected to compare `this`
inside a member function; the initial evidence is preserved in artifacts.

[statement-trace.cc](statement-trace.cc) traces a nontrivial declaration and two
demanded template instances through source to ELF. It checks destruction order
321 on both normal and early return paths. With no tools on PATH, it reports:
273 parsed nodes, 473 total occurrence identities, two specializations and two
body transitions, four lowered statement regions, two lifetime mappings and
22 mapping-cache hits, five native functions, 158 native instructions, and 1581
text bytes. It executes successfully. Explicit AST rendering and
`--emit-lowir --validate-lowir` also pass; these are inspection adapters, not
production transports.

The existing 61 driver checks and 19 scalar controls (96 seeded full-width pairs)
remain green. Required PA25 checks improve from 71/101 to 74/101. PA1–PA24 remain
4152/4152; the through-PA25 report is 4226/4253. File audit passes with its four
inherited header warnings. All observations, including intermediate failures,
are retained under `artifacts/pa25-137/`.

## Handoff boundary

This completes the scoped statement/result/control/lifetime behavior group.
There is no known outstanding defect in the covered group. The remaining 27
required failures need native class/RTTI/allocation definitions (12), shared
source-EH matching/unwinding and function-try support (14), or the inherited
floating evaluation policy (1). Source exceptions initiated inside statement
bodies still depend on that same unfinished EH runtime. Completing those groups
requires different ABI/runtime owners; merely extending this statement analysis
cannot supply their missing execution machinery.

Independent review remains pending for the whole stage, including these new
lifetime overlays, skipped-initialization guards, query environments, and terminal
control flow. Review questions are separate from the remaining implementation;
this handoff does not certify PA25 completion or permit advancement.
