# PA3 final independent audit

Scope: `pa3/README.md`, `spec.md` as applicable to a controlling-expression
frontend, and the requested full-stage gates. Independent entry HEAD:
`c0bb94df8`, clean, on 2026-09-07. Read the full stage history (`bc1a3f303`,
`e8cf198c7`, `4314a6eb3`, `c0bb94df8`), handout, spec, testing policy, plan,
and actual PA1 cursor/identity, PA2 scalar, PA3 evaluator/value/CLI sources.
The prior goal turn was progress: committed implementation and retained
validation existed. Reused green status was not accepted as final proof.
This audit found and fixed an unnecessary numeric-conversion ownership path.
No later milestone is claimed complete.

## Contract evidence

| Requirement | Authoritative implementation and evidence |
| --- | --- |
| Phases 1–3 and logical lines | Unchanged PA1 `PPTokenCursor`; `ppexpr.cpp` finishes on newline/EOF, skips empty lines and emits EOF once. Personal splices, trigraphs, block comments and CRLF cases pass. Lexical exceptions escape the evaluator and return failure even after a line syntax error. |
| Integral literals and course promotion | PA2 `decode_number`/`decode_character` feed `promote_pp_literal`; no phase-7 token serialization or string concatenation. PA3 requests `NumberDomain::integral`, so floating values and numeric UD suffix identities are never constructed. PA2 retains its full-domain default. Original ABI type selects signed/unsigned extension to 64 bits. Primary/Unicode fixtures and API checks of every integral enum pass. Strings, floats, UD and invalid literals are rejected unconditionally, including discarded branches. |
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
and RSS without adding analyses. [Final performance evidence](../student.tests/pa3/final-audit-performance.md)
freezes inputs/binaries/flags, validates output hashes and resource envelopes,
and reports every observation, A/A noise and ABBA paired spread.

Spec sections on declaration graphs, lookup/overloads, templates, semantic demand,
IR, optimization levels and ELF emission belong to later assignments. PA3 creates
no declarations, specializations, IR or executable. No optimization/runtime
benefit or generated-code-size claim is made; generated runtime/text size are
explicitly N/A. Reuse remains direct and typed for PA4's future macro expansion
stream and real `defined` callback.

## Independent finding, legality and profitability

For `0 ? 1.25e100L : 7`, PA1 still visits the entire spelling and the parser
still rejects the line unconditionally. Previously PA2 constructed a host
floating value before PA3 discarded it. For `1_suffix_18`, PA2 also interned
the suffix even though the entire number was invalid in this consumer. Those
identities remained in the TU table until process exit. The new allocation and
name-retention API check failed on the reviewed original implementation.

`f0b92212e` repairs the full owning path: `NumberDomain::integral` is an explicit
request to the shared numeric decoder, selected by `PPExpressionEvaluator`.
Integer grammar, target type selection and overflow checks remain shared.
Unsupported numeric domains return the existing compact invalid result before
floating extraction or suffix interning. PA2's default full domain preserves
its required float conversion and UD identity. Rejection never bypasses lexing,
phase errors or checks in an unselected arm; a following `7` still yields 7.
No numeric suffix from a rejected PP number can affect a language `defined`
query. IDs of actual identifiers remain stable; no semantic cache exists to
invalidate. No exception or diagnostic string implements ordinary rejection.

The work reduction is legal because both removed facts can only belong to an
invalid controlling-expression token, not a usable branch value. It adds one
constant domain decision per numeric token and no pass, search, IR or code
expansion. Work stays O(source bytes + tokens), with geometric O(depth) scratch
and distinct actual identifiers retained at TU scope. Conservative behavior is
the same invalid-line result; full decoding remains explicitly available to
consumers that need it. There are no optimizer levels, analysis invalidations,
ABI/debug transformations, spills or loop costs at this PA.

Before timing, fixed budgets allowed at most 1% host-tool text growth, 1 MiB
peak RSS growth, and 5% latency regression beyond A/A noise on unaffected work,
alongside the existing work/storage/scaling envelopes. The report evaluates
campaign means of paired changes and retains individual block spread. This
frontend change passes those limits; it does not authorize IR growth later.

