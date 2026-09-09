# PA4 personal validation and performance

Run explicitly from the repository root:

```sh
python3 student.tests/pa4/check.py
make test-pa4
make test-report-through-pa4
perl scripts/cppgm_file_audit.pl --stage pa4 --paths dev/src
```

The 168 personal invocations check macro state, stringizing/pasting, variadics,
course recursion and rescan boundaries, inactive groups, lazy expression errors,
line/include state, hard-link once identity, primary-source reset, random macro
DAGs, a 16,000-macro chain with 64 shared argument uses, and 2,000 nested arguments.
Reuse cases vary arity, empty/variadic arguments,
raw/expanded uses, counters and function-name lookahead within one expander.
Expected expansions are independently specified and converted by the already
validated PA2 tool; raw trigraph stringizing has an explicit decoded-byte oracle.
Course fixtures and references remain unchanged.

`api.cpp` consumes the structured cursor directly. It traces all tokens of a
macro-generated template declaration,
including condition selection, a pasted name's canonical identity, presumed
filename, physical location and decoded line value. It also checks literal
operator suffix splitting, three 20,000-deep invocations capturing 180,000 raw
tokens in 625 task slabs, and <=128 KiB spelling storage for 100,000 counters.
Allocator interception proves the third deep invocation allocates nothing after
two warm-ups; explicit counters additionally check depth-local buffer reuse.
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

`benchmark.py BASELINE CANDIDATE REPORT.md --candidate-commit COMMIT` uses frozen binaries and eight fixed
inputs: 4/16 MiB plain text, C++ template/loop/call/memory/floating spellings,
repeated argument use, nested arguments, long helper chains, counters and
literal-operator locations.
It checks equivalent output on every observation, records input/output/binary
hashes, uses two A/A pairs, one B/B pair and two ABBA blocks, and measures one
separate telemetry observation. Eight empty-input probes verify workload/startup
separation. All final compiler
wall-time and peak-RSS samples are preserved in
[final-audit-performance.md](final-audit-performance.md). Host-tool text size is also
recorded. `verify_performance.py REPORT.md [CANDIDATE]` recomputes all budgets
from the saved observations and optionally checks the candidate binary hash.
PA4 produces tokens; generated-program runtime and text size do not
yet apply. These inputs exercise frontend work, not semantic template
instantiation, native code, or self-hosting.

The budgets were recorded in `pa4/plan.md` before the campaign. Reproduce using
an isolated checkout of `77bd7bc51` for A and `5c200a4df` for final B, with identical
ordinary flags and course-runner settings. Keep binaries outside the repository;
no generated objects, diagnostic logs, or course `.my*` outputs are committed.
The [initial campaign](performance-initial.md) preserves the earlier `95dc4d4b6`
candidate, including the helper-chain regression that motivated the final fix.
The [parameterless campaign](performance-parameterless.md) records `965f7ba6e`
before the final literal-operator location correction. Both remain as evidence.

The independent final audit froze A at `695e607c3` and B at `e0b7bf8ab`. Reproduce
with isolated ordinary builds and frozen copies outside the repository:

```sh
python3 student.tests/pa4/benchmark.py /tmp/preproc-A /tmp/preproc-B /tmp/final-pa4.md --baseline-commit 695e607c3 --candidate-commit e0b7bf8ab --candidate-description 'pooled prescan and capture storage'
python3 student.tests/pa4/verify_performance.py student.tests/pa4/final-audit-performance.md dev/preproc
```

[performance-pooled.md](performance-pooled.md) preserves the intermediate
`6c1867c8b` comparison. Both final-audit campaigns keep every observation,
including regressions and noisy groups. See [the audit](../../pa4/audit.md) for
allocation ownership, validity and work/growth budgets. The older campaigns
above remain historical evidence; their candidate hashes refer to their own
commits rather than the latest binary.
