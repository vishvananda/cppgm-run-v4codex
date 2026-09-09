# PA4 independent final architecture audit

Reviewed stage base `468a93526` through final implementation `e0b7bf8ab` against
`spec.md`, `pa4/README.md`, `macros.md`, `directives.md`, the root testing guide,
source sets, implementation sources and every PA4 stage commit. Entry was clean
at `695e607c3`; its passing checkpoint was evidence to recheck, not the basis of
this architecture reconstruction. The previous goal turn made progress through
committed implementation and validation. This audit does not advance to PA5.

## Spec Alignment

| Spec surface | Actual PA4 implementation and audit conclusion |
| --- | --- |
| §1 source/cursor | Immutable `SourceBuffer` bytes; bounded `CharacterCursor` lookahead; `PPTokenCursor` → `Preprocessor` → `PostTokenCursor`. These are shared structured pull interfaces. Only macro definitions, invocation arguments, rescan work and the current directive are deferred. The CLI alone renders PA2 records. |
| §2 identity | TU-owned flat `IdentifierTable` provides compact stable IDs; dense macro definitions and prebound parameter indices avoid spelling-key lookup. Tokens carry physical file/offset and presumed filename/line identities. Interned byte views must be reacquired after interner growth; retained identities are IDs. |
| §3–5 applicable state | Macro lookup touches one indexed definition; conditional evaluation queries live macro state. Prescan readiness belongs to one invocation/parameter, not a cross-invocation cache. Redefinition and `#undef` cannot reuse an old expansion. Conditional/file stacks and the device/inode once table have explicit TU ownership. Semantic lookup, overloads and template-demand caches are later stages. |
| §6–7 applicable transforms | PA4 has no LowIR, MIR, optimizer levels, register allocation or encoder. Its optimization is reuse of compiler scratch storage, with no change to tokens, required work or source locations. Legality, invalidation, work and growth are detailed below. |
| §8 allocation | Sources, interned names and definition spellings release at TU end. Deferred argument tokens are borrowed indexed slices. Task frames use stable 32-frame slabs, argument/output/index buffers grow geometrically and are reused by depth. Active state is cleared on completion; capacities release with the expander. No per-token allocation, shared ownership or recursive task destruction. |
| §9 measurement | Fixed frontend inputs, frozen functional A/B, A/A and B/B calibration, two ABBA blocks, output equivalence, every time/RSS observation and separate telemetry. Host text is recorded separately from generated text. New allocator interception checks actual warmed allocations, in addition to work counters. |
| §10 self-containment | The shared implementation reads files, calls `stat` for identity, performs required lexing/expansion/conversion, and writes output itself. No host/reference preprocessor or compiler implements any required output. The inherited test runner's process machinery is a harness boundary, not a compiler phase. |

## Representative data and ownership traces

**Declaration and template spelling.** The direct API test defines `CAT(a,b)`
and `DECL(Name)`, then selects `#if defined(DECL) && (1 || 1/0)`, applies
`#line 120 "logical.cc"`, and consumes `DECL(Sample)` followed by
`SampleBox<int> value;`. Raw tokens enter the TU interner. `defined` suppresses
operand replacement; the PA3 typed evaluator validates the expression and
selects away the arithmetic error. The file frame applies presumed location
state without changing source bytes.

`DECL` captures `Sample` once; its prescan result is substituted into the
replacement. Rescanning finds `CAT`, whose raw boundary spellings form
`SampleBox`. `generated` invokes the shared phase-3 lexer on this required
paste and validates exactly one token. It does not replay phases 1/2 or transport
a textual phase output. The pasted identifier and the later source occurrence
have the same canonical ID. Replacement locations follow the invocation;
`__LINE__` becomes a numeric token at 120 and PA2 decodes its target integer
bytes directly. The API checks all 29 tokens, the declaration/use line split,
physical file identity, presumed filename, repeated name identity and integer
value without consulting a reference dump.

PA4 neither demands that template nor lowers the declaration. The reviewed
handoff is the structured post-token cursor; a semantic graph, specialization
facts, LowIR, MIR and ELF are explicitly absent until their owning assignments.
Likewise, semantic template-heavy compilation and executable/self-hosting
benchmarks cannot be claimed from these token workloads.

