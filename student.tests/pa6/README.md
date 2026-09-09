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

`check.py` covers 57 independent declaration-point, type, lookup, constant and
scope interactions. `check_api.cpp` checks canonical declaration/signature and
array identities, source parameter distinctions, alias reference collapse,
completed signature reuse, alias and nested function-pointer identity, definition/body scope links, and source-graph lifetime
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
`0dc05603e6ba9f136d9f2d0ac470c169c43aaceb`, first working PA6 `00cb86a9f`, and
the final implementation revision `b3f468fdf` recorded in `final-audit-performance.json`. Rebuild each
in its own temporary checkout with `make -C dev cppgm++` (g++ GNU++11, -O3,
course test runner enabled). Do not run builds/tests during timing.

```sh
python3 student.tests/pa6/measure.py measure /tmp/pa5-base /tmp/pa6-first /tmp/pa6-final student.tests/pa6/final-audit-performance.json
python3 student.tests/pa6/measure.py verify /tmp/pa5-base /tmp/pa6-first /tmp/pa6-final student.tests/pa6/final-audit-performance.json
python3 student.tests/pa6/measure.py report student.tests/pa6/final-audit-performance.json
```

The record retains binary/input/output hashes, every observation, phase/work
counters, startup probes, host flags/version and explicit budgets. The verifier
checks protocol completeness, output equivalence, startup separation, paired
latency/RSS limits and fourfold scaling. `superseded-performance.json` retains
an interrupted earlier campaign: a declaration-point correctness finding caused
an explicit stop, so those observations support no final-binary claim.
`pre-compact-performance.json` retains the complete earlier campaign that
failed the largest template RSS budget. Its binary is identified by source
revision and hash; final claims use the new complete campaign.
Generated executable runtime/text and self-hosting are N/A for PA6.

The independent audit also exercises qualified class/enum owners across parser
and semantics, constructor function-try bodies, control-statement block scope,
inline namespace direct-hit rules, and layout overflow. Its separate edge
corpus uses 6,000/24,000 namespaces with two qualified using directives per
namespace. Inputs remain separate from the original twenty frozen workloads.
The same startup, ABBA/A/A, latency, memory and scaling budgets apply:

```sh
python3 student.tests/pa6/audit_performance.py measure /tmp/pa6-checkpoint /tmp/pa6-final student.tests/pa6/final-edge-performance.json
python3 student.tests/pa6/audit_performance.py verify /tmp/pa6-checkpoint /tmp/pa6-final student.tests/pa6/final-edge-performance.json
python3 student.tests/pa6/audit_performance.py report student.tests/pa6/final-edge-performance.json
```

The edge A binary is `08e24f046`, B is `b3f468fdf`. `semantic_edges` counts
unique pair identities; final `semantic_lookup_work` must equal twice the
namespace count, proving that duplicate insertion and qualified direct lookup
do not scan the growing ordinary edge list. Latency, not work counts alone,
establishes the performance result. `performance.json` remains the historical
checkpoint campaign and is not used as evidence for the corrected final binary.
