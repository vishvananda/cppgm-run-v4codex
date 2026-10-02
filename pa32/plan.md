# PA32 implementation plan

Stage base commit: e82bf4152fe8d6d68b9cd966655db0d8142cf81b
Last reviewed commit: e82bf4152fe8d6d68b9cd966655db0d8142cf81b

## Design/spec alignment and completed ownership

Source -> typed LowIR Program -> shared optimizer -> writer or native/ELF.
External LowIR is validated once and enters the same optimizer. No textual
production transport, semantic reconstruction, fixture keys or oracle delegation.
O0 returns before optimizer analysis. O1 adds bounded local/CFG and call work;
O2/O3 also propagate closed-world constant arguments. Higher-level loop and
memory objectives remain unfinished. Existing ABI, debug and object facts travel
in LowIR. Clone IDs are canonical; generated names are collision-safe views.

| Owner | Facts, legality and data flow | Complexity, work and growth |
| --- | --- | --- |
| Transport (207) | CLI, levels, sections/object extents and debug locations survive replay | Native/ELF order deterministic; both replay lanes required |
| Scalar/local memory (207) | Typed folding/CSE/unescaped slots; mutable snapshots, traps, volatile and effects preserved | Dirty def/use worklists, 16*(I+uses+1) per propagation/rewrite; no growth |
| Slots/ordinary CFG (208) | Sparse demanded slot entries, typed snapshots/phis; ordinary dominance, edge equalities, scalar choices and phi-safe bypass | Promotion 16*(I+O+B), phis <= I, new phi operands <= 2O. Dominance 32*(I+O+E+B), CSE 16*(I+O+B). Transactional fallback; no global fixed point |
| Calls (209) | Completed direct-call counts, address escape and reverse edges; acyclic-call proof; typed cloning reuses attribute expander. Parameters capture width/rounding and objects get independent homes; phi exits and result slots map by ID | O(I+O+B+F) census. Recursive chains, stackalloc/varargs, live callee EH/throw/resume and no_inline decline. Cloning depth <=64, work <=32768/caller and 32*(I+O+P+S+F+1)/unit. Charge instruction/operand/slot copies before mutation |
| Inline profit (209) | Small acyclic bodies, single-use discardable bodies, hints and noreturn cold blocks have explicit budgets. Shared loops require hints; expensive arithmetic only expands into small acyclic callers | Ordinary size 40, callful single-block 6, hinted callful 32 / call-free 128, single-use 512. Cold discount still caps actual body at 128. Added body instructions <=1536/caller (2048 single-use); <=1-instruction additions still consume unit/work budgets |
| Calls/EH (209) | Signature effects permit unused readonly/readnone calls only with no-unwind and normal return. Persistent registration stacks retire regions without unwind sources. Reverse call edges publish monotonic no-unwind facts; conflicting merges retain valid input | O(I+O+B+F) indexing; retirement work <=32*(I+O+B+1). Each block gets one incoming stack, each registration one node. Only affected callers requeue; exhaustion is conservative |
| Roots/constants (209) | Reachability from ordinary exports, roles, object_root/keep_alias, aliases and globals retains referenced support bodies. O2 constant arguments require internal, unescaped, unmutated parameters and agreement at every direct call | O(I+O+P+F); completed escape/mutation census precedes rewriting. No signature/ABI change. Dead definitions become declarations; CFG compaction releases their bodies |

I/O/B/E/P/S/F denote input instructions/operands/blocks/edges/parameters/slots/
functions at the owning pass. All state is unit/function-owned vectors, pools or
flat ID indexes; there is no persistent cache, per-node owning pointer or
recursive graph destruction. The cloner retains immutable input pools only for
its bounded invocation; it releases them before ordinary optimization.

One optional expansion invocation follows initial scalar/region cleanup.
A second region sweep runs only after expansion. The inherited scalar/CFG
schedule still has at most five scalar worklists, two CSE and local-slot sweeps,
one promotion/diamond/bypass and three structural sweeps; the entry/call/root
work adds one scalar and at most four structural sweeps. No repeated whole-unit
fixed point. Admission sees immutable body costs and reserves nested expansion
separately. Later local/CFG passes cannot introduce calls or cycles. Retaining
originals plus reserved clones bounds pool growth linearly before sparse slot
promotion's <=2I/3O bound. All later passes shrink. A loose combined accounting
envelope is 2^24*(I+O+B+P+S+V+F+1) charged operations/transient bytes over input
pools (V: values); it is not a typical-memory claim. Native allocation policy
remains the inherited backend's responsibility, without creating a PA33 gate.

