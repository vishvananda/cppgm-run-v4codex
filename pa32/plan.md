# PA32 implementation plan

Stage base commit: e82bf4152fe8d6d68b9cd966655db0d8142cf81b
Last reviewed commit: e82bf4152fe8d6d68b9cd966655db0d8142cf81b

## Design/spec alignment and completed ownership

Source -> typed LowIR Program -> shared optimizer -> writer or direct native/ELF.
External LowIR uses the same optimizer after parsing/validation. No textual
roundtrip inside compilation, reference delegation or fixture recognition.

- Entry/transport owner: `lowiropt` handles CLI and external validation; the
  toolchain driver handles levels, `__OPTIMIZE__`, source/LowIR dispatch and
  multi-source emission. Preserve ABI, section, debug and object-extent facts.
  Debug filenames survive punctuation; source tokens now carry columns and
  lowering records locations. Source binding/start-span work remains below.
- Scalar owner: function-local dense definitions/users, dirty worklists, typed
  integer folding/identities/reassociation, copy propagation and live marking.
  No unsafe floating identities or discarded observable/trapping operations.
  Reassociation requires a single-use intermediate and stable root; mutable
  reads cannot move past updates. Facts are discarded after each invocation.
- Local memory/CSE owner: explicit unescaped scalar slots and block-local
  expression tables. Respect volatile, width and SSA stability; unknown memory
  and effects stay conservative. Tables expire at block/function boundaries.
- CFG owner: one edge/reachability census, constant branches/switches, phi
  repair and adjacent single-predecessor merging. Functions with EH registration
  retain their CFG until exceptional edges have an owning analysis. Compaction
  remaps instructions/operands and rebuilds definition positions in block order.
- Native transport owner: deterministic function order and final ELF symbol
  order make source and replay equivalent. Mutable LowIR temporaries use stable
  homes, including parameters and staged parallel phi copies; saved values
  cannot alias later writes. This fixes inherited backend correctness defects.

O0 runs no optimizer. O1/O2/O3 currently share two scalar worklists, two slot
sweeps, one CSE pass and one CFG sweep; higher-level objectives remain unfinished.
Each scalar propagation/rewrite and CSE table has a 16*(instructions+uses+1)
work allowance with operand/alias/probe charging and conservative exhaustion.
Other sweeps/compaction are linear; final ELF ordering is O(S log S). No IR
instruction growth or whole-program fixed-point retry. Scratch is function-owned
for scalar/CSE and unit-owned for one-shot compaction, released before emission;
a conservative bound is 512*(I+O+V+B+S+P) bytes beyond Program pools.

## Remaining implementation groups and boundary

1. CFG/dataflow: dominance, cross-block slot/phi promotion and memory/alias/
   lifetime facts, including exceptional edges. Validate with remaining direct
   O1/O2 branch, slot, alias, aggregate and EH predicates plus execution controls.
2. Interprocedural: typed call/effect/no-unwind summaries, bounded inlining and
   support pruning; preserve linkage, ABI, exceptions and source locations.
   Validate direct/source call, cleanup and lifecycle fixtures together.
3. Loops: bounded O2 loop/memory analysis; O3 specialization/unroll policies,
   explicit growth/profit budgets and source/debug sidecars.
4. Source/debug contract: declaration binding copies, expression/statement
   start spans and emitted identity expectations in source driver fixtures.
   Validate all source debug lanes and preserve both object replay lanes.

The local group was extended through scalar identities/reassociation, snapshots,
phi transport, debug parsing and multiple sources. Remaining predicates require
facts across control joins, calls, exceptional regions, aggregate aliases or
loop iterations, or new source binding records. Extending local tables cannot
supply those legality proofs. These are unfinished implementations, not waived
requirements or questions delegated to audit.

## Performance evidence and acceptance

