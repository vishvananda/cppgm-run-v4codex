# PA21 checkpoint audit 109

Stage base commit: `ac988ea33d4997b44e82baaca5a86623fff3127a`.
Previous review: `f65eae8d7d434735a0ce981173a347eb8f8a1e59`.
Audit entry: `e6582e3751084e80a41a12a623d2cd83e183b25c`.
Last reviewed commit: `f57bdd3b0e5c7dd2dd87aacaecc73c9ddc96d114`.
Target: **PA21 full-stage**. Phase: **checkpointAudit complete; implementation remains**.

The checkpoint preserves progress: **108/116 pass, the same eight failures**;
PA1–20 pass **3596/3596**. This accepts the accumulated checkpoint review and
its repairs, not whole-stage completion. PA22 advancement remains blocked by
the eight required PA21 comparisons. The [105 audit](audit105.md) is preserved.

## Complete range and findings

All **14 entry commits**, their combined **41 implementation paths**, the
repair `bd6bfd71`, and measurement-driver completion `f57bdd3b` were reviewed, including interactions between handoffs. The
[manifest](../student.tests/pa21/audit109-review.json) retains full IDs, per-commit
review-diff hashes, entry and final source hashes, verified archived binaries,
and entry controls. No range was narrowed to the latest handoff. The preceding
turn is classified as **progress**, supported by committed implementation and
validation; no live previous job was assumed. The entry worktree was clean.

| Commits | Contribution and interaction reviewed |
|---|---|
| `06b2d989` | Prior review records; establishes the inherited baseline. |
| `9c4f64da`, `301ef6fd`, `e80ad0c7`, `c64e88fb`, `14225657` | Source exception facts, handler parameters, implicit moves, protected-context continuations, constructor prefixes, catch defaults, removal of empty lifetime records, reference proof and measurements. |
| `b821682b`, `20476a77`, `a2b9e823` | Full-expression effects and result ownership, condition/logical joins, destination defaults, final consumers, range operations, display names and validation. |
| `efcb52b1`, `2e392cec`, `a30acab5`, `ac3a28e8`, `e6582e37` | Result transfer, nonthrowing boundaries, partial aggregates/arrays, helper transfer proofs, constructor defaults, parser prediction, zero initialization, corrected prefix reference and archived evidence. |
| `bd6bfd71`, `f57bdd3b` | Audit repair and reproducible measurement baselines: semantic jump destinations, ordered scope/handler exits, protected-entry rejection and reachable-label traversal. |

**Nonlocal jumps omitted exception-region exits.** Return lowering had an EH
exit path, while `goto`, `break` and `continue` only destroyed lexical objects
and jumped. The supplied object backend rejects the resulting mismatched region
stacks; catch exits also lose exception-object retirement. This affects nested
handlers, loops, switches, ranges and template lambdas. The reduced
[controls](../student.tests/pa21/audit109.py) improve **7/22 → 22/22**: thirteen
execution defects and two missing required rejections are repaired.

The existing semantic jump walk now records each target's lexical try/handler
identity. Lowering consumes that fact, destroys each exited scope in order,
closes its region and ends its handler before continuing outward. It updates
the active context while emitting later destructors, so a throwing destructor
outside an exited try cannot return to that try's handlers. Return uses this
same owner. Targets inside the current handler retain it. Missing nonzero
ancestor identities are invariant errors rather than textual recovery.

**Protected-scope entry and label traversal were incomplete.** The semantic
initialization-prefix ancestry check now gives try bodies an entry barrier;
existing handler barriers remain. Goto and switch entry into those scopes are
rejected even when they contain no destructible local. The lowering entry walk
now includes try/handler children, preserving labels reached past terminated
fallthrough. The fix uses existing traversal/identity owners, not a second
parser or a whole-function search for each jump.

