# PA9 personal checks

Run from the repository root; none of these tests changes or replaces the
course harness or reference fixtures.

```sh
make -C dev abimangle
mkdir -p /tmp/pa9-evidence
python3 student.tests/pa9/build_checks.py
python3 student.tests/pa9/check.py
python3 student.tests/pa9/build_checks.py --sanitize --output /tmp/pa9-evidence/check-api-sanitized
python3 student.tests/pa9/check.py --api /tmp/pa9-evidence/check-api-sanitized
```

The direct C++ API checks graph identity, substitution reset, explicit operator
shape, enclosing function context, virtual-result thunks, 100,000 unrelated
names and 20,000-level modifier chains. The Python driver independently checks
all 117 checked-in fixtures by exit status and exact successful output, and
roundtrips each successful fixture through the fact serializer. It also runs
11 valid and 15 invalid probes, including tagged templates, unsigned modulo
identity, 64-bit parameter indices, deep serialization, and multiple-file case
ordering. Sanitizer builds use ASan/UBSan with leak detection and non-PIE linking
for stable shadow mapping; they use the same source implementation.

Performance reproduction (frozen baseline is the first correct PA9 encoder):

```sh
mkdir -p /tmp/pa9-evidence/base-tree
git archive df7dbb00a dev | tar -x -C /tmp/pa9-evidence/base-tree
make -C /tmp/pa9-evidence/base-tree/dev abimangle
python3 student.tests/pa9/benchmark.py --baseline /tmp/pa9-evidence/base-tree/dev/abimangle --directory /tmp/pa9-evidence/final-performance
python3 student.tests/pa9/verify_performance.py
```

`performance.json` retains final AAAA/ABBA/ABBA observations, compiler flags,
binary/input/output hashes, latency, peak RSS, compiler text size, phase/work
counters, budgets and all pass/fail calculations. The initial smaller-workload
run remains in `initial-performance.json`, including its failed text-growth
budget. No observation was discarded to improve a reported result. Final timing
runs start after builds and correctness checks have completed. The compiler
executable's text size is inspected only for growth accounting, never to obtain
ABI-name answers. PA9 emits no executable, so generated runtime/text is N/A.

The serializer is an inspection API. Its canonical inline forms can retain
structured owners (`*-type`, `template-name`, `local-owner`) and complete
function shape (`param`, `argument`, `result`, `terminal`, `qualifier`,
`member-shape`). These are typed adapter records, not raw Itanium fragments.
Production uses `Graph` and `Target` directly, without this text boundary.