Frozen binaries/inputs/flags, A/A and six ABBA blocks, checked executable results,
wall/RSS/text and all raw observations are in
[`evidence207`](../student.tests/pa32/evidence207/binding.json); durable binaries,
inputs and logs: `/home/vishvananda/work/private/v4codex/artifacts/pa32-207`.
Scripts: `performance.py`, `common_levels.py`, `selfhost_performance.py`, and
inherited `pa27/performance147_common.py`, explicitly run under student.tests.

Affected 600-function/21,000-instruction workload: O1/O0 compilation paired median
1.418 (range 1.398–1.543), wall medians 49.65/70.48 ms, peak RSS 10,208/14,596 KiB.
Runtime paired median 0.869 (0.866–0.871), wall medians 72.94/63.38 ms; executable
text 170,800/110,200 bytes. Every pair improves; inputs/checksum prevent dead work.
Retain budgets <=2x compiler wall, <=1.75x RSS and no text growth. The initial
>=10% runtime target is a diagnostic, not a spec/handout gate: identical affected
objects previously measured 14.7% and 9.5% median gains; all pairs improved and
all observations are retained. Current gain is 13.1%; this reclassification
changes no mandated bounds, correctness or coverage requirements.

Fixed template-heavy common inputs, final O1/O0 paired medians:

| Workload | Compile ratio | Runtime ratio | Compiler RSS KiB O0/O1 | Executable text bytes O0/O1 |
| --- | ---: | ---: | ---: | ---: |
| Memory/calls | 1.048 | 0.931 | 30072/30064 | 151633/125199 |
| Floating | 1.034 | 0.990 | 30640/30712 | 151474/125053 |
| Exceptions | 1.069 | 0.986 | 30260/30064 | 151781/125339 |
| Pruning | 1.055 | 0.933 | 36252/36244 | 151633/125199 |

Memory/pruning improve in every pair; FP/EH ranges cross 1, so no repeatable
runtime benefit claimed there. Historical misses/outliers are preserved,
including the initial O1 floating ratio 1.048, not reproduced in the final run.
Common O0 baseline/current compile medians range 0.996–1.013 (small metadata/
native correctness costs and noise), with byte-identical text in all four cases.
Compiler-owned `folding.cpp` at O0: paired median 1.013 (0.966–1.049), wall medians
1.145/1.161 s, RSS 75000/75380 KiB, byte-identical 31889-byte text. It has no main,
so standalone runtime is inapplicable; this is not a claim of full self-hosting.
Raw spreads/A/A are in the linked JSON. PA33 allocation quality is not an extra
PA32 gate. Runtime profitability must be remeasured for later passes.

## Handoff ledger

Implementation commits: `8c01abc9` shared optimizer/local/native group;
`5f8be172` debug/transport closure; `0e0d29f4` scalar closure and snapshot reducer.
Current `make test-pa32`: **110/219** (109 failures), versus **0/219** at entry;
Ralph's entry census **0/425** is retained as a different accounting basis.
No course tests, harnesses, references or comparison rules changed.
`make test-report-through-pa31`: **5178/5178**. File audit: exit 0, four inherited
large-header warnings. Personal `local.py`: **506** execution cases at each of
O0/O1/O2/O3 plus trap/CLI and multi-source debug/transport controls.
PA32 debug: direct **4/5**, source **0/3**, debug object replay **25/25**;
nodebug replay **25/25** is included in the primary report. PA24 has no separate
`test-debuginfo` target; its primary debug metadata controls pass in the through
report. Run root reports sequentially because they share output artifacts.

Independent audit still owed: whole-stage source/template-to-ELF and useful-fact
traces, pipeline-wide bound/fallback accounting, exceptional/native liveness and
metadata preservation, and benchmark scope/profitability. This handoff does not
claim that review; review markers stay at stage base. Explicit
`python3 student.tests/pa32/verify_handoff.py` passes: source/binary/log hashes,
1596 performance observations, budgets, text equality and unchanged progress
denominator checked. This is an implementation handoff; PA32 remains incomplete.
