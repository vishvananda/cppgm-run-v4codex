# PA4 personal validation and performance

Run explicitly from the repository root:

```sh
python3 student.tests/pa4/check.py
make test-pa4
make test-report-through-pa4
perl scripts/cppgm_file_audit.pl --stage pa4 --paths dev/src
```

The 163 personal invocations check macro state, stringizing/pasting, variadics,
course recursion and rescan boundaries, inactive groups, lazy expression errors,
line/include state, hard-link once identity, primary-source reset, random macro
DAGs, a 16,000-macro chain with 64 shared argument uses, and 2,000 nested arguments.
Expected expansions are independently specified and converted by the already
validated PA2 tool; raw trigraph stringizing has an explicit decoded-byte oracle.
Course fixtures and references remain unchanged.

`api.cpp` consumes the structured cursor directly. It asserts presumed filename
and physical offset identity, canonical identifiers, exactly 60,000 captured
raw tokens for 20,000 nested invocations, 19,999 borrowed nested argument ranges,
20,000 prescans, and <=128 KiB spelling storage for 100,000 counter expansions.
The following builds both standalone sanitizer executables without changing the
ordinary compiler or its object directory:

```sh
python3 - <<'PY'
from pathlib import Path
import subprocess
line = next(s for s in Path('dev/frontend_source_sets.mk').read_text().splitlines()
            if s.startswith('FRONTEND_OBJ_BASENAMES_preproc :='))
sources = ['dev/src/' + s + '.cpp' for s in line.split(':=')[1].split()]
flags = ['g++', '-std=c++11', '-O1', '-g', '-fno-omit-frame-pointer',
         '-fsanitize=address,undefined', '-fno-pie', '-no-pie', '-Idev/src']
for main, binary in [('dev/preproc.cpp', '/tmp/pa4-sanitize'),
                     ('student.tests/pa4/api.cpp', '/tmp/pa4-api-sanitize')]:
    subprocess.run(flags + [main] + sources + ['-o', binary], check=True)
PY
ASAN_OPTIONS=detect_leaks=1 python3 student.tests/pa4/check.py /tmp/pa4-sanitize
ASAN_OPTIONS=detect_leaks=1 /tmp/pa4-api-sanitize /tmp/pa4-api-input.cc
make -C pa4 test CPPGM_TEST_APP=/tmp/pa4-sanitize CPPGM_SKIP_DEV_REBUILD=1
```

`benchmark.py BASELINE CANDIDATE REPORT.md` uses frozen binaries and seven fixed
inputs: 4/16 MiB plain text, C++ template/loop/call/memory/floating spellings,
repeated argument use, nested arguments, long helper chains and counters.
It checks equivalent output on every observation, records input/output/binary
hashes, uses two A/A pairs, one B/B pair and two ABBA blocks, and measures one
separate telemetry observation. All compiler wall-time and peak-RSS samples
are preserved in [performance.md](performance.md). Host-tool text size is also
recorded. `verify_performance.py REPORT.md [CANDIDATE]` recomputes all budgets
from the saved observations and optionally checks the candidate binary hash.
PA4 produces tokens; generated-program runtime and text size do not
yet apply. These inputs exercise frontend work, not semantic template
instantiation, native code, or self-hosting.

The budgets were recorded in `pa4/plan.md` before the campaign. Reproduce using
an isolated checkout of `28279a9d0` for A and `957b47c37` for final B, with identical
ordinary flags and course-runner settings. Keep binaries outside the repository;
no generated objects, diagnostic logs, or course `.my*` outputs are committed.
The [initial campaign](performance-initial.md) preserves the earlier `c90cf1e62`
candidate, including the helper-chain regression that motivated the final fix.
