# PA10 personal evidence

Run explicitly; required course discovery/comparison is unchanged:

```sh
python3 student.tests/pa10/check.py
python3 student.tests/pa10/check_multifile.py
python3 student.tests/pa10/check_ir.py
python3 student.tests/pa10/check_reference_corrections.py
python3 student.tests/pa10/build_sanitizer.py /tmp/pa10-sanitize
UBSAN_OPTIONS=halt_on_error=1 ASAN_OPTIONS=detect_leaks=1 python3 student.tests/pa10/check.py /tmp/pa10-sanitize/cppgm-sanitize
UBSAN_OPTIONS=halt_on_error=1 ASAN_OPTIONS=detect_leaks=1 python3 student.tests/pa10/check_multifile.py /tmp/pa10-sanitize/cppgm-sanitize
UBSAN_OPTIONS=halt_on_error=1 ASAN_OPTIONS=detect_leaks=1 python3 student.tests/pa10/check_ir.py /tmp/pa10-sanitize/cppgm-sanitize
python3 student.tests/pa10/benchmark.py verify student.tests/pa10/independent-performance.json
```

`check.py` runs fourteen independent native programs and nine semantic
rejections, covering references/defaults/control flow, narrow/bool increments,
discarded volatile accesses, static addresses/strings, aggregate padding,
nested calls, reference returns, compound-assignment sequencing, nested control
entries, condition-initialization barriers, volatile readback/discard rules,
static conversions, negative pointer differences, C linkage and symbol collisions.
`check_multifile.py` executes three source TUs in both orders, checking typed
external linkage, internal isolation, reference temporaries and inline deduplication.
All outputs are constructed by our compiler;
only explicit execution checks invoke the allowed reference native backend.
`check_ir.py` audits the in-memory Program on 123 successful course/control
inputs. The two reference-order reducers observe a documented pinned-reference
bug; see [proof and bundle provenance](../../pa10/reference-corrections.md).

[Protocol](performance-protocol.md) was recorded before measurement. The
[report](../../pa10/performance.md) gives all four performance dimensions,
work/caching evidence, calibration, paired spread and stage-scoped limits.
Frozen A is `17f3deb7`/`55b33a44`; audited B implementation is `3af0da70`, and
final B is `63592ef0`. Rebuild in
isolated checkouts with `make -C dev cppgm++`, then copy to the recorded paths.
The timing sequence, kept separate from any builds/tests, was:

```sh
python3 student.tests/pa10/benchmark.py measure /tmp/pa10-first-correct /tmp/pa10-audited student.tests/pa10/performance.json /tmp/pa10-performance
python3 student.tests/pa10/benchmark.py continue /tmp/pa10-first-correct /tmp/pa10-audited student.tests/pa10/short-runtime-performance.json /tmp/pa10-performance student.tests/pa10/performance.json
python3 student.tests/pa10/benchmark.py continue /tmp/pa10-first-correct /tmp/pa10-audited student.tests/pa10/audited-performance.json /tmp/pa10-performance student.tests/pa10/short-runtime-performance.json
python3 student.tests/pa10/benchmark.py final /tmp/pa10-first-correct /tmp/pa10-final student.tests/pa10/final-performance.json /tmp/pa10-final-performance
python3 student.tests/pa10/benchmark.py final /tmp/pa10-first-correct /tmp/pa10-independent-final student.tests/pa10/independent-performance.json /tmp/pa10-independent-performance
```

The first historical run completed compiler timings but its original size
adapter stopped on sectionless ELF; its JSON is preserved. The current adapter
can finish that command. To reproduce the historical three-record split, use
the respective protocol commits, or choose fresh result filenames for a new
campaign. Do not overwrite historical observations. Verification requires the
recorded `/tmp` artifacts; after cleanup, rebuild/rerun into new records rather
than claiming old binary hashes were reverified. No binaries, objects or logs
are committed.

The independent audit's retained final compiler is `60f7088f`, frozen at
`/tmp/pa10-independent-final`. It uses the same protocol, flags, nine compiler
workloads and three long native workloads as the previous final campaign.
The final sanitizer build was `/tmp/pa10-independent-sanitize`; the generic
commands above rebuild the current sources into the chosen directory.
See the reconstructed [architecture and findings](../../pa10/audit.md).

The standalone `lowering-trace.cpp` exercises a typed namespace call with a
reference/default argument, static relocation and negative pointer differences.
Compile it with `--emit-lowir --validate-lowir -O0`, execute through the supplied
native backend, and inspect native bytes as described in the audit. The fixed
`linkage-growth.py` script records 4/16-TU declaration deduplication and existing
work counters; `linkage-growth.json` is diagnostic evidence, not a speed gate.

The final delta uses `benchmark.py delta /tmp/pa10-independent-final
/tmp/pa10-consolidated-final student.tests/pa10/consolidated-performance.json
/tmp/pa10-consolidated-performance 60f7088f`. A is retained; B (`4fc61de6`)
is rejected and reverted. Both records and frozen binaries remain verifiable;
see [final assessment](../../pa10/final-audit-performance.md). The source hash in
`linkage-growth.json` identifies the subsequently reverted candidate; its code
for linkage is identical to the retained compiler and those structural counters
remain applicable. Its phase times are diagnostic only.
