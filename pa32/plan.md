# PA32 implementation plan

Stage base commit: e82bf4152fe8d6d68b9cd966655db0d8142cf81b
Last reviewed commit: e82bf4152fe8d6d68b9cd966655db0d8142cf81b

## Active implementation (209)

Entry HEAD `d2e412d65f3fca9c8e225c575721c6b017651204`; previous turn
classified as progress (validated dataflow implementation). No live build at
entry. Baseline remains 127/219, 92 failures; course coverage is unchanged.
Preserve the stage/review markers above.

Work next: typed direct-call summaries, bounded expansion using the existing
attribute expander, then reachable-support pruning and exceptional-region
cleanup. Owner: LowIR optimizer; facts flow from symbol/signature/body IDs
through legality and profitability admission into the shared typed clone path.
Summaries/roots use indexed edges, once per immutable input; cloning consumes
per-caller and unit-linear budgets and retains valid calls on exhaustion.
Validate scalar/branching/object boundaries, mutation, recursion, EH/debug,
name collisions and direct/replayed objects. Freeze same-level binaries and
ABBA runtime/compiler/RSS/text observations before making performance claims.
Aggregate memory and finite-loop proofs remain separate unfinished owners;
whole-stage independent review is still owed.

## Design/spec alignment and completed ownership

Source -> typed LowIR Program -> shared optimizer -> writer or native/ELF.
External LowIR uses the same optimizer after validation. No textual production
roundtrip, private semantic side channel, reference delegation or fixture keys.
O0 returns before any optimizer analysis. O1/O2/O3 share the current bounded
pipeline; distinct higher-level quality objectives remain unfinished.

| Owner | Data flow and legality | Work/growth and validation |
| --- | --- | --- |
| Entry/transport (207) | CLI, levels, source/LowIR dispatch; ABI/sections/object extents and debug locations survive serialization | Deterministic native/ELF order; both 25-case object replay lanes pass |
| Scalar/local memory (207) | Typed constants/identities, stable-value CSE, unescaped slots; mutable reads retain snapshots, traps/volatile/effects stay observable | Dirty def/use worklists; 16*(I+uses+1) each for propagation/rewrite; 506 explicit executable controls at four levels |
| Sparse slot promotion (208) | Load-demanded (block ID, slot ID) entries; typed store snapshots become copies, joins become phis. Unknown initialization, escape, volatile or incompatible access declines. Exceptional entry accepts only one entry store before every registration; handler-defined values can join on ordinary edges | Each demand published once; no recursion; 16*(I+O+B) admission work. Added phis <= I and phi operands <= 2O, otherwise transaction discarded. Loop/merge/EH/width and stress controls |
| Ordinary CFG/dataflow (208) | Dirty dominators scope expression/equality/zero-nonzero facts. Mutable values cannot become facts. Phi inputs use predecessor environments. RPO requires a def/use dominance proof. Boolean and integer-equality/extreme choices preserve noncanonical truth and widths | Dominance <=32*(I+O+E+B); CSE probes <=16*(I+O+B). Shared completed dominance feeds edge facts; EH/exhaustion uses conservative local CSE. No instruction growth |
| CFG cleanup (208) | Constant edges, reachability/phi repair, adjacent merging and empty-jump bypass. Conflicting phi choices or uncertain exceptional flow remain intact | Memoized chain traversal; <=16*(I+O+B) phi expansion work and <=2x incoming phi operands; transactional failure. No added blocks; identity/edge/diamond reducers |
| Admission (208) | One iterative SCC census identifies calls inside ordinary cycles. Those functions keep local optimization until a measured call/loop cost policy exists; a call outside a loop does not block promotion | O(I+O+B), cached for this invocation. Transforms add no calls/cycles, so this conservative decision remains valid. Explicit before-loop/in-loop controls |

I/O/B/E are instruction/operand/block/edge counts at the owning pass. This is
function-local state, released after each function; unit compaction pools and
ID arrays die at the pass boundary. Generated names use a monotonic collision
cursor, never semantic identity. No process-global cache or per-node ownership.