## Remaining implementation and concrete handoff boundary

1. Memory/aggregate facts: projected-object alias/lifetime proofs, scalar
   replacement, load/copy forwarding and exceptional memory state. Inlined
   object arguments/results are correct independent homes; eliminating their
   copies requires these missing memory proofs.
2. Specialized call facts: candidate-specific folding of exception-bearing
   bodies after constant/builtin query evaluation. Unconditional EH expansion
   would violate the guarded-call quality fixtures; it is not a safe extension
   of the completed context-independent admission.
3. Loop facts: finite trip counts, effect-free loop deletion/fill, hoisting and
   O3 unrolling/versioning. These require trip/effect proofs and distinct runtime
   workloads; adding more scalar/inlining rounds cannot supply them.
4. Source/debug facts: binding/start spans and emitted lifecycle/declaration
   identities. Saved `student.tests/pa32/call-nested-cleanup.cpp` also exposes
   inherited nested-try flow construction failure at O0, before optimization.
   This reducer is retained as unfinished frontend work, not a passing test.

The 209 group extends from direct scalar calls through branching/typed-object
boundaries, support reachability, recursive/size guards, no-unwind/EH retirement,
readonly call DCE, cold admission, closed-world constants and measured profit
fallback. Remaining quality failures need the independent owners above.
They are unfinished implementation, not audit questions or waived requirements.

## Performance acceptance and evidence

209 frozen binaries, sources, commands, hashes, A/A calibration, six ABBA blocks,
checked outputs, compiler wall/RSS, runtime/text and all diagnostic runs are
retained under `/home/vishvananda/work/private/v4codex/artifacts/pa32-209`.
Durable raw JSON and source bindings: `student.tests/pa32/evidence209/`.
[Binding and raw samples](../student.tests/pa32/evidence209/binding.json) freeze
all inputs/flags/binaries and record diagnostic as well as accepted data.

| Fixed workload | Compiler ratio (range) | Compile ms A/B | RSS KiB A/B | Runtime ratio (range) | Runtime ms A/B | Object text bytes A/B |
| --- | --- | --- | --- | --- | --- | --- |
| Division calls O1 (control) | 1.181 (1.047–1.208) | 141.38/166.00 | 12572/13660 | 1.006 (0.974–1.022) | 134.00/135.72 | 128438/128438 |
| Regions O1 | 1.322 (1.096–1.550) | 133.12/173.83 | 12488/13832 | 0.803 (0.763–0.907) | 161.09/131.62 | 128503/128438 |
| Cheap calls O1 | 1.531 (1.455–1.668) | 132.77/212.38 | 12368/16816 | 0.976 (0.957–0.981) | 64.64/63.08 | 128427/132000 |
| Constant arguments O2 | 1.214 (1.104–1.282) | 131.51/158.67 | 12004/12716 | 0.932 (0.899–0.945) | 70.83/65.56 | 134487/134427 |

Each affected runtime improves in all six pairs; the unchanged divider control
is byte-identical. Ratios are paired means/medians, not ratios of marginal
medians. Compiler loads varied substantially on the host; A/A ranges and all
observations are retained rather than censored.

Fixed template-heavy common O1 workloads: memory/floating/exceptions/pruning
compiler ratios 1.043/1.068/1.087/0.994, runtime 1.018/0.956/0.998/1.025,
RSS KiB 30344/30584, 30452/30760, 30384/30640, 36024/36416. Memory and
pruning objects are byte-identical controls; floating text 124813/124867,
exceptions 125097/125067 bytes. No common memory/pruning speedup or slowdown
is attributed to the compiler. Every O0 common object is byte-identical;
compiler ratios 1.011/1.004/0.982/1.005. Compiler-owned folding.cpp at O0:
ratio 1.006 (0.846–1.324), wall medians 1.109/1.103s, RSS 75552/75856 KiB,
identical text; standalone runtime inapplicable (no executable entry). This
component benchmark is not a claim of whole-compiler self-hosting.

