# PA8 final plan and ledger

Target: **PA8 full-stage**. Phase: **complete**.
Stage base commit: `7e4297484f117980a3f1d55f43932d6bda3a8cc1`.
Last reviewed commit: `7e4297484f117980a3f1d55f43932d6bda3a8cc1`.
Entry: **0/109**, confirmed by unchanged course run (incoming 0/127 was stale).
Final: **109/109**; PA1–PA8 **793/793**, earlier PA1–PA7 **684/684**.

## Design/spec alignment and completed groups

| Owner | Data flow | Complexity and validation |
| --- | --- | --- |
| Model / constructors | Inline canonical types, interned names, typed IDs and unit-owned flat pools; direct exercise/later lowering construction. | Expected linear construction/storage; API growth, isolation, shape and writer checks pass. |
| Reader / writer | One lexical view over each immutable input -> typed records -> shared deterministic writer; no textual production transport. | Linear bytes + IR; all successful course roundtrips, multifile and helper-only cases pass. |
| Validator | Symbol/signature/value/block IDs -> type, metadata, ownership and predecessor checks. | O(IR + E log E), one CFG edge sort, O(IR + E) scratch; all course rejections and personal probes pass. |
| Exercises | Direct model construction -> writer -> supplied harness backend. | Sum domain proves bounded i64 arithmetic; swap preserves aliases; indirect calls preserve order. Course/native probes pass. |

[Implementation audit](audit.md) records ownership, fact flow, lifetimes and the
later design boundary. [Personal checks](../student.tests/pa8/README.md) include
76 semantic cases, writer fixed points, typed API checks and final ASan/UBSan.
No optimizer was introduced and no course fixture/reference/harness was changed.

## Performance evidence

[Measurements](performance.md): frozen first working A (`66167cf72`) versus
final B (`01d39a2f6`), fixed flags/inputs, AAAA calibration + two ABBA blocks,
168 primary and 14 small construction/adapter observations, all retained.
Budgets fixed before measurement: paired compiler wall <=10% + A/A noise;
RSS <=20% +1 MiB; host text <=25% growth; 4x input <6x wall / <5x RSS +1 MiB;
samples >20x startup; native runtime <=5% + noise and text growth 0%.

All budgets pass. Additional correctness/shape checks cost 2.52–3.72% compiler
wall; maximum median RSS growth 22 KiB; host text +3.73%. Fourfold input scales
3.82–3.98x wall / 3.45–3.83x RSS. All four native pairs are byte-identical;
text spans are 234/250/270/417 bytes. Runtime, paired spread, telemetry overhead
and the callback A/A outlier are disclosed. No speedup claim is made.

## Handoff ledger

- `da047b4a9`: initial plan/review markers committed before implementation.
  Previous PA7 completion classified as progress; no live work required resuming.
- `66167cf72`: all model, text, validation and exercise groups implemented;
  original 109 failures eliminated without reduced coverage.
- `01d39a2f6`: signalling NaNs and wide literal signs preserved, local shape
  invariants enforced, independent semantic/API/native checks and pool telemetry.
- Final evidence: final sanitizers pass; required PA8, prior-through and full
  cumulative gates pass; file audit passes 97 files without warnings. Frozen
  performance outputs/work/hashes verify. Review markers remain for Ralph audit.
- Remaining PA8 groups: **none**. Handoff reason: full-stage completion; no
  incomplete checkpoint or advance into PA9. Evidence is committed, and clean
  status is checked after the final required commands.