The schedule has at most five scalar worklists (one only after actual promotion),
two CSE sweeps, two local-slot sweeps, one promotion/diamond/bypass invocation and
three structural sweeps. Additional scalar/CSE sweeps visit only admitted
functions with an original multi-block CFG; unchanged straight-line functions
keep the original two scalar worklists. The final structural sweep consumes edges exposed by
phi repair; it does not restart the pipeline. There is no global fixed point.
Promotion bounds output by 2I instructions/3O operands; bypass adds no
instructions and at most doubles phi operands. Subsequent passes only shrink.
Together these give input-linear whole-pipeline work and memory, including
conservative exhaustion. A loose accounting envelope is 32768*(I+O+B+V+S+1)
charged work units and transient bytes over input pools (V/S: values/slots).
Actual charged work, phase wall/RSS and IR counts are retained in measurements;
this envelope is a bound, not a claim about typical memory consumption.

## Remaining implementation and concrete handoff boundary

1. Memory/aggregate ownership: typed projections, alias/effect/lifetime facts,
   scalar replacement, load/copy forwarding and exceptional-region state. The
   current scalar-slot proof cannot justify arbitrary memory elimination.
2. Interprocedural ownership: call/effect/no-unwind summaries, bounded inlining,
   support pruning, cleanup and lifecycle convergence. These own most remaining
   direct/source call predicates; adding local scalar passes cannot satisfy them.
3. Loop ownership: finite-loop proofs, motion/fill/deletion, O3 specialization
   and unrolling, with measured growth/profit policies. Calls inside cycles
   deliberately retain the measured local policy; no call-summary or allocation
   benefit is presumed. PA33 allocation quality is not an additional PA32 gate.
4. Source/debug ownership: binding copies, start spans and emitted declaration
   identities. Source-debug lanes remain 0/3; O3 direct debug still needs unroll.

The completed group extended from slot merges through ordinary dominance,
branch facts, phi-safe bypass and scalar diamonds, plus the performance policy
needed to retain it. Remaining predicates require memory, call, exceptional,
loop-trip or source-binding facts absent from these owners. Further related work
would require those new analyses and separate runtime workloads, rather than
more iterations of the completed passes. These are unfinished implementations,
not review questions, waivers, or a claim that PA32 is complete.

## Performance evidence and stage-scoped acceptance

Frozen binaries, inputs, flags, A/A calibration and six ABBA blocks, checked
outputs, wall/RSS/text, raw observations and source/log hashes are in
[evidence208](../student.tests/pa32/evidence208/binding.json). Durable artifacts:
`/home/vishvananda/work/private/v4codex/artifacts/pa32-208`.
Scripts: `dataflow_performance.py`, `runtime_pairs.py`, `common_levels.py`,
`performance.py`, `selfhost_performance.py`; all run explicitly.

Final frozen baseline/current comparison at O1 (1,200 functions per affected workload):

| Workload | Compiler ratio (range) | Compile ms A/B | RSS KiB A/B | Runtime ratio (range) | Runtime ms A/B | Text bytes A/B |
| --- | --- | --- | --- | --- | --- | --- |
| slots | 1.495 (1.483–1.511) | 113.02/168.76 | 14512/15480 | 0.849 (0.845–0.855) | 94.45/80.34 | 149200/144400 |
| dominance | 1.271 (1.240–1.357) | 92.48/117.01 | 16772/17212 | 0.810 (0.806–0.814) | 116.28/94.21 | 220000/148000 |

Every affected runtime pair improves. Acceptance budgets: paired-median compiler
wall <=2x, maximum observed compiler RSS <=1.75x and no text growth on fixed
affected workloads. These are explicit policy budgets, not course IR envelopes.
All required IR predicates and semantic tests retain their own acceptance.

Fixed template-heavy common inputs at O1:

| Workload | Compile ratio | Runtime ratio | RSS KiB A/B | Text bytes A/B |
| --- | --- | --- | --- | --- |
| memory | 1.009 | 1.004 | 30364/30656 | 125199/125199 |
| floating | 0.992 | 1.010 | 30532/30648 | 125053/125053 |
| exceptions | 1.022 | 0.999 | 30220/30676 | 125339/125337 |
| pruning | 1.012 | 1.006 | 36064/36220 | 125199/125199 |

