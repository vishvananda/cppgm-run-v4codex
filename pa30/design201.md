# PA30 implementation201 ownership

Entry `b0790726de6c67722826833b395b76853e051e2d`, frozen before edits.
Stage/review markers remain in plan.md. Entry is 148/153 (five failures);
Ralph's cached 148/154 (six) is retained in evidence201/entry.json.

| Group / owner | Data flow | Complexity / lifetime | Validation |
|---|---|---|---|
| Automatic object use / semantic closure_capture.cpp | Selected declaration and evaluated-use context → ordinary function boundary check or explicit closure capture chain → recorded object-use fact → existing storage lowering | O(lexical depth) per use, required enclosing scopes only; captures indexed by closure/entity, existing TU owner. No new cache, retry, parse or representation. | Illegal local-class reads, parameters, constant addresses/references, template bodies, captures crossing ordinary members; constants, sizeof/decltype, static storage and nested legal captures. |
| Fallthrough / typed LowIR function completion | Function-local blocks and selected call boundary facts → one reachability worklist → reject reachable fallback for non-void functions except global main | O(instructions + edges), one visited bit per function-local block; scratch released on return. No IR rewrite, semantic reconstruction or optimizer. | Conditional returns, literal loops, breaks, goto labels, switches, EH, template bodies and noreturn calls; full prior report. |
| Noreturn / parser native attributes, semantic entity, LowIR signature | Parse attribute once → declaration identity, inherited specialization attribute → signature return contract → reachability and backend | Constant work per attribute/publication, existing source/TU/signature owners. No spelling-based lowering or whole-program search. | Both standard and GNU spellings, declarations/definitions, specialized/member calls, preserved exceptional exits. |

Work in progress. Initial CFG checking exposed an inherited missing noreturn fact:
the standard/GNU attributes were parsed but discarded. This group must include
that repair to preserve control convergence and hosted regex, rather than treat
those regressions as separate future work. Independent review remains separate
from implementation and is not waived.
