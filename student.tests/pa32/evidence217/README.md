# Implementation 217 evidence

Entry `7e04081b611b89cd46e5b294a963aba891c36073`; implementation commits
`cc7d3c20`, `940d3396`, `c81197b0`. Course results move **213/219 -> 219/219**:
five original failures are fixed by implementation and one by a proved oracle
correction. The unchanged course input population and comparison machinery are
bound in `binding.json`, together with the exact two-sidecar correction.

This is an implementation handoff. **Three source-debug checks remain required
implementation work**, and independent accumulated review since `40151904`
remains pending. Both review markers are preserved in the compact
[plan](../../../pa32/plan.md). Passing course checks does not certify the stage.

## Owners and proofs

- `symbol_display` renders canonical entity/scope identities. A single dense
  declaration census assigns overload ordinals independent of emission order;
  the parser stores two lambda declarator cursor ordinals, not deferred tokens.
  Rendered strings never key semantics or ABI merging. The existing collision
  allocator handles ambiguous flattened names. Earlier source-view contracts
  retain their presentation; production source/object LowIR shares one graph.
  Work is O(declarations + rendered bytes), with 4 bytes/entity and a temporary
  flat count table; lambda ranges use 16 bytes each on this target. All storage
  releases with its TU, without per-node ownership or global invalidation.
- `binding`, `address`, `field` and `base_projection` carry most-derived class
  identity and current offset for known nonreference objects. A base conversion
  consumes the completed layout by identity, including successive adjustments.
  Unknown references and loaded pointers still use dynamic offsets. Facts cost
  16 bytes in a transient Value and O(1) lookup/conversion; their result is
  ordinary typed LowIR arithmetic, not a frontend side channel to encoding.
- The existing semantic destructor-effect fact covers user bodies, subobject
  destruction and virtual dispatch. An empty nonvirtual destruction cannot
  observe intermediate vptr stores; required complete/base ABI bodies remain.
  Real virtual destructor dispatch and base/member/reference paths are tested.
- Weak inline bodies and their ABI aliases share emission lifetime. Rooted,
  addressed or called definitions remain, as do ordinary/internal explicit
  exported aliases. The reachability walk uses compact IDs and a deduplicated
  worklist. Work/storage remain linear in symbols, aliases, data and IR; dead
  aliases disappear with their bodies. There is no additional optimization
  fixed point or growth allowance.
- One internal `no_inline` termination adapter is shared by landing pads. It
  retains the required named LowIR definition without cloning a cold catch/
  terminate action. This adds no normal-path instructions and reduces text.
- Benchmark construction exposed template members acquiring the C linkage of
  a demanding function. Semantic declaration attribute publication now applies
  [C++11 draft N3485 7.5/4, dcl.link](../../../doc/n3485.txt): C linkage is ignored
  for class member names/member-function types. This is an O(1) owner-kind
  rule, not a mangling repair or recognition of library/test spellings.
- The even-stride fixture correction has a separate
  [reducer, LowIR contract proof and pinned-bundle identity](../../../pa32/reference-corrections.md).
  Its arithmetic invariant proves nontermination for different mod-eight
  residues. Timed executions are supporting observations, not the proof.
  The original input/status and validators/comparators are unchanged; the
  replacement outcome requires a conditional branch and forbids calls and
  memory accesses. Other finite-loop deletion requirements remain unchanged.

All existing optimizer unit/work/growth budgets remain operative; the plan
summarizes them and links their original evidence. The new lowering facts
reduce instructions without speculative memory or floating transformations.
Canonical typed lowering and direct MIR/ELF emission remain in use.

## Validation and boundary

`checks.json` records exact commands, exit statuses and SHA-256 digests of full
logs. Final root reports ran sequentially: earlier **5178/5178**, course
**219/219**, through PA32 **5397/5397**, file audit pass with four inherited
header warnings. Normal/debug object replay each pass **25/25**. Direct debug
passes **5/5**; the three source-debug lanes still fail their existing relaxed
comparisons, explicitly recorded. All inherited personal semantic reducers,
bounds tests and source/template/native traces were rerun and pass.