Memory/floating/pruning O1 objects are byte-identical; EH text shrinks by two
bytes. No common runtime benefit is claimed; spreads/noise are retained in JSON.
Common O0 objects are all byte-identical; compile medians range 0.981–1.002.
The inherited scalar O1/O0 check: compiler 1.896, RSS 10180/14576 KiB, runtime
0.868 (all pairs improve), text 170800/110200 bytes. The full spread
includes compiler pairs above 2x; the specified budget uses the paired median.
Compiler-owned `folding.cpp` at O0: 0.998 (0.975–1.009), wall medians
1.154/1.154s, RSS 75484/75840 KiB, identical 31889-byte text.
It has no executable entry, so standalone runtime is inapplicable; this is not
a full self-hosting claim. Removing extra scalar work for unchanged single-block
functions reduced common O1 compiler overhead from roughly 6–8% to 0–2%.

Before acceptance, the combined promotion/reuse policy regressed the fixed
memory workload: initial median +6.7%, repeated pinned runtime +8.1%. Diagnostic
variants and every observation are preserved. Disabling either promotion or
cross-block CSE alone did not resolve it; refusing cheap address CSE across calls
made it worse and was removed. The SCC admission policy restores the baseline
memory object byte for byte, preserving gains on affected call-free workloads.
This is a resolved optional-transform regression, not a later-stage exemption.

Earlier evidence is preserved in [evidence207](../student.tests/pa32/evidence207/binding.json):
scalar O1/O0 compiler 1.418, runtime 0.869, text 170800/110200 bytes, common and
compiler-component observations included. Its historical >=10% runtime target
was already reclassified as diagnostic with measurements; repeatable benefit and
explicit compiler/RSS/text budgets remain. No mandated limit or coverage changed.

## Handoff ledger and independent review

Entry HEAD: `399faac3130eee32004cfddbd7fc07df866693da`; prior goal turn was progress
(the committed local/transport group and its evidence), with no live job at entry.
207 commits: `8c01abc9`, `5f8be172`, `0e0d29f4` (110/219 from 0/219).
208 commits: `1aecbd03` sparse slots/ordinary CFG; `f36657ff` scalar choice closure,
SCC admission and independent growth budgets; `a1bbf8e7` selects only affected
functions for extra scalar work. The final evidence/ledger commit records this
handoff boundary.

Current required `make test-pa32`: **127/219**, **92 failures**, versus entry
**110/219**, **109 failures**: **17 original failures removed**, no new failures.
Ralph's supplied **110/425** census uses a different denominator and is retained
as context, not substituted for the root harness's unchanged 219 fixtures.
No course tests, references, harnesses or comparison rules changed.
`make test-report-through-pa31`: **5178/5178**. File audit exits 0 with four
inherited large-header warnings. Explicit `dataflow.py`: **134** cases x four
levels x two native paths, plus EH direct/replay, floating snapshots/signed zero,
uninitialized/volatile/escape/width/cycle guards, sparse-budget stress and
call-cycle admission controls.
Inherited `local.py`: **506** executable cases x four levels plus trap/CLI checks.
Debug required target: direct **4/5**, source **0/3**, separately completed debug
object replay **25/25**; nodebug replay **25/25** is in the primary suite. The
whole debug target still fails; its failures remain implementation work above.

Independent audit still owed: whole-stage source/template-to-ELF and useful-fact
traces; pipeline-wide bound/fallback accounting, EH/native liveness and metadata;
benchmark scope and profitability. Neither those questions nor remaining
implementation is waived. Stage/review markers remain at stage base. This is an
implementation handoff for Ralph's next implementation/audit, not advancement.

Explicit `python3 student.tests/pa32/verify_handoff.py` passes: 515 source
bindings, frozen binary/log hashes, 2044 raw observations and their recomputed
paired summaries, current budgets, object equality, 17 removed failures and
unchanged contract contents checked. This verifier does not certify the stage.