**Deferred expansion and recursion.** For `I(I(...42...))`, the outer invocation
captures/indexes parentheses once. Nested prescans borrow ranges and jump to
indexed delimiters. The explicit task depth descends and resumes iteratively;
no host call-stack frame is added for each nested invocation. Ordinary argument
uses share one expansion; `#`, `##` and unused arguments retain the raw spelling
and do not trigger unnecessary prescans. Course ancestry is token-local:
persistent radix sets plus permanent unavailable paint preserve recursive
suppression, parameter-substitution nesting rules and helper/source boundaries.
The course's `f(f(x))`, `z`, and `g(f)(g)(3)` groups and the personal variants
exercise these distinct paths. No global expansion cutoff substitutes for the
language rule.

**Directives, includes and token lifetime.** `raw` stops at a directive boundary
before executing it. `advance` waits for prior expansion to drain, then mutates
definitions/conditional state or pushes an included file. Include lookup uses
the presumed filename-relative path, then the supplied path; device/inode
identity unifies hard-link aliases for once state. `_Pragma` is recognized only
in expanded text sequences. Included files share TU definitions, but each file
owns its conditional stack. Each primary constructs a fresh `Preprocessor`,
resetting definitions, once state, counters and location state.

Unchanged spellings borrow immutable source bytes. Translated definition
spellings live in persistent slabs; generated/translated expansion spellings
live in transient slabs, rewound only when pending expansion drains. The
post-token cursor decodes a string before pulling another PP token and holds
only one lookahead. Its literal-operator suffix split consumes that lookahead
before advancing the underlying cursor; direct and macro-generated suffixes
retain the corrected presumed sublocation. Its optional joined spelling is an
observation view, not parser transport. Borrowed output views expire on the
next cursor call; names retain TU-stable IDs.

## Findings, changes and transform audit

1. **Hot task allocation:** the checkpoint's `deque<Task>` constructed and
   destroyed a large task and its child buffers for every argument prescan.
   Indexed slices fixed copying complexity but did not eliminate this churn.
   `6c1867c8b` replaces this with an inline root and stable slabs of 32 tasks,
   reuses formal-argument slots, and swaps reusable child output buffers.
2. **Remaining capture allocation:** an allocator-counting API assertion still
   failed after pooling. `ArgumentStorage::index` rebuilt its temporary delimiter
   stack for every capture. `e0b7bf8ab` retains this stack with its argument
   storage. The same assertion now passes with zero allocation calls during the
   third 20,000-deep invocation, after two warm-ups.
3. **Evidence and handoff:** the old plan left its review marker at PA3 and did
   not constitute independent review. This audit closes every PA4 handoff below,
   adds a complete declaration/token trace, strengthens reuse-state checks, and
   records both pooling campaigns without discarding intermediate evidence.

**Legality and invalidation.** Slab growth moves only slab-owning pointers;
`ArgumentStorage` addresses remain stable while nested slices borrow them.
Argument arrays are indexed, not pointed into across growth. A completed child
swaps its output into its parent's selected argument and clears its own active
input/output. Substitution clears argument slices, readiness and expanded
contents, and clears captured tokens/indices. Capacities retain storage, not
cached semantic results. Each later invocation overwrites the head, macro ID,
context, prescan index and raw slices; no stale `__COUNTER__`, redefinition,
empty/variadic argument or function-name lookahead result survives. An exception
unwinds the owner normally; destruction is iterative. The new tests vary arity,
empty arguments, raw/expanded uses, repeated counters and deferred function names
within the same expander to exercise those invalidations.

**Work and growth budget.** Lexing tracks source bytes; indexing visits each
newly captured token once; nested ranges reuse that index. Each ordinarily used
argument is expanded once per invocation. Replacement work tracks consumed
replacement elements and produced expansion tokens. Identifier/parameter/once
lookup is indexed; ancestry paint has 32-bit fixed depth and intersections visit
only differing shared branches, never unrelated definitions. Source-mandated
expansion is not discarded to meet an optimization budget.

