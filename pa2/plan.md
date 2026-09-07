# PA2 implementation plan

Stage base commit: `be4bff26fcc81762196d29d4f0b0c59a20110c9d`
Last reviewed commit: `be4bff26fcc81762196d29d4f0b0c59a20110c9d`
Target: **PA2 full-stage**. Baseline: 0/26 passing, 26 failures; PA1 passes.

## Design and remaining groups

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
work. No speedup claim against the nonfunctional baseline. Record absolute
compiler latency/peak RSS and scaling with frozen flags/binary/inputs, A/A
calibration and ABBA telemetry comparisons; generated runtime/text size N/A.
Budgets and all observations belong in `student.tests/pa2/`.

## Handoff ledger

- Entry: read instructions/spec/handout and PA1 ownership; verified clean HEAD
  and authoritative log (26 unimplemented failures). No previous implementation
  goal turn is available to classify; this entry establishes the baseline.
- Remaining: all groups above. Continue through full stage, related boundary
  behavior, required checks and committed clean state; no handoff boundary yet.