Budgets for this group: paired-median compiler wall <=2x, peak compiler RSS
<=1.75x, affected executable text <=1.5x; every accepted affected workload must
show repeatable runtime benefit. Unchanged images are controls, not benefit
claims. These are policy budgets; course IR envelopes and semantic requirements
remain independently mandatory. O2 constant propagation has the same budgets. These opt-in levels trade
bounded compile work for reused executable work: the cheap-call benchmark
adds about 80ms compiling 1,200 functions and saves about 1.55ms per measured
six-million-call execution of one function (roughly 52 executions to amortize
that entire compile delta). Region cleanup and constant propagation repay their
compile deltas sooner. The small cheap-call gain is accepted with 2.8% text
growth; no universal per-program speedup is claimed.

Initial division-loop inlining regressed runtime 21.4% (all pairs). The current
policy retains expensive calls in cyclic or >128-instruction callers. An
intermediate common-memory regression of 4.9% came with cold arithmetic
expansion that shifted the identical hot loop. Restricting expensive expansion
to small acyclic callers restores that memory object byte for byte. All rejected
measurements remain in evidence; these are resolved optional-policy costs,
not excuses based on later allocator work.

Inherited evidence is preserved in `evidence207/binding.json` and
`evidence208/binding.json`. Historical O1/O0 scalar: compiler 1.896, RSS
10180/14576 KiB, runtime 0.868, text 170800/110200 bytes. 208 affected slots:
compiler 1.495, runtime 0.849, text 149200/144400; dominance: compiler 1.271,
runtime 0.810, text 220000/148000. Their budgets remain compiler <=2x,
RSS <=1.75x, no text growth. The earlier arbitrary >=10% runtime target was
already reclassified diagnostic with evidence; repeatable benefit remains
required. Current-stage scoped acceptance governs all inherited evidence;
historical misses do not permanently fail a corrected implementation.

## Handoff ledger and independent review

209 entry HEAD: `d2e412d65f3fca9c8e225c575721c6b017651204`. Previous goal turn
was progress (validated dataflow implementation); no live build at entry.
207: 0 -> 110/219; 208: 110 -> 127/219. 209 implementation commits:
`acf38fc7` typed calls, EH/roots/constants and executable reducers;
`410c67bf` measured expensive-call admission and bounded expansion selection.
Final validation/evidence commit records this handoff boundary. Stage/review markers above persist.

Latest course result: **178/219**, **41 failures**, versus entry **127/219**,
**92 failures**. Sequential final root checks and failure-set comparison are recorded in the binding.
Ralph's **127/425** entry census has a different denominator; preserve it as
context, never substitute it for the root harness's unchanged 219 fixtures.
No course fixture, reference, harness or comparison rule was changed.
`make test-report-through-pa31`: **5178/5178**. File audit exits **0** with
four inherited header warnings. Required debug target: direct **4/5**, source
**0/3**; O3 loop/debug and source-location checks remain unfinished. Separately
completed debug replay **25/25**; primary nodebug replay **25/25**. Both final
root reports were run sequentially: an earlier concurrent attempt interleaved
the root summary store and its logs are diagnostic only.
Explicit `calls.py`: 63 boundary cases x four levels x two native paths,
source EH/debug replay, constant/escape/mutation and growth guards.
Inherited `local.py`: 506 cases; `dataflow.py`: 134 cases at four levels,
both native paths and EH/stress controls. Its call-cycle guard now marks the
identity helper no_inline so the call remains present under optional inlining;
coverage and its original conservative property are preserved.

Independent audit still owed: whole-stage source/template-to-ELF traces, useful
fact/legality/profit/invalidation traces, whole-pipeline bounds/fallback,
EH/native liveness and metadata, benchmark scope and profitability. These are
review questions distinct from the unfinished implementations above. Neither is
waived. This is an implementation handoff to Ralph, not PA32 advancement.

`python3 student.tests/pa32/verify_calls_handoff.py` checks source/binary/input/
object/log bindings (528 source bindings), 1,204 raw observations and their
ABBA summaries, budgets, unchanged images, 51 removed failures and contract
contents. It does not certify whole-stage completion.
