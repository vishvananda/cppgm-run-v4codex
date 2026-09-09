# PA1 consolidated plan

Stage base commit: `f6a53056a0dac62ac05aa3b3684e362edb65c071`
Audit input: `79b74e688d67720ab07f561836836c71926d7e32`
Last reviewed commit: `762ae8f247a5e76879a03ccd6893c1a0a6e66c16`
Target: **PA1 full-stage final audit**; PA2 has not been started.

## Final Spec Alignment

- Immutable `SourceBuffer` → 18-slot `CharacterCursor` → typed
  `PPTokenCursor::next()` → explicit PA1 output view. UTF-8, UCNs, trigraphs,
  splices, raw reversion and EOF rules preserve physical ranges/locations.
- Tokens borrow contiguous source bytes or one reusable transformed-token buffer.
  TU-owned flat identifier storage publishes stable IDs for names and suffixes.
  Only new names can trigger geometric table growth; completed hits allocate
  nothing. No token graphs, interphase serialization or mutable global cache.
- Source, table and cursor have explicit release boundaries. Work is expected
  linear in bytes/names; punctuation/raw lookahead is bounded. Capacity/probe
  counters make growth visible. Resource envelopes include unique-name metadata.
- The independent [audit](audit.md) maps every relevant spec section, traces a
  declaration/template spelling, include context and raw/suffix transitions,
  and reviews legality, profitability, invalidation and pipeline budgets.
  Parsing, semantic demand, typed IR, native optimization and ELF are later PAs.

## Findings and changes

The audit reproduced unnecessary table doubling on an existing-name hit at half
occupancy. Growth now follows the miss check; identity and new-name insertion
remain valid. Rehash work and source/scratch capacities are exposed in telemetry.
Personal validation now includes 64 cases, large resource boundaries and a
completed-hit storage regression. Benchmarks add dense-name scaling, long tokens,
raw near-matches and table-boundary hits, with frozen A/B inputs and timeouts.

## Performance and validation

Two completed A/B runs use identical frozen before/after binaries and ten fixed
inputs, with A/A calibration and two ABBA blocks each; the second run pins CPU 0.
All outputs agree. Table-boundary retained storage falls by 2 MiB and peak RSS
by about 4 MiB in both runs. Timing noise precludes a speedup claim. Repeated
8→32 MiB input takes 3.993–3.996x final-binary latency (limit 6x). The final
telemetry comparison also passes; ordinary CLI latency is 1.514611/6.015027 s
and peak RSS 12092/36672 KiB at those sizes. Dense-name fourfold latency grows
4.187x/4.326x (limit 6x). [Performance evidence](../student.tests/pa1/performance.md)
retains all 360 observations, calibration, spreads, output hashes and explicit
regression/noise disclosures. Host-tool text grows 488 bytes (0.85%); no speed
gain is claimed. Generated runtime/text size is N/A: PA1 produces tokens.
No executable optimization or node-count benefit is claimed.

Final validation: `make test-pa1` and `make test-report-through-pa1` pass 54/54
(1/1 stages); there are no earlier assignments. The full course suite also passes
54/54 using the standalone ASan/UBSan binary without the batch wrapper.
`check.py` and `check.py --sanitize` pass all 64 personal cases and the
identity/location/streaming/allocation checks. The required file audit passes
24 files, the additional entry-point audit passes, and `git diff --check` passes.
All benchmark artifacts/hashes, 360 observations and both variants' resource
gates were independently verified. No PA1 behavior group or handoff remains.
The implementation fix is committed below; the accompanying consolidation
commit contains the reviewed audit, plan, benchmark controls and evidence.

## Handoff ledger

- `5b69ae0d5`: plan and 0/54 baseline; independently reread.
- `e5c65b70c`: shared PA1 implementation; all sources and tool registration reviewed.
- `a3abcf619`: EOF/BOM/escaped-UCN closure; changes and properties revalidated.
- `79b74e688`: checkpoint/performance evidence; frozen binary/hash/protocol reviewed.
- `762ae8f24`: table ownership fix, telemetry and expanded resource checks; reviewed
  with final course, sanitizer and frozen benchmark evidence.
- Final consolidation: audited plan, benchmark controls and complete performance
  evidence. No compiler changes follow the reviewed implementation commit.
  [Detailed dispositions](audit.md#handoff-ledger)
  cover every handoff since the original pre-implementation review marker.
