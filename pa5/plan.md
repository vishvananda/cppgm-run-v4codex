# PA5 implementation plan

Stage base commit: `a27ec8877221e4d9acea5f2f63b97855cc0fd365`
Last reviewed commit: `a27ec8877221e4d9acea5f2f63b97855cc0fd365`
Target: **PA5 full-stage**. Phase: **implement**. Entry: **0/188**, 188 failures.

## Design and remaining groups

| Owner | Data flow and scope | Complexity / validation |
| --- | --- | --- |
| Syntax storage/cursor | PA4 streaming PP → PA2 post tokens → compact syntax cursor → TU arena nodes; source locations and interned IDs survive; dump is a view | Linear token/node work; bounded ambiguity checkpoints, no copied streams; API ownership and full AST fixtures |
| Declarations/declarators | Type specifiers, pointer/array/function shapes, initializers and functions on the same graph | Parse common prefixes once; spec 100 and declaration general cases |
| Expressions/statements | Precedence, calls/casts/new/lambda, control flow and exception syntax | Linear syntax construction; expression and statement fixtures plus personal precedence checks |
| Syntactic names/templates | Scope-indexed type/value/template categories, namespaces/classes/enums, template syntax, angle and declaration ambiguity | Compact identities and lexical fallback only for unknown names; spec 200/300 and related general fixtures |
| Driver/rendering | Separate TU owners in command-line order, deterministic AST view and real failure exits | Full `make test-pa5`, through report, file audit |

No semantic graph copy, serialized interphase transport, host/reference delegation,
or per-node owning pointers. Later canonical semantic facts attach to these IDs;
template bodies remain parsed nodes. No later semantic/backend surface is claimed.

## Performance evidence

Record compiler latency/peak RSS, work counters and node/token growth on fixed
personal declaration, expression and template workloads. Generated runtime/text
are N/A at PA5. No optimization benefit claimed without frozen A/B, A/A and ABBA
observations and equivalent output. Initial scaling budget: 4x source <6x latency
and <5x RSS (plus 1 MiB startup); syntax work linear in consumed/produced nodes.

## Validation and handoff ledger

- Entry inspected: clean HEAD above; prior report 205/205; PA5 0/188 because
  `--emit-ast` is unimplemented; prior file audit passes. No previous PA5 goal
  turn with work is present; this is the first implementation entry.
- Remaining: every implementation group above; no handoff boundary yet.
- Required final checks: `make test-pa5`, `make test-report-through-pa5`,
  `perl scripts/cppgm_file_audit.pl --stage pa5 --paths dev/src`, personal tests,
  clean committed worktree. Preserve review markers during implementation.
- Core increment: streaming ring cursor, flat AST/name storage, structured names
  and template arguments, declarations/declarators, expressions/statements,
  classes/enums/templates and AST driver. Full PA5 **131/188** (57 failures);
  personal core 10/10; file audit 61 files; whitespace check passes.
  Remaining shared owners: qualified-name/context prediction; special members;
  dependent template argument expressions; class-wide category availability;
  exception/declarator suffixes and presentation details. Continuing work.
