# PA2 implementation plan

Stage base commit: `be4bff26fcc81762196d29d4f0b0c59a20110c9d`
Last reviewed commit: `be4bff26fcc81762196d29d4f0b0c59a20110c9d`
Target: **PA2 full-stage**. Baseline: 0/26; current: 26/26, through-PA2: 80/80.

## Design and validation groups

Immutable source → inherited PA1 cursor/identifier table → typed post-token
cursor → explicit PA2 output adapter. No textual phase transport or complete
token vectors. Names/suffixes retain TU-owned IDs; token payload scratch lives
until the next pull. Only maximal adjacent string sequences need deferred
storage, because their final encoding/suffix determines conversion.

| Group / owner | Data flow and complexity | Validation |
| --- | --- | --- |
| Simple classification / post-token model | PP kind/spelling → bounded immutable keyword/operator metadata → typed kind; O(bytes) | 100-simple, invalid PP tokens, cursor identity |
| Numeric conversion / literal decoder | PP-number → grammar/base/suffix → ABI type/value or UD prefix + interned suffix; O(bytes), checked accumulation | zero, suffixes, decimal/octal/hex limits, floats, malformed shapes, Unicode UD suffixes |
| Characters and strings / literal decoder + cursor | PP spelling → decoded elements → selected code-unit width; maximal sequence checks encoding/suffix before emission; O(input + output) | character/Unicode/raw/numeric escapes/concat/UD fixtures and boundary properties |
| CLI, telemetry and evidence / tool + personal harness | streaming adapter, observable work/capacity counters; no generated executable at PA2 | full stage + through report, explicit personal checks, latency/RSS workloads, file audit |

Later semantic graphs, demand, IR and executable optimization remain later-stage
work. [Completion audit](audit.md) traces current ownership/spec requirements.
All groups above are complete; no PA2 behavior or incomplete handoff remains.

## Performance and final checks

[Evidence](../student.tests/pa2/performance.md) retains 168 timed observations
from two frozen seven-workload campaigns, A/A calibration and ABBA comparisons.
The final CPU-0 run measures 0.797030/3.130805 s and 8024/20312 KiB peak RSS for
4/16 MiB repeated source. Fourfold latency grows 3.928x (ordinary) and 4.031x
(telemetry), below the 6x budget. All work, scratch, identifier and RSS bounds
pass, including long raw strings, late-encoding concatenation and 200k suffixes.
Timing noise precludes a speedup claim. Generated runtime/text size is N/A;
PA2 emits tokens. Host-tool text and every measured sample are in the report.

Final validation: `make test-pa2` 26/26; exact prior-through command 54/54;
`make test-report-through-pa2` 80/80; required file audit 35 files and additional
entry-point audit 36 files; `git diff --check` passes. Course fixture trees are
unchanged from the stage base. Personal checks and complete course suites also
pass with ASan/UBSan. Final binary/build-source/input hashes match the evidence.

## Handoff ledger

- `85d89e23e`: read instructions/spec/handout and PA1 ownership; verified clean HEAD
  and authoritative log. Entry checkpoint: no PA2 progress (stub, 26 failures);
  revalidation established implementation as the next safe action.
- `e08732384`: all three language groups complete; first required run
  passes 26/26 and through-PA2 passes 80/80. File audit passes 35 files. The
  empty-character recovery policy is PA2-only; literal-operator splitting uses
  grammatical context and records physical suffix locations. Numeric escapes
  retain code-unit identity until final encoding. No course fixtures changed.
- `ceaa2dd1b`: 316 personal cases, including 7,062 independent integer
  values/types, encoding triples, long literals, physical locations and zero
  allocation calls in the warmed integer/character/string path. PA1's 64 personal
  cases, both API checks and all 80 course cases pass ASan/UBSan. Fixed counting
  of the second decoded element in invalid characters; token output unchanged.
- Final consolidation: completed benchmark evidence, current plan and completion
  audit; all required checks pass. Handoff reason: PA2 full-stage is complete,
  including related boundary/resource groups. Review markers remain unchanged.
