# PA27 final architecture audit150

Disposition: **full-stage audit complete; PA27 passes**.
Stage base: `f833cf1ff361529147361cada33eca62e55330cf`.
Audit entry: `9f5e095d450519d8660a29d4629ec5b8f5bcddde`.
Final code: `2fba819daa6e8a79a38dc0d35a3f8f942ff01bb7`.
Compiler SHA-256: `74d142d3f7cac9931ece646a63ad3d71689fe1174fd0d5fb596e1e45d151b06d`.

I read `spec.md`, the PA27 README, testing/reference rules, assignment map, stage
commits, current source and inherited plan. Review covers the accumulated **23
commits and 81 implementation paths**, plus inherited pipeline owners needed to
establish their behavior. [Reviewed range](../student.tests/pa27/evidence150/reviewed-range.json)
enumerates every commit and source hash. The unchanged [audit148 archive](audit148.md)
preserves the checkpoint; its conclusions did not substitute for this review.
Handoff149, including the previously failing hosted fixture, is now independently
reviewed. Entry had no live build/test process; this audit made progress through
independent reconstruction, a reducer, a repair and new measurements.

## Findings, repairs and handoff review

**Header probes escaped their conditional context.** Entry accepted
`int value = __has_include("absent-audit150.h");`. The
[reducer](../student.tests/pa27/evidence150/header-context-entry.json) records entry
success and host rejection. GNU documents both header-probe operators for
`#if`/`#elif` expressions; this documented extension contract, rather than compiler
agreement, establishes the defect. See the official
[header-probe contract](https://gcc.gnu.org/onlinedocs/gcc-14.3.0/cpp/_005f_005fhas_005finclude.html).

`Preprocessor::builtin` now checks the existing `MacroExpander::expression_`
context for both operators. Argument prescans and replacement rescans already
inherit that context, so the check belongs at the shared expansion owner. It
adds no cache, traversal, syntax rule or phase transport. Fourteen
[controls](../student.tests/pa27/evidence150/header-context-controls.json) cover
`defined`, nested `#if`/`#elif`, ordinary source, macro arguments/replacements,
both operators and another directive. An initial control expected GCC to reject
`#line __has_include(...)`; GCC accepts that extension beyond its documented
contract. The [initial trial](../student.tests/pa27/evidence150/header-context-initial-trial.json)
is retained; the final control explicitly records the host difference. No course
oracle changed.

**Handoff149's complete ownership paths were reviewed.** Named variadics preserve
omitted versus explicitly empty tails using existing argument slices. Header
probes and includes share search order, while probes test existence without
processing a header. GNU null/enum syntax feeds ordinary typed nodes. String
comparison builtins become ordinary C calls. Atomic signatures are cached by
builtin kind and unqualified pointee identity; normal conversions and single
argument evaluation feed one typed atomic plus at most one subtraction. Pointer
increments use bytes. Sequential consistency for all requested orders is
permitted by the [GNU atomic contract](https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html);
width, side-effect and contention controls cover this policy. Named variadic
details follow [GNU's macro contract](https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html).

Dependent alias bases keep typed identities. Friend access follows the primary
template and its specializations; explicit instantiation applies cached partial
ordering with winner/dominance passes over matching candidates. Sole unnamed
`void` normalizes before parameter reconciliation. These match N3485
[temp.friend]/3, [temp.deduct.decl]/2 and [dcl.fct]/4 in the
[local C++11 draft](../doc/n3485.txt). First-signature lookup remains frozen while
the selected source declaration owns its access recipes. `(frame, recipe)` checks
memoize complete results and prune source subtrees; lexical friend access need
not instantiate an unrelated class, while receiver/body demands still require
the concrete context. Missing diagnostic names retain valid diagnostics.
All 66 hosted controls and 33 inherited signature cases pass on the final tip.

Earlier object, ABI and storage repairs were rechecked together: GOT scratch
lifetimes, relocation addends, COMDAT placement, template suppression, TLS/import
roles, anonymous receiver access, selected constructor storage and constant
reference dependencies. Their positive/negative controls pass. No additional
defect was found in these combined paths. No workaround based on a source
filename, expected answer or library type spelling was found; prescribed Itanium
standard substitutions are ABI rules.

## Independent architecture reconstruction / Spec Alignment

| Spec surface | Actual owner, representation, validity and release |
|---|---|
| Source/parser (§1) | `lowering/driver.cpp` owns preprocessing, `PostTokenCursor`, the interned syntax cursor, parser and cooperating analyzer per TU. Immutable buffers hold bytes/file IDs; bounded lexical lookahead and scoped parser checkpoints consume the stream. Retained tokens/regions serve deferred language behavior, not successive owning token graphs. |
| Source graph/templates (§§1–4) | `syntax/occurrence.cpp` publishes source regions once and indexes `(context, source)` occurrences. Parsed bodies/defaults/nested classes are retained; instantiation projects dependent edges rather than calling the parser again. `ExpressionStore` inherits fixed facts, with slab/flat storage for facts and edges. |
| Identity/lookup (§§2–3) | `semantic/model.cpp` canonicalizes complete type shapes in compact flat indexes; scope/name/kind indexes and explicit using/base/ADL edges limit lookup. Selected candidates and conversions persist. Rendered manglings are outputs, not equality keys. Expected candidate rejection returns status; final errors may construct diagnostics. |
| Substitution/demand (§§4–5) | `template_type_facts.cpp` interns parent-linked frames including specialization/parameters/arguments and parent identity. `(frame, source entity)` publications distinguish source-access and instantiated contexts. `template_call.cpp` keys specialization by canonical pattern plus argument pack; pattern identity carries its owner. Definition, body, layout, defaults, storage, RTTI/vtable and emission states are separate, with in-progress recursion handling. |
| Scheduling/invalidation (§5) | `semantic/declaration.cpp::finish` advances distinct monotonic cursors for deferred uses, specialization, member/friend, storage, vtable and exception demands. `query_dependencies.cpp` records consuming facts for incomplete-class queries and resets only those facts on relevant completion. No global generation flush or retry of unrelated pending bodies was found. |
| Storage/constant facts (§§2,4,6) | Constructor actions own selected variant paths and cumulative offsets, separating destination from expression receiver. Default contexts are immutable. Completed `(field, normalized receiver class)` projections are TU-owned facts shared by runtime, binding and constexpr consumers; missing layout is an invariant failure. Constant activations own flat group/address/value buffers; cache keys include referenced storage versions/liveness, and aggregate results freeze once. |
| ABI/lowering (§6) | `lowering/symbols`, `local_abi`, `query_abi` and `standard_abi` build typed ABI graphs from recorded entities, template parameters, linkage and source ordinals. TLS wrapper/init roles, aliases, suppression and internal support objects have TU-owned identities. Procedural lowering directly builds typed `Program`; it does not reconstruct C++ decisions from symbol text. |
| MIR/object (§§6–8) | `toolchain/driver.cpp` consumes the typed program. Native selection/allocation/encoding retain one function's MIR and release it after encoding. `HostElf` moves byte lanes into sections, sorts placement once, assigns final symbols, repairs owner-relative fixups/groups/FDEs and streams ELF. Relocation sections exist only for actual fixups. No assembler or host compiler implements required output. |
| Lifetimes/telemetry (§§8–9) | Source/parser/semantic storage dies after each TU is lowered; only required typed LowIR/linkage pools survive into native work. Function temporaries then die; object buffers survive until writing. Hot identities use slabs/flat indexes rather than per-node shared ownership or ordered trees for incidental determinism. Mutable caches have TU/function/activation owners, never process-global lifetime. Identical objects with stats on/off verify telemetry neutrality. |

These conclusions follow actual call graphs and structures, not only the diff.
Host-link and inspection commands belong to the harness or personal audit adapter.
Explicit tools may render/parse LowIR; production object creation does neither.
No unexplained text roundtrip, global retry, repeated demanded body, semantic
reconstruction or new per-node hot allocation remains in reviewed paths.

## Source-to-ELF and optimization trace

The new [trace source](../student.tests/pa27/audit150-trace.h) demands
`measure150<11>` and `Packet150<11>`, containing a prefix, anonymous union and
nontrivial `Resource150`. Two compiler-built TUs share the instantiation; a
host-built third TU provides imported data, TLS and runtime checks. The first TU
also emits an aligned named section, an imported-address initializer with addend
8, an addressed local function and an unused local function.

[Trace evidence](../student.tests/pa27/evidence150/trace.json) records 19 commands,
source/view/object hashes and complete counters. Both object link orders and the
host control execute successfully. They check address coalescence, results 36/40,
destruction counts, throwing construction, TLS mutation, alignment 32 and
relocation identity. The unused local and an invalid but undemanded dependent
member body are absent. An initial probe used GNU `aligned`, outside PA27's
required attribute subset; the final probe uses C++11 `alignas(32)` to test the
required alignment fact.

The source consumes **299 tokens**, maximum pending **81**, and creates **475
parsed nodes**. It records four specialization entities, **one template body
transition**, one class completion, seven inherited expression facts and zero
expression variants. All six member demands are processed exactly once. Four
constructor actions, one constructor storage path and five field projections
carry offsets **0, 8, 16** into direct typed LowIR. Class completion does not
substitute/check the unused member body.

The audit adapter validates LowIR, renders the actual host MIR and compares its
code/data/TLS image with ordinary object compilation. Its object is byte-identical
to production `-c`. Duplicate encoding is confined to this explicit adapter.
The trace has **11 native functions, 109 instructions, 663 code bytes and 176
aggregate frame bytes**. `measure150` has a 64-byte frame, including a 24-byte
packet and saved return value across destruction. MIR exposes O0 stores/reloads
around that return; they are not claimed as optimized away. `read` preserves its
accumulated result in RBX across the TLS wrapper call. These are actual
call/lifetime/spill costs.

The useful fact followed through selection is the proven field/import
displacement. Typed `index` offsets become `[receiver+8]`/`[receiver+16]`, subject
to carrier lifetime; unknown cases materialize an address. A single-use, adjacent
nonvolatile integer RHS load can fold into an allowed ALU/compare operand without
moving effects or reassociating FP arithmetic. Imported data is materialized
through GOT; the trace folds its addend into `add ...,[reg+8]`. Allocation extends
carrier intervals, rejects unsafe clobber crossings and preserves values across
calls. Selected location tags remain where supplied. Relocation/unwind inspection
confirms GOT imports, weak TLS-init checks, TPOFF, COMDAT groups, lazy relocation
members, aliases and section-relative FDEs. Normal and throwing executions agree
with the recorded cleanup semantics.

## Work/growth budgets and performance acceptance

| Work / inherited local choice | Legality, profitability, invalidation and bound |
|---|---|
| Semantic facts | Complete canonical keys and precise completion edges; one completed fact per key, dependent-only work and demand-specific queues. Projection facts require completed layout and never demand a body. TU/activation ownership bounds memory. Constant limits remain 1,000,000 steps and depth 512. |
| Address/adjacent-load folding | Proven displacement, allowed opcode/width, single nonvolatile adjacent use and carrier lifetime. Removes a materialization/load; no code growth or fixed-point search. Carrier intervals update locally; failure keeps the ordinary operation. |
| Private temporary carrying | `native/carry.cpp` uses two interval scans and a maximum 64-instruction window with three reserved carriers. Only private, single-write 64-bit values within block/effect rules qualify. Calls, unsupported symbols/immediates and hidden-clobber instructions stop the proof. No speculative growth; unproved cases retain memory. |
| Bulk selection | `bulk_encoding.cpp` uses reserved vector scratch for 16-byte zeroing; otherwise it trials scalar zeroing up to 32 bytes against REP's actual encoded size. Copy selection uses fixed size/alignment thresholds (32 bytes, or 64 with alignment 8), otherwise constant-size REP. No unbounded unrolling or cached analysis to invalidate. |
| Allocation/encoding | Compact per-function intervals, near-linear ordinary allocation, bounded parameter-flow facts and linear selection/encoding. Frame size remains below `0x70000000`; native global alignment is capped at 4096. No new higher-level optimization policy. |
| Object demand/ELF | O(symbols + visited operands) root reachability, each reached body scanned once. Required base/runtime/externally visible/addressed entries remain roots. Placement O(S log S); byte movement, fixup repair and writing linear in output. One group/body section per mergeable definition; relocation members only for actual fixups; section count below `SHN_LORESERVE`. |

Together these bound work by source/expansion output, required candidates/demanded
facts/dependency edges and produced IR/object bytes, plus placement sorting and
language-required overload/partial-order comparisons. There is no new global
fixed-point transform or speculative body duplication. Necessary emitted bodies
and ABI metadata count in RSS/output evidence; fewer IR nodes do not prove profit.

[Performance150](../student.tests/pa27/performance150.md) reports all four
dimensions for common memory/FP/EH/pruning, construction/constant and signature
workloads, plus newly supported hosted streams. The final frozen compiler has
**416 observations**; **416 pre-repair** and **2904 historical** observations are
preserved. Every supported pair uses A/A calibration and six ABBA blocks. Output
checks and instruction comparisons accompany timing. No precise speedup is
claimed: spreads are broad, and identical pre-repair images also ran at
substantially different wall times. Required local pruning removes 30,816 text
bytes; EH requires one LEA-to-GOT-load change with unchanged instruction count.
No optional transform was added or retained on an unsupported profit claim.

Spec §9's stage-scoped acceptance governs inherited plans too. Blanket 15%
latency/RSS and zero-growth targets are diagnostics, as historical evidence
already established; they cannot reject necessary correct COMDAT/GOT/access
facts. Avoidable empty relocation metadata and repeated projection work were
removed earlier and remain removed. All measurements, semantic requirements,
mandated limits, timeouts and coverage stay intact. O0 is the available native
policy; accepted O2 invocations are compatibility controls. PA32/33 optimizer/debug
work and PA34 self-hosting do not become PA27 exit gates. The checked-in hosted
stream fixture remains required and passes despite the README's broader exclusion.

## Reference proof, validation and closing ledger

Only the four [overlay145 inspection outputs](reference-corrections.md) differ
from the stage-base contract: `_Z1g` becomes `g`. I rechecked the two-source
reducer, N3485 [basic.link], the raw host-name requirement and Itanium 5.1.2's
global-variable rule in the local ABI text. Bundle revision
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, SHA-256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`, remains pinned.
Proof is independent of compiler agreement. No fixture, comparison rule,
runtime oracle or relocation expectation was weakened; no new correction exists.

[Final validation](../student.tests/pa27/evidence150/validation.json) passes
`make test-report-through-pa27` (**4441/4441, 27 stages**), `make test-pa27`
(**158/158**, sections **3/3**) and the exact required file audit. Its four
warnings are inherited substantial headers: `lowering/procedural.h`,
`lowir/model.h`, `semantic/analyzer.h`, `semantic/model.h`. All **423 personal
command checks plus 33 signature cases** pass explicitly. The primary entry log
also says 4441, correcting the request's 4604 summary without changing coverage.
PA1–PA26 account for 4283 of these tests.

All **19,693 tracked test/reference paths** and 158 PA27 anchors are preserved;
infrastructure is separately diff-checked unchanged. Audit148's 19,749 inventory
also included 56 root scripts: a scope difference, not deleted coverage. The
final inventory includes Git blob IDs, explaining its different hash format.
Entry-to-tip protected-path changes are empty. Full logs, binaries, objects and
views remain in `$RALPH_ARTIFACT_DIR/pa27-150/`; no generated objects/logs are
committed.

| Boundary | Cumulative review and result |
|---|---|
| 145 `554f05f0..5aefb962` | ELF/GOT/sections/demand and lazy relocation repair re-reviewed. |
| 146 `364ebbcc..8d595af6` | Canonical ABI, suppression, TLS and internal support ownership re-reviewed. |
| 147 `7c71b6e6..7dbb0678` | Storage/default/lifetime/reference validity and flat activation ownership re-reviewed. |
| 148 `cbe7871e`, record `226b8866` | Anonymous access, field projection and constant receiver repairs re-reviewed; incomplete checkpoint retained verbatim. |
| 149 `d1b017e5..9f5e095d` | Previously unaudited hosted prerequisites, signature access and diagnostics reviewed end to end. |
| 150 `2fba819d` plus final evidence record | Conditional probe context fixed; architecture trace, measurements and required validation complete. |

No unaudited handoff, known PA27 blocker or unresolved architecture finding
remains. The final evidence commit changes only personal audit tooling, records
and documentation after the validated code tip.