`source_identity.py` executes five source groups at O0–O3 and compares normal
objects and debug O1/O3 objects after text replay. It covers overload/collision
names, lambdas in template specializations, class declarations and demand inside
C linkage, by-value/complete/dynamic virtual bases, returned and member objects,
copy/move, actual virtual destructor effects, termination and explicit/weak/
addressed/rooted aliases. `pointer_congruence.py` checks the reducer, unchanged
input and corrected reference: equal, forward-residue and mismatched-residue
calls at four levels through both native paths, plus all 65,536 pairs in the
finite 8-bit model. The original pointer-loop guard tests still pass.

The handoff closes source/ABI/lifecycle identity and the loop-oracle proof.
Remaining source-debug closure needs assignment/value identities and statement
locations carried through scalar promotion, propagation and phi creation.
Declaration rendering and complete-object facts cannot reconstruct eliminated
source values. This is a separate unfinished cross-phase implementation group;
independent review additionally owns accumulated legality/architecture and
performance questions. Neither obligation is waived.

## Performance protocol and acceptance

The final set (`affected.json`, `common-o0.json`, `common-o1.json`,
`common-o3.json`, `selfhost.json`) contains **868 observations**. Frozen baseline
and final binaries, input hashes, flags, CPU affinity, A/A calibration and six
ABBA blocks are recorded. Compilation and execution have separate wall time and
peak RSS observations. All executable results are checked. Drivers and kernels
are compiled by this compiler; the host command only links objects. Telemetry
is measured separately on affected inputs and consistently on both common
compiler variants. Native text sizes and normalized kernel instruction
inspection accompany latency/RSS/runtime; no hardware-counter dependency.

The affected layout benchmark chains each result into the next call's input,
making load-to-return latency visible. Its 1200-wrapper compiler median is
0.810x, RSS 23952/22228 KiB, runtime 0.731x, text 66099/16865 bytes, and the
kernel shrinks from 14 to 6 instructions. All six paired runtime ratios show
improvement; the data justifies the bounded fact propagation.

Move construction has 1.002x compiler time, 38336/37912 KiB RSS, 0.981x runtime
and identical 98510-byte text/normal-path instructions. No speed claim is made
for that case. Cold action sharing has 0.978x compiler time, 40528/40848 KiB RSS,
1.010x runtime and text 170012/164040 bytes. It meets the retained-helper
requirement and saves 3.5% text without added hot work. The roughly 1% runtime
cost is disclosed; unchanged normal-path instructions and changed image layout
are recorded, not evidence of a new algorithmic slowdown. Placement is a later
machine-code owner, not an additional PA32 gate.

All common template-heavy memory/floating/EH/pruning objects at O0/O1/O3 are
byte-identical between A/B. Their paired compiler medians are 0.992–1.011x,
runtime medians 0.996–1.007x. The O0 compiler-component object is also identical:
compiler median 1.000x, RSS 77544/77888 KiB, 34301 text bytes; it has no runnable
entry. The new class-linkage reducer has no correct baseline executable and is
treated as a semantic correction, not a runtime comparison. Both timed landing
variants use valid C++ linkage so equivalent correct implementations are timed.

**280 historical observations** are preserved: 112 from an initial independent-
input layout/move run before the C-linkage failure, and 168 from the final
binaries while checks ran on other CPUs. The closed affected run remeasures
without simultaneous checks. Initial layout runtime was 1.039x with independent
inputs; it is not discarded. Every input and every observation, scheduling
outlier and spread remains, including a 4.69x paired block on byte-identical
pruning executables. Intermediate overlapping root reports had mixed census
totals; they are exploratory, superseded by the sequential final reports.

Acceptance follows spec.md's current-stage rule. Layout has repeatable useful
runtime/latency/text gains; required source/ABI fixes stay within linear work
and non-growing IR budgets; cold sharing discloses its small runtime tradeoff.
Inherited unsupported ratio/zero-growth/runtime targets remain diagnostics.
No mandated limit, correctness requirement, coverage or observation is removed.

Run `python3 student.tests/pa32/source_verify.py` to check the committed clean
handoff, implementation/evidence/artifact hashes, final statuses, failure
reduction, exact fixture delta, measurements and preserved review markers.
