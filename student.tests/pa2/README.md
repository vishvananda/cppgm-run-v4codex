# PA2 personal validation

Run explicitly from the repository root:

```sh
make test-pa2
python3 student.tests/pa2/check.py
python3 student.tests/pa2/check.py --sanitize
python3 student.tests/pa1/check.py --sanitize
taskset -c 0 python3 student.tests/pa2/benchmark.py
make test-report-through-pa2
perl scripts/cppgm_file_audit.pl --stage pa2 --paths dev/src
```

`check.py` runs 316 cases including 7,062 independently computed integer
type/value combinations, every encoding-prefix triple, scalar/code-unit
boundaries, long numeric escapes, overflow, raw restoration, Unicode suffixes,
literal-operator context, empty-character recovery and phase-1–3 rejection.
`cursor.cpp` verifies borrowed source views, physical suffix locations after UCN
translation, canonical identities across growth, lazy failure, transformed
lookahead lifetimes, production operation without joined spellings, and zero
allocation calls in a warmed stream of integers, characters and string sequences.
The sanitizer option builds standalone tools and the API check with ASan/UBSan;
outputs stay under `obj/student-pa2/`. The same standalone tools can run the
complete course suite with `CPPGM_BATCH_TESTS=0 make -C pa2 test
CPPGM_SKIP_DEV_REBUILD=1 CPPGM_TEST_APP=/absolute/path/to/posttoken-sanitized`.

`benchmark.py` freezes binary, build flags, source/harness hashes and seven inputs.
It checks full output hashes for ordinary execution (A) and `--stats` (B), then
records two A/A calibration pairs and two ABBA blocks per workload. Every sample
records wall time and compiler peak RSS; optional telemetry records read and
fused scan/conversion/output time, bytes, elements, parts and storage growths.
Hashing/warmup runs are untimed. Measured stdout goes to `/dev/null`; every
invocation has a 60-second timeout. All observations, hashes and summaries are
retained under `obj/student-pa2/performance-*`. `--inputs DIRECTORY` reuses the
exact manifest inputs, and `--baseline PATH` supports future frozen compiler A/B
comparisons. Compare equivalent outputs before claiming a benefit.

The 4/16 MiB repeated input contains declaration/template, loop, call, memory
and floating spellings. Other inputs cover floats, implementation-source text,
200k unique numeric suffixes, a 4 MiB raw literal with delimiter near-matches,
and 466k adjacent ordinary strings with a late UTF-16 prefix. These measure
frontend work only; semantic template/executable/self-hosting benchmarks belong
to the later stages that implement those behaviors.

Budgets: fourfold source growth must take at most 6x latency; decoded source
units ≤ 2N+64, number/literal bytes and decoded elements ≤ N, encoded string
bytes ≤ 8N+4, and string scratch capacities ≤ 28N+64. N is input bytes.
Growth events must stay ≤ 256 for these fixed workloads. Scratch accounts for
geometric decoded-element storage (8 bytes/element), optional joined spellings
and final 1/2/4-byte code units. Peak RSS must satisfy both 40N+32 MiB and twice
the reported retained capacities plus 32 MiB, allowing transient geometric
growth and stream/runtime overhead. Identifier storage and rehash envelopes
are inherited from PA1. These are resource limits, not claimed optimization
benefits. [Measured evidence](performance.md) discloses calibration and spread.
PA2 emits tokens: generated-program runtime and text size are N/A. Host-tool
text size is recorded separately, with no generated-code or speedup claim.
