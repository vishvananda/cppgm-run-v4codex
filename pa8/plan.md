# PA8 implementation plan and ledger

Target: **PA8 full-stage**. Phase: **implement**.
Stage base commit: `7e4297484f117980a3f1d55f43932d6bda3a8cc1`.
Last reviewed commit: `7e4297484f117980a3f1d55f43932d6bda3a8cc1`.
Entry: **0/109** confirmed by fresh unchanged course run. First implementation:
**109/109**, including all three native exercise checks; file audit 96 files passes.

## Design and remaining groups

| Owner | Data flow and work | Complexity / validation |
| --- | --- | --- |
| LowIR model | Canonical types and interned names; unit-owned flat instruction/operand storage; typed symbol/value/block references. Constructors serve later direct lowering and exercises. | Expected linear storage and construction; personal typed API checks. |
| Text adapters | Streaming lexical cursor -> typed records -> deterministic writer. No stored source spellings as instruction implementations, no production serialization transport. | Linear bytes + IR; all successful roundtrips, multifile and helper-only units. |
| Validator | Indexed symbols, signatures, values and blocks -> operand/metadata checks and explicit predecessor edges. | O(IR + edges), no repeated whole-unit searches; all required rejection fixtures and personal probes. |
| Exercises | Model constructors -> shared writer -> supplied harness native backend. | Sum, aliased swap, ordered indirect calls; behavioral suite and independent runtime inputs. |

No optimizer is introduced. Record compiler wall/RSS and applicable executable
runtime/text measurements with fixed binaries/flags/inputs, A/A calibration,
ABBA observations and checked results before any performance claim. Later
frontend/native requirements remain owned by their milestones.

## Handoff ledger

- Entry inspection: clean worktree at stage base, previous PA7 completion is
  authoritative progress; PA8 implementation has not begun. Read AGENTS,
  testing/reference guidance, spec, handout and grammar. No live work to resume.
- First increment: all four groups implemented in shared typed pools and text
  adapters; all 109 original failures eliminated without fixture/harness changes.
  Frozen first working binary: `/tmp/pa8-evidence/lowir-initial`.
- Remaining: independent semantic/API and sanitizer checks, compiler/runtime
  evidence, cumulative report, final audit and clean commits.
- Handoff reason: work ongoing; no incomplete checkpoint proposed.
