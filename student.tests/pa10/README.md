# PA10 personal evidence

Run explicitly; required course discovery/comparison is unchanged:

```sh
python3 student.tests/pa10/check.py
python3 student.tests/pa10/check_ir.py
python3 student.tests/pa10/check_reference_corrections.py
python3 student.tests/pa10/build_sanitizer.py /tmp/pa10-sanitize
UBSAN_OPTIONS=halt_on_error=1 ASAN_OPTIONS=detect_leaks=1 python3 student.tests/pa10/check.py /tmp/pa10-sanitize/cppgm-sanitize
UBSAN_OPTIONS=halt_on_error=1 ASAN_OPTIONS=detect_leaks=1 python3 student.tests/pa10/check_ir.py /tmp/pa10-sanitize/cppgm-sanitize
python3 student.tests/pa10/benchmark.py verify student.tests/pa10/final-performance.json
```

`check.py` runs eight independent native programs and seven semantic
rejections, covering references/defaults/control flow, narrow/bool increments,
discarded volatile accesses, static addresses/strings, aggregate padding,
nested calls, reference returns and compound-assignment sequencing. All outputs are constructed by our compiler;
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
```

The first historical run completed compiler timings but its original size
adapter stopped on sectionless ELF; its JSON is preserved. The current adapter
can finish that command. To reproduce the historical three-record split, use
the respective protocol commits, or choose fresh result filenames for a new
campaign. Do not overwrite historical observations. Verification requires the
recorded `/tmp` artifacts; after cleanup, rebuild/rerun into new records rather
than claiming old binary hashes were reverified. No binaries, objects or logs
are committed.
