# Personal PA6 validation

These inputs and harnesses are explicit checks, not additions to course discovery.
From the repository root:

```sh
python3 student.tests/pa6/check.py
python3 student.tests/pa6/check_course.py dev/cppgm++
python3 student.tests/pa6/build_checks.py /tmp/pa6-sanitizer --sanitize
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 /tmp/pa6-sanitizer/api
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 /tmp/pa6-sanitizer/pa5-api
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 python3 student.tests/pa6/check.py /tmp/pa6-sanitizer/compiler
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 python3 student.tests/pa6/check_course.py /tmp/pa6-sanitizer/compiler
```

`check.py` covers 33 independent declaration-point, type, lookup, constant and
scope interactions. `check_api.cpp` checks canonical declaration/signature and
array identities, source parameter distinctions, alias reference collapse,
completed signature reuse, definition/body scope links, and source-graph lifetime
under arena growth. `check_course.py` reruns the unchanged 105 PA6 fixtures
against a standalone binary, including `.t2` primary translation units and
required rejections. It does not write course sidecars.

`bench_inputs.py` reuses the fixed PA5 corpus (declarations, expressions,
templates, nesting, classes and procedural loops/calls/memory/floating point)
and adds PA6 constants, template parameter environments, namespace graphs and
repeated compound function signatures. Every workload has 1x/4x variants.
Ordinary observations use fixed source repetitions, CPU affinity, A/A noise
pairs and two ABBA blocks. Separate observations measure telemetry overhead.

Freeze three optimized compiler binaries before timing: the stage base
`9249196518f45492822fb2e3da4eb5d82af0ed13`, first working PA6 `5749f43b4`, and
the final implementation revision recorded in `performance.json`. Rebuild each
in its own temporary checkout with `make -C dev cppgm++` (g++ GNU++11, -O3,
course test runner enabled). Do not run builds/tests during timing.

```sh
python3 student.tests/pa6/measure.py measure /tmp/pa5-base /tmp/pa6-first /tmp/pa6-final student.tests/pa6/performance.json
python3 student.tests/pa6/measure.py verify /tmp/pa5-base /tmp/pa6-first /tmp/pa6-final student.tests/pa6/performance.json
python3 student.tests/pa6/measure.py report student.tests/pa6/performance.json
```

The record retains binary/input/output hashes, every observation, phase/work
counters, startup probes, host flags/version and explicit budgets. The verifier
checks protocol completeness, output equivalence, startup separation, paired
latency/RSS limits and fourfold scaling. `superseded-performance.json` retains
an interrupted earlier campaign: a declaration-point correctness finding caused
an explicit stop, so those observations support no final-binary claim.
Generated executable runtime/text and self-hosting are N/A for PA6.