The pool adds O(1) indexing per depth transition and at most
`32 * ceil(D/32)` child frames for maximum prescan depth D, plus geometric
capacity for each depth's maximum argument/rescan/index work. Capacities stay
with their expander and all release at its destruction; they never become a
process-global cache. The root allocates no task slab for ordinary text or
parameterless helpers. For the fixed 600-deep, 128-repeat workload, 76,800
prescans use 19 slabs, 600 argument-buffer growths and at most 1,200 prescan-output
growths. Three 20,000-deep API invocations use 625 slabs and capture exactly
180,000 tokens, with no further warmed allocation. Counter spellings remain
within 128 KiB. No IR/executable code growth, ABI action, observable macro side
effect or debug-location transformation is introduced; no speculative proof,
fixed-point pass or optional-optimization fallback is needed.

**Profitability.** Acceptance was recorded before measurement in the plan:
paired latency regression <=10% plus calibrated noise, RSS <=15% plus 1 MiB,
host text growth <=15%, fourfold source growth <6x time and <5x RSS. Require the
nested benefit to exceed noise in both paired blocks. The final campaign and
validation results are recorded in the plan and linked performance report.

## Performance and validation

[Final frozen evidence](../student.tests/pa4/final-audit-performance.md) compares
`695e607c3` with `e0b7bf8ab`: 120 workload observations plus eight startup probes,
identical input/output hashes within each A/B group, and verified final binary
hash. Nested latency falls 0.108902 → 0.094080 s, improving 12.35–13.90% in paired
blocks against 3.94% noise; median RSS rises 236 KiB. Long chains regress
1.42–1.84%, with RSS +74 KiB; inconclusive groups are not claimed as wins.
Host text falls 1.66%; 4x input takes 3.9663x time and 3.0752x RSS. Every declared
work/growth budget passes. The preceding pooled-frame campaign is preserved in
[performance-pooled.md](../student.tests/pa4/performance-pooled.md); historical
stage evidence remains in [performance.md](../student.tests/pa4/performance.md).
Generated-program runtime/text size are N/A at PA4; fewer allocation calls or
tokens are not offered as evidence of generated-program speed.

Fresh ordinary validation: `make test-pa4` 105/105;
`make test-report-through-pa4` 205/205, all four stages passing;
`perl scripts/cppgm_file_audit.pl --stage pa4 --paths dev/src` 43 files passing.
PA4 personal 168 cases and direct API checks pass. The final standalone
ASan/UBSan builds with leak detection pass those cases/API checks and all 105
course inputs. Prior personal suites pass: PA1 64 cases/API, PA2 316 cases/API
and 7,062 independent integer values, PA3 72 invocations/15,045 results/API.
Source-set registration, whitespace and unchanged-fixture/reference/harness/
timeout checks pass. All eight benchmark input hashes agree across both audit
campaigns. No additional debug/inspection gate is prescribed for PA4.

The intended refactors and audit/evidence consolidation are committed. No PA4
correctness, self-containment, timeout, file-audit, architecture or performance
blocker remains; final status is clean.

## Handoff ledger closed by this review

| Commit | Previously unaudited handoff now checked |
| --- | --- |
| `7496be903` | Baseline/design and relevant shared phase boundaries. |
| `77bd7bc51` | Full macro/directive implementation, source ownership, live PA3 integration, location propagation and output adapter. |
| `95dc4d4b6` | Indexed argument slices, prescan task stack, transient spelling reuse and existing-work telemetry; task allocation defect fixed above. |
| `965f7ba6e` | Parameterless close consumption preserves context intersection and requires no deferred argument/index work. |
| `5c200a4df` | Presumed literal-operator suffix locations survive raw, generated and replacement ownership paths. |
| `695e607c3` | Completion assertions independently reconstructed; historical binaries/manifests/observations checked, not treated as architecture proof. |
| `6c1867c8b` | Stable frame/buffer reuse, state invalidation and whole-stage frozen performance comparison. |
| `e0b7bf8ab` | Retained delimiter scratch, real warmed allocation assertion and complete declaration/template spelling trace. |

There is no deferred PA4 behavior group or unaudited PA4 handoff. Parsing,
semantic template demand, hosted-header compatibility and native/executable
optimization remain explicit future assignment surfaces, not PA4 substitutes.
