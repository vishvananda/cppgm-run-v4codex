# PA9 personal checks and independent audit

Run from the repository root. These checks neither change nor replace the
course fixtures, references or harness.

```sh
make -C dev abimangle
python3 student.tests/pa9/build_checks.py
python3 student.tests/pa9/check.py
python3 student.tests/pa9/check_final.py
python3 student.tests/pa9/build_checks.py --sanitize --output /tmp/pa9-evidence/check-api-sanitized
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 python3 student.tests/pa9/check.py --api /tmp/pa9-evidence/check-api-sanitized
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 python3 student.tests/pa9/check_final.py --api /tmp/pa9-evidence/check-api-sanitized
```

`check.py` runs direct API assertions, all 117 checked-in status/output cases
and successful serialization roundtrips, 11 original valid/15 invalid probes,
and file/case ordering. The six root fixtures supplement the 111 discovered
by the default course harness. `check_final.py` adds eight exact/roundtrip
probes and eleven controlled rejections; rejects must exit 1, never signal or
exceed the 15-second timeout. Direct API checks retain 100k unrelated facts and
20k iterative modifiers and additionally cover canonical literals, qualified
function substitution, function-template/tag identities, invalid graph links,
context depth, external depth and complete typed roundtrips. Sanitizer builds
use ASan/UBSan, leak detection and non-PIE linking for stable shadow mapping.

The serializer is an explicit inspection adapter. Added forms preserve typed
function qualifiers (`function-type-qualified <bits> <variadic> ...`), an inline
function's `context <binder>`, and a virtual thunk's `this-adjust <offset>`.
Production callers construct graph facts and targets directly. No fact text,
Itanium fragments or host object inspection supplies production decisions.

## Frozen performance reproduction

The [frozen protocol](final-audit-protocol.md) predates timing. Two experiments
compare the final implementation with the completed checkpoint and first
correct PA9 encoder. Each uses the same eight inputs, AAAA noise calibration,
two ABBA blocks, checked output hashes and all unchanged budgets. Use fresh
artifact directories: the harness refuses to overwrite existing observations.

```sh
mkdir -p /tmp/pa9-repro/checkpoint /tmp/pa9-repro/first
git archive 8195487d1 dev | tar -x -C /tmp/pa9-repro/checkpoint
git archive 2b19ba07a dev | tar -x -C /tmp/pa9-repro/first
make -C /tmp/pa9-repro/checkpoint/dev abimangle
make -C /tmp/pa9-repro/first/dev abimangle
make -C dev abimangle
python3 student.tests/pa9/benchmark.py --baseline /tmp/pa9-repro/checkpoint/dev/abimangle --baseline-commit 8195487d1 --directory /tmp/pa9-repro/delta --report /tmp/pa9-repro/delta.json
python3 student.tests/pa9/benchmark.py --baseline /tmp/pa9-repro/first/dev/abimangle --baseline-commit 2b19ba07a --directory /tmp/pa9-repro/stage --report /tmp/pa9-repro/stage.json
python3 student.tests/pa9/verify_performance.py --report /tmp/pa9-repro/delta.json
python3 student.tests/pa9/verify_performance.py --report /tmp/pa9-repro/stage.json
```

Commit implementation changes before timing. Finish all builds/tests first;
the benchmark pins itself to one allowed CPU. Compiler flags, entry/runner
configuration, source tree IDs, binary/input/output hashes, all timed output
hashes, wall/RSS samples, phase/work counters and budgets are retained.
`size` measures the compiler executable itself, never an ABI-name oracle.
Generated executable runtime/text are N/A because PA9 emits only ABI names.

Current evidence is `final-audit-performance.json` and
`final-stage-performance.json` (96 observations each). The verifier defaults
to the former and confirms current sources/binary, frozen artifacts, raw order,
paired summaries, spread, output equivalence and all 42 budget checks.
`pre-prefix-*-performance.json` retains both successful preliminary experiments
on `f2353211e`, superseded after the final malformed-prefix fix. Verify those
with `--historical --report <path>`; that mode does not claim the older binary
is current. Original checkpoint `performance.json` and
`initial-performance.json` also remain, including the initial text-growth
failure. No observations are deleted or pooled to improve a result.