The proof is C++11 [except]/3 (no goto/switch entry), [except.throw]/2,4 (nearest
unexited try and exception-object retirement), [except.handle]/7,16 (active
handler and catch-parameter lifetime), and [except.ctor]/1 (reverse unwinding):
[local standard](../doc/n3485.txt:21326),
[exception lifetime](../doc/n3485.txt:21421). LowIR's
[handler-stack contract](../pa8/lowir.md#handler-stack-management) requires balanced
regions. Tests check destruction counts/order, retained enclosing handlers,
throwing outer cleanup, source/template composition and invalid-entry rejection.

## Architecture and ownership audit

The [fresh trace source](../student.tests/pa21/audit109_trace.cpp) follows a
polymorphic declaration and two demanded template specializations through class
lists, captures, RTTI, throws and handler `continue` to checked native execution.
The [trace](../student.tests/pa21/audit109-trace.json) retains LowIR, stats,
syscalls, native hash and disassembly. Ordinary and instrumented/validated output
are identical; compiler tracing contains one `execve`, with no host/reference
frontend invocation. The authorized supplied object backend and host linker
produce a checked **2244-byte `.text`** executable.

- Immutable source buffers and the streaming cursor feed retained source
  occurrences. Declaration-prefix prediction inspects tokens without abandoned
  parses. The trace has **550 parsed nodes**, **two template body transitions**,
  **37 fixed expression recipes / 74 uses**, and **two closures**. Runtime catch
  objects and lexical jump destinations remain specialization-specific facts;
  fixed operands/conversions retain shared semantic recipes.
- Canonical entity/type/node IDs key throw conversions, handler initialization,
  exception facts, constructor/proof-mode independence, list plans and layouts.
  Scope lookup and retained template environments use the earlier indexed owners.
  There is **one list plan, two backing objects, one list representation**, and
  **three RTTI expression records**. No rendered signature or name replaces
  semantic identity. Symbol display changes do not change ABI identity.
- Demand still distinguishes checking, completion and emission. Constructor
  actions are classified only after completion; unavailable facts conservatively
  prevent helper selection. Semantic exception/default/suffix caches see completed
  recipes and have TU lifetimes. New jump facts are sparse per occurrence; they
  do not restore the previously removed empty records for ordinary returns.
- Typed lowering consumes selected conversions, exact destination addresses,
  lifetime prefixes and active context IDs. Cleanup sharing keys include live
  state and a terminal determined by the full parent-linked context. Raw
  constructor-region changes get distinct resume identities. Conditional throw
  arms restore sibling state; completed destinations are published before
  argument-temporary destruction without mutating earlier unwind snapshots.
- Function-owned context/prefix/continuation state resets between bodies;
  instruction scratch for nonthrowing boundaries is released immediately after
  the single function-slice rewrite. TU facts and program LowIR have their
  existing explicit owners. No new per-node owning pointer, copied syntax graph,
  process-global cache, global retry or production text roundtrip was introduced.

Student MIR selection/allocation, ELF encoding, debug-backend improvements and
self-hosting belong to PA24–34. The supplied-backend trace checks their boundary;
it does not claim those later implementations. O0 disassembly retains real calls,
loop branches and frame traffic; no allocator or spill improvement is inferred.

## Legality, profitability and work budgets

Named-result reuse requires the recorded eligible nonvolatile local and retained
unwind ownership through successful return. Default-argument temporaries, partial
member/array completion, delegated construction and parameter ownership are
covered together by the 134 ownership controls. Helper transport requires
storage-independent operands; early transfer additionally requires nonthrowing,
single-argument, completed constructor actions with no external effects. Unknown
proofs retain ordered destination construction. Aliases, self pointers, observable
copies and throwing prefixes exercise those conservative paths.

Full-expression cleanup facts distinguish result omission and observable effects
in two cache bytes per node. Work follows demanded facts, traversed edges and
emitted regions. Prefix construction is memoized; helper/action traversal is
linear, block presentation sorting is O(b log b), and nonthrowing-boundary
insertion uses one function-sized scratch slice. Array expansion remains capped
at **eight elements**, with counted loops above it. New jump lowering costs one
sparse target fact and work proportional to the contexts/cleanup operations it
must actually exit. It adds no fixed point, code cloning or unbounded search.

[Performance 109](performance109.md) measures the entire accumulated range from
the last-reviewed compiler to this code tip with frozen inputs/binaries, A/A
calibration, ABBA pairs, compiler latency/RSS, checked runtime and text/payload
size. Earlier [106](performance106.md), [107](performance107.md) and
[108](performance108.md) evidence remains, including unfavorable observations and
interrupted runs. Archived binary hashes and 108's executed-driver reconstruction
were verified. Required O0 regions and completed-subobject cleanup costs are
explicit; invalid earlier EH output is never used as a speedup baseline.

Spec §9 acceptance is scoped to **PA21/O0**. Historical **+15%, +16 MiB, 5.5×**
self-selected diagnostics remain evidence, not additional gates. The handout
sets no numeric latency/RSS/text ceiling. Correctness, contract comparisons,
coverage and mandated work/growth bounds remain binding. Later backend costs
are identified rather than turned into a PA21 exit requirement.

## References, coverage and validation

No reference changed during this audit. The three changed references since 105
are the proved [nested-handler correction](reference-corrections106.md) and
[aggregate-prefix correction](reference-corrections108.md). Their reducers,
C++11/LowIR proofs, bundle revision and exact reconstruction from original bytes
were reviewed. Fresh reconstruction and execution pass; the
[prefix replay](../student.tests/pa21/audit109-reference108.json) again shows the
original reference/old implementation fail while the revised reference/current
implementation pass. Compiler agreement is corroboration, not the proof.
The historical [102 RTTI correction](reference-corrections102.md) is preserved.

[Validation](../student.tests/pa21/audit109-validation.json) records:

- Required prior-through command: **3596/3596**, exit 0.
- `make test-pa21`: **108/116**, exit 2, the **exact entry eight-failure set**.
- `make test-report-through-pa21`: **3704/3712**, the same eight failures.
- Required file audit: exit 0, the same three advisory header-body warnings.
- **116 source identities** and the **15048-path contract/harness inventory**
  exactly match checkpoint 108. Only the four proved historical reference paths
  differ from the stage base. No status, comparison rule or coverage was reduced.
- Personal suites: **580/581**, including **22/22 audit controls**. The sole
  inherited freestanding RTTI discrepancy, `public_base_inside_private_derived`,
  remains recorded; its host-runtime counterpart passes. No required course
  failure is waived by that distinction.

## Remaining work and ledger

Two broad integration groups remain. **EH/lifetime and support identity** owns
three source-handler/static/continuation comparisons and two initializer-list
backing-storage/lifetime comparisons. **Generated construction and template
lowering** owns two special-member/helper comparisons and the constant-array O0
policy interaction with PA17. All eight remain required implementation work;
native execution does not substitute for relaxed LowIR comparison.

Handoffs 106–108 improved 71 → 92 → 101 → 108 cases, but splitting source exits,
full-expression consumers and completed destinations left avoidable fragmentation
across the same lifetime owners. The missed nonlocal jump paths demonstrate it.
Finish each remaining ownership group with its handler/list/template compositions
and integrated validation instead of another sequence of fixture-shaped repairs.

| Audit | Range and disposition |
|---|---|
| 105 | `ac988ea3..f65eae8d`; RTTI recipe reuse and copied-subobject cleanup repaired; 45 failures retained. |
| 109 | `f65eae8d..f57bdd3b`; all 14 entry commits and interactions reviewed; protected-scope jump ownership/rejection/labels repaired; accumulated performance, references and coverage verified; earlier/file/progress gates pass; eight unchanged failures remain. |

The marker names the committed code tip. Audit, plan and evidence records follow
in a separate commit with no further implementation edits.