Three sequential frozen A/B campaigns retain 324 measurements. Floating
rejection improves 37.3–40.4% in the six paired blocks. Suffix rejection improves
18.4–21.2%; campaign one's 31.11% A/A excursion makes its timing inconclusive
alone, and campaigns two and three independently confirm the gain beyond noise.
All original samples remain. The suffix workload drops 200,000 unused names,
12,935,068 → 64 bytes of table storage and about 20,854 → 8,028 KiB peak RSS.
Host-tool text is 86,656 → 86,968 bytes (+0.36%). The fixed seven original
workloads also pass; existing recovery improves, while valid-input variations
are disclosed without a blanket speedup claim. In campaign three one nesting
block is +7.64%, the other -0.85%; paired mean +3.39% stays inside the budget.
Ordinary 4/16 MiB final medians are 0.440050/1.743335 s, 8,028/20,320 KiB,
3.961673x scaling. Generated executable runtime and text size are N/A.

The report verifies both frozen binaries, all input/output and harness hashes,
B's build-source hashes against the current tree, and A's against `c0bb94df8`.
The original 168 telemetry observations were independently recomputed before
the fix and remain archived in `performance.md`; they do not describe final B.
Untimed hash-launch RSS can inherit the Python parent's high-water mark; only
the separately launched `/usr/bin/time` measurements establish peak RSS here.

## Final Spec Alignment

| Spec surface | Independent conclusion |
| --- | --- |
| §1 source/streaming/parsing | Immutable source; bounded translation lookahead; immediate borrowed-token consumption; single iterative grammatical pass, no checkpoints or retained syntax graph. |
| §2 identity/facts | TU-owned flat identifier IDs and inline typed scalar facts; canonical signedness/conversions/error selection recorded once, no rendered semantic keys. Declaration/type graphs are not yet a PA3 surface. |
| §§3–5 lookup/demand/caches | Live `defined` lookup consumes an ID and opaque owner context. No overloads/templates/dependency scheduling or result cache exists at PA3; state reset is local to the expression. |
| §§6–7 lowering/optimization | PA3 directly reduces typed constants. Conditional result type is retained even when an arm's arithmetic error is discarded. No IR/backend/ELF or generated-code optimization exists to audit yet; the numeric-demand optimization has the legality and resource proof above. |
| §8 lifetimes | Source and identifier storage own TU data; scalar conversions are inline; geometric operator/value scratch clears per line and frees with evaluator, no per-node allocation, recursive destruction or global mutable cache. |
| §9 complexity/evidence | Each source byte/token/operator has bounded work. Deep and flat fixed inputs check pending-stack bounds; A/A and ABBA compiler latency/RSS plus host-tool text and explicit generated runtime/text N/A are retained together. |
| §10 self-containment | Source-set registration links the actual PA1/PA2/PA3 owners. CLI writes results itself. The supplied batch runner calls that same main; external process logic belongs to the harness, not expression semantics. No host/reference compiler or fixture lookup implements output. |

## Validation and provenance

- `make test-pa3`: 20/20, improving the unchanged entry suite from 20 failures to 0.
- `make test-report-through-pa2`: 80/80; `make test-report-through-pa3`: 100/100.
- `python3 student.tests/pa3/check.py` and `--sanitize`: 72 invocations and 15,045
  expression results each, plus API promotion/identity/ownership/allocation checks.
- Full PA3 suite under ASan/UBSan: 20/20 with `CPPGM_TEXT_TEST_TIMEOUT_SEC=60`,
  `CPPGM_BATCH_TESTS=0`, and the isolated `obj/student-pa3/ppexpr-sanitized` binary.
  Fresh final run passes. The original stage's first auxiliary run hit the
  default 10-second triple-fixture timeout; required optimized course checks
  pass with their original timeout and coverage.
- `python3 student.tests/pa2/check.py --sanitize`: 316 cases including 7,062
  independent integers, protecting the shared decoder's full-domain behavior.
- File audit: 39 implementation files, 40 with the optional stage entry-point
  audit. `git diff --check` passes. PA1 sources and all course fixtures,
  references and harnesses are unchanged from the stage base. The shared PA2
  decoder's explicit domain is the only inherited implementation change.

`plan.md` consolidates the reviewed handoffs, including the previously unaudited
`c0bb94df8` evidence/renderer changes and `f0b92212e` ownership fix. Final
plan/evidence and clean-worktree checks close this audit. There is no incomplete
language group, unaudited PA3 handoff or related work deferred to the next PA.
