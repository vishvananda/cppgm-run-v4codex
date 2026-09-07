# PA3 completion audit

Scope: `pa3/README.md`, `spec.md` as applicable to a controlling-expression
frontend, and the requested full-stage gates. Implementation revision:
`e8cf198c7`; personal harness revision: `4314a6eb3`. Review markers in `plan.md`
remain at the entry HEAD. No later milestone is claimed complete.

## Contract evidence

| Requirement | Authoritative implementation and evidence |
| --- | --- |
| Phases 1–3 and logical lines | Unchanged PA1 `PPTokenCursor`; `ppexpr.cpp` finishes on newline/EOF, skips empty lines and emits EOF once. Personal splices, trigraphs, block comments and CRLF cases pass. Lexical exceptions escape the evaluator and return failure even after a line syntax error. |
| Integral literals and course promotion | PA2 `decode_number`/`decode_character` feed `promote_pp_literal`; no phase-7 token serialization or string concatenation. Original ABI type selects signed/unsigned extension to 64 bits. Primary/Unicode fixtures and API checks of every integral enum pass. Strings, floats, UD and invalid literals are rejected unconditionally, including discarded branches. |
| Identifiers and `defined` | PP identifiers retain TU-interned IDs; only true is 1, others are 0. A small state machine accepts exactly an identifier or parenthesized identifier. Keywords are PP identifiers; alternative operators remain punctuation. The CLI callback checks the first UTF-8 byte. Course defined fixtures plus Unicode/translated names and live macro state API checks pass. |
| Full expression grammar | `PPExpressionEvaluator` uses precedence-ranked unary/binary operators and explicit parenthesis/question barriers. Equal binary precedence reduces leftward; completed ternaries have lower precedence and associate rightward. Missing operands/delimiters, commas, unsupported or trailing tokens fail the line. All precedence/chaining fixtures, 492,075 triple expressions, independent typed-tree cases and deeply nested ternaries pass. |
| Arithmetic and conversions | `expression_value.cpp` computes using uint64 bit patterns; mixed arithmetic converts to unsigned, shifts retain the left type, and comparisons/logicals yield signed 0/1. Signed ordering flips the sign bit; signed division/remainder use magnitudes and explicit signs. Zero divisors, min/-1 quotient/remainder and shift counts outside [0,63] become errors before any hazardous host operation. Right shift sign-fills explicitly. Boundary pairs pass independent Python mathematical-integer evaluation and ASan/UBSan. |
| Short circuit and conditional type | Every syntactic branch is parsed once. An arithmetic error is a typed value fact, and selection propagates only the selected branch's error plus the condition's error. Both conditional arms contribute signedness. No undefined host operation occurs in discarded branches. Course eval-order/cond-ret-type/overflow fixtures and personal nested selection tests pass. |
| Recovery and output | Invalid syntax is sticky until `finish`; the caller continues lexing to preserve phase errors. `finish` clears syntax and stack lengths, with no stale per-line results. CLI renders signed magnitude or unsigned decimal with `u`, `error`, and `eof` directly. Course malformed-line recovery and personal recovery after 200k unmatched parentheses pass. |

## Spec ownership and data traces

For `defined(al\\\npha) ? -7 : u'a'`, immutable source bytes feed the existing
bounded PA1 translation lookahead. `alpha` is interned once; the mock/macro query
consumes that ID without caching. The unary minus operation creates a signed
bit-pattern value. PA2 decodes `u'a'` as char16_t; promotion records uintmax
signedness. The conditional reduces three inline values to the chosen -7 bits
with unsigned result type. Only the explicit CLI view emits
`18446744073709551609u`; no spelling is used as a semantic equality key.

For `false ? 5/0u : -5`, both arms remain grammatical and typed. Division by zero
records an error without executing a host division. Conditional selection drops
that arm's error, retains its unsigned type contribution, and yields unsigned
-5. Invalid tokens or malformed grammar in the same arm still fail the line.
The implementation may compute safe arithmetic facts for unselected arms; this
pure bounded folding has no observable side effects and does not instantiate
or execute language bodies.

The TU owns its immutable source and flat identifier table. PP tokens borrow
source/cursor views only until the next pull; `push` consumes them immediately.
Only stable identifier IDs reach macro lookup. Converted PA2 scalar bytes are
inline temporaries. Values and 3-byte operators reside in geometric vectors;
there are no retained token arrays, parse trees, per-node ownership, deep copies,
recursive destruction, process-global mutable caches or host/reference delegation.
The two scratch vectors clear between lines and release with their evaluator.
Macro state is deliberately queried live, so changes require no cache invalidation.

Each token is consumed once and each operator pushed/popped at most once.
Precedence has a fixed number of levels; flat chains retain two values and one
operator. Unary/parenthesis/conditional nesting uses O(depth) scratch instead of
the host call stack. Names grow only with distinct TU identifiers. API checks
observe zero allocations after a representative line warms the scratch. Optional
`--stats` observes existing bytes, probes, capacities, reductions, phase wall time
and RSS without adding analyses. [Performance evidence](../student.tests/pa3/performance.md)
freezes inputs/binaries/flags, validates output hashes and resource envelopes,
and reports every observation, A/A noise and ABBA paired spread.

Spec sections on declaration graphs, lookup/overloads, templates, semantic demand,
IR, optimization levels and ELF emission belong to later assignments. PA3 creates
no declarations, specializations, IR or executable. No optimization/runtime
benefit or generated-code-size claim is made; generated runtime/text size are
explicitly N/A. Reuse remains direct and typed for PA4's future macro expansion
stream and real `defined` callback.

## Validation and provenance

- `make test-pa3`: 20/20, improving the unchanged entry suite from 20 failures to 0.
- `make test-report-through-pa2`: 80/80; `make test-report-through-pa3`: 100/100.
- `python3 student.tests/pa3/check.py` and `--sanitize`: 67 invocations and 15,015
  expression results each, plus API promotion/identity/ownership/allocation checks.
- Full PA3 suite under ASan/UBSan: 20/20 with `CPPGM_TEXT_TEST_TIMEOUT_SEC=60`,
  `CPPGM_BATCH_TESTS=0`, and the isolated `obj/student-pa3/ppexpr-sanitized` binary.
  The first auxiliary run hit the default 10-second triple-fixture timeout;
  rerunning with sanitizer overhead allowed resolved it. Required optimized
  course checks pass with their original timeout and coverage.
- File audit: 39 implementation files, 40 with the optional stage entry-point
  audit. `git diff --check` passes. Existing PA1/PA2 sources, course fixtures,
  references and harnesses are unchanged from the stage base.

Final plan/evidence commit and clean-worktree checks are recorded in `plan.md`.
There is no incomplete language group or related PA3 work deferred at handoff.
