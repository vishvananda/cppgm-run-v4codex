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

No optimizer is introduced. Evidence budgets fixed before measurement:
compiler paired wall <=10% + A/A noise; RSS <=20% +1 MiB; host text growth
<=25%; 4x input <6x wall / <5x RSS +1 MiB; observations >20x startup.
Executable paired runtime <=5% + A/A noise, text growth 0% for unchanged
programs. Freeze binaries/flags/inputs; retain AAAA + two ABBA blocks, output
equivalence, separate telemetry, and checked volatile runtime workloads.
Earlier frontend evidence remains in PA7; native emission stays with PA24.

## Handoff ledger

- Entry inspection: clean worktree at stage base, previous PA7 completion is
  authoritative progress; PA8 implementation has not begun. Read AGENTS,
  testing/reference guidance, spec, handout and grammar. No live work to resume.
- First increment: all four groups implemented in shared typed pools and text
  adapters; all 109 original failures eliminated without fixture/harness changes.
  Frozen first working binary: `/tmp/pa8-evidence/lowir-initial`.
- Second increment: 76 independent semantic cases, typed API identities/local
  invariants and ASan/UBSan checks; signalling NaN and wide integer literal
  preservation fixed. Direct constructors reject malformed local shape. Native
  checks cover the complete sum domain, aliasing, callbacks and floating memory.
  Cumulative first run: 793/793; PA8 after changes: 109/109.
- Remaining: final sanitizer rerun after pool telemetry changes, compiler/runtime
  measurements, final cumulative and file audits, evidence and clean commits.
- Handoff reason: work ongoing; no incomplete checkpoint proposed.
