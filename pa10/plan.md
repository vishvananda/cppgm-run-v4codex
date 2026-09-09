# PA10 implementation

Stage base commit: c2a4786ebe60f52a7384c8c6e1ce12bd7e0b173a
Independent full-stage review: starts at a5e10cfd; final validation in progress.
See [audit](audit.md) for reconstructed architecture, new defects and fixes.

## Design/spec alignment and ownership

One streaming syntax/semantic graph feeds PA8's typed Program directly; PA9's
shared typed encoder owns ABI names. No textual phase transport, duplicated
semantic resolution, host compilation, or reference delegation implements output.

| Group / owner | Data flow and complexity | Validation |
|---|---|---|
| Symbols, calls, defaults / semantic declarations + lowering symbols | Resolved EntityId/TypeId → SymbolId/signatures/ABI; dense maps and canonical signature cache, expected O(n) | Course calls/defaults/function-pointer fixtures; nested-call and reference-return native checks |
| Scalars, casts, addresses / lowering values + expressions | Selected conversions/lvalue facts → numeric operands/slots; single traversal and recycled call scratch, O(n) | Course arithmetic/array/enum/cast fixtures; narrow/bool, volatile, assignment-order native checks |
| Control flow / semantic jump validation + lowering control flow | Resolved targets → blocks; immutable initialization prefixes with DFS intervals, O(n) construction and O(1) bypass check per edge | Course branches/loops/switch/goto; short-circuit controls; invalid-jump rejection checks |
| Static/local initialization / semantic constant facts + lowering initialization | Complete (NodeId, target TypeId) facts → typed data/relocations/stores; sealed-TU active/success/failure memoization, expected O(n) | Course global/array/reference fixtures; native storage/padding checks; two documented reference corrections |
| Driver/IR boundary / lowering driver + PA8 model | TU arenas released after lowering; function builders released per body; Program retained through explicit output; O(n) writer and one opt-in audit | Earlier modes; 123 successful source/control inputs under --validate-lowir and ASan/UBSan |

## Remaining groups

Independent audit corrections are implemented; refreshed performance, sanitizer
and final root exit validation remain before closing the stage. Object-model
helpers, template lowering, MIR/native optimization, ELF and self-hosting belong
to later stages; current procedural behavior preserves those typed boundaries.

## Performance evidence

O0 adds required lowering and no optional generated-code optimization passes.
[Protocol](../student.tests/pa10/performance-protocol.md) freezes A/B binaries,
flags and growing inputs, with AAAA/ABBA/ABBA, equivalence checks, latency/RSS,
runtime/text and separate work telemetry. Historical observations are preserved.
The stage base is a stub, not an equivalent timing baseline. Compare the first
correct common subset against the ownership audit and final sequencing fix.
The static-reference cache reduces the long-case median from 2.819 s to .113 s,
with ~25x benefit in both ABBA blocks; peak RSS rises 2064 KiB. Compiler text
grows 23296 bytes (4.38%); ordinary workloads meet diagnostic budgets. Paired native executables
are byte-identical (206/434/230 bytes), with no runtime speed claim. The
[report](performance.md) records all four dimensions, spread and work counters.
Self-selected diagnostic budgets are not course gates. Inherited PA9 naming-tool
budgets remain with the unchanged naming tool; later-stage limits stay scoped
to their assignments. Mandated complexity, correctness and coverage remain.

## Handoff ledger

- Entry: clean base above, 0/121 PA10; earlier PAs/file audit reported passing.
- Implementation: 75 → 119/121; all five controls pass. Earlier PAs 904/904.
- Reference review: two constant-initialization sidecars corrected using cited
  C++11 rules and executable reduced observations; sources/statuses/comparison
  unchanged. [Proof and pinned bundle](reference-corrections.md). PA10 121/121.
- Ownership audit: typed in-memory validation, constant memoization, shared
  signatures/scratch, volatile discard, bool storage, aggregate padding and
  ABI/IR name separation. No generated-code optimization pass introduced.
- Final sequencing increment `63592ef0`: RHS precedes a computed compound-LHS
  address, evaluated once. Eight native programs, seven semantic rejects and
  123 in-memory IR audits pass, also under ASan/UBSan (leaks and halt-on-UB).
  Full root report passes 1025/1025.
- Final exit: `make test-pa10` 121/121 plus 5/5 controls; prior through PA9
  904/904; file audit 126 files; diff check passes. Final and historical frozen
  artifacts verify. Both pinned-reference reducers reproduce the documented
  defect. Existing stage failures fell from 121 to zero without reduced coverage.
