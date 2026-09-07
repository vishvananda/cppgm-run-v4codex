# PA2 implementation plan

Stage base commit: `be4bff26fcc81762196d29d4f0b0c59a20110c9d`
Last reviewed commit: `93c5baabfdb4c2591b40188d2eba9e467a1a0b57`
Last implementation change: `ceaa2dd1b7eafbe5f5860e122f6bec6875c30c1f`
Target: **PA2 full-stage**. Baseline: 0/26; current: 26/26, through-PA2: 80/80.

## Final Spec Alignment and validation groups

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

Spec §§1–2/5/8–10 apply to this streaming path, canonical names/types, explicit
scratch ownership, bounded work and self-contained implementation. Later
semantic graphs, demand, IR and executable optimization (§§3–4/6–7) remain
later-stage work. The [independent final audit](audit.md) traces representative
data, legality, view invalidation and release boundaries across the actual
source. All groups are complete; no PA2 behavior or unaudited handoff remains.

## Performance and final checks

[Evidence](../student.tests/pa2/performance.md) retains 252 timed observations
from three frozen seven-workload campaigns, A/A calibration and ABBA comparisons.
The independent CPU-0 audit run measures 0.758290/3.025535 s and 8024/20312 KiB
peak RSS for 4/16 MiB repeated source. Fourfold latency grows 3.990x (ordinary)
and 3.948x (telemetry), below the 6x budget. All work, scratch, identifier and RSS bounds
pass, including long raw strings, late-encoding concatenation and 200k suffixes.
Timing noise precludes a speedup claim or additional optimization work/growth
budget. Generated runtime/text size is N/A; PA2 emits tokens. Host-tool text
stays 83,290 bytes. Every measured sample and positive cost is in the report.

Fresh final validation: `make test-pa2` 26/26; `make test-report-through-pa2`
80/80; required file audit 35 files and additional entry-point audit;
`git diff --check` passes. Course fixtures/references are unchanged from the
stage base. PA2's 316 personal cases (7,062 integer values/types), PA1's 64
personal cases, both API checks and all 80 course cases pass ASan/UBSan.
Final binary/build-source/input hashes match the evidence. No further compiler
defect was found; this audit clarifies the course literal-operator adaptation
and replaces the stale review marker without changing compiler behavior.

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
- `93c5baabf`: completion record and two frozen performance campaigns. Final
  audit independently recomputed all 168 historical observations and reviewed
  every handoff from the initial marker; its source/build/input hashes agree.
- Independent final audit: reconstructed whole-stage source ownership and data
  flow, reran correctness/sanitizers and all exit checks, and added 84 frozen
  observations. No further compiler change was justified. The accompanying
  records are the final audit consolidation; no PA2 item is deferred or advanced
  into PA3. Generated outputs stay ignored; intended changes are committed.
