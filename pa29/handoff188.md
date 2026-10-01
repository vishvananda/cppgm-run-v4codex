# Implementation188 — contextual coroutine grammar and expression boundaries

Entry `c1caa0fc6b2985a633c4db587097ac5c08c30c75` had **394/403** PA29 passes.
Implementation `e52f4586`, refined by `bf8bc00f`, has **395/403**.
`700-hosted-coroutine-contextual-operators-compile` now passes. All other entry
passes remain, with the same 403 inputs and unchanged references/comparisons.
This completes an implementation behavior group, not PA29 or its independent audit.

## Ownership and data flow

| Owner | Facts and consumers | Work/lifetime |
|---|---|---|
| Parser contextual classification | An existing lexical declaration keeps the C++11 identifier meaning. Otherwise retained template bodies recognize await, yield and coroutine return. Distinct arena nodes preserve operands, precedence and locations. Concrete coroutine syntax retains PA5's required rejection. | Existing interned names and lexical scope edges; one grammar parse and one node per operation. Depth and interned contextual IDs have parser lifetime. |
| Complete-class name index | Later declarators, including comma lists, parenthesized declarators and unscoped enumerators, are available in earlier member bodies. Constructor initializers, attributes, operator spellings, using names and template arguments do not introduce spurious value bindings. Member variable templates retain their template category. | The existing class-region pre-scan visits prefixes and balanced delimiter ranges; no AST construction/replay or whole-program lookup. Existing indexed angle/delimiter results are shared. Storage is ordinary TU name entries. |
| Retained template binding | Operand names and fixed expressions are checked at definition time. Await/yield's promise protocol remains dependent; coroutine return does not acquire an ordinary return conversion. Defaults, member initializers and unevaluated operands cannot borrow an enclosing coroutine-body context. Nested bodies restore their own context. | Existing per-body monotonic binding state and dependent-node facts; one scoped boolean, no cache, cloned graph, rendered key or per-node allocation. Context checks visit lexical parents only. |
| Shared expression grammar | Yield and throw take assignment operands and permit an outer comma expression. Throw also enters that path in statement position. Existing semantic and LowIR exception/lifetime facts implement the resulting tree. | Constant parser work per expression edge. No new lowering pass or optimizer. |

The throw correction follows N3485 [expr.ass] §5.17's assignment-expression,
[expr.comma] §5.18's expression grammar, and Clause 15's throw-expression grammar;
see [the checked-in text](../doc/n3485.txt). `throw x, wrong()` throws x and cannot
execute the right operand. Both standalone and parenthesized forms, rethrow,
template demand and destructor effects are exercised by `throw-comma.cpp`.
No reference correction or bundle revision is involved.

Await/yield are never lowered to an ordinary call or fabricated result. Demanding
one reaches an explicit unsupported promise/frame diagnostic before emission.
The PA29 fixture requires only retained template syntax and ordinary identifier
execution; actual coroutine runtime support would need promise selection,
suspension/resumption, frame ownership/destruction and ABI/lifetime lowering.
That is a distinct implementation boundary, not a claim that those facilities exist.

## Validation and inspection

- PA1–28: **4538/4538**, exit 0.
- `make test-pa29`: **395/403**, exit 2, exactly eight unchanged entry failures.
- `make test-report-through-pa29`: **4933/4941**, exit 2, PA29 alone fails.
- File audit: pass, the same four inherited substantial-header warnings.
- **61** explicit control commands: positive compilation/link/run, host GCC/Clang
  identifier and throw agreement, AST grouping assertions, validated LowIR,
  symbol absence, and **22** required rejections (one AST plus 21 semantic/source).
  The dormant coroutine bodies emit no symbols; throw tails contain no call to
  `wrong`. Earlier PA5 concrete-coroutine rejection is explicitly preserved.

[Controls](../student.tests/pa29/check188.py) keep their inputs under
`student.tests/pa29/controls188/`; generated artifacts stay in scratch.
The evidence manifest binds final source/binary hashes, stage failure delta,
all 1,707 inherited contract/harness path hashes and the validation logs.
No test, reference, discovery or comparison rule changed.

## Performance and boundary

[Performance188](performance188.md) reports frozen O0 A/B comparisons with A/A
calibration and six ABBA blocks for common workloads and the affected class-name
index, plus corrected-only coroutine scaling. Compiler time/RSS and executable
runtime/text are reported together. All 880 observations and 16 launchers are retained. No runtime
speedup or optimization benefit is claimed; this change adds required syntax and
correct lookup/exception behavior, with no optional transformation or growth policy.
Mandated evaluator/frame/alignment/time limits remain unchanged. Historical
blanket 15%/zero-growth targets remain diagnostic under spec §9, not exit gates.

The initial investigation also traced the vector failure to absent type-operand
builtin support (`__builtin_convertvector`, `__builtin_bit_cast`), reduction,
vector comparison and executable vector storage/ABI. The existing canonical
lane deduction is not the missing contextual-grammar operation. Implementing that
owner needs new semantic intrinsic and representation/lowering work, rather than
another extension of the completed contextual-expression path. Binary128/half
also need new numeric representations and operations. Further unrelated work is
therefore left explicitly unfinished in the compact plan and failure ledger.

Independent review still owns the inherited nothrow shorthand, invocability and
nested ABI-tag contract questions. Primary `char_traits` remains unfinished with
its inherited contract discussion. None is resolved by a waiver or a special
library-name implementation. Review markers and all previous evidence are retained.
