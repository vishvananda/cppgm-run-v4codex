# PA8 personal validation

These inputs and scripts are explicitly run; course discovery is unchanged.

```sh
python3 student.tests/pa8/check.py
python3 student.tests/pa8/check_native.py
python3 student.tests/pa8/build_checks.py /tmp/pa8-sanitize
python3 student.tests/pa8/benchmark.py verify student.tests/pa8/performance.json
```

`check.py` covers 24 valid and 52 invalid cases, writer fixed points, helper-only
units, multifile identity, forward references, parallel phi inputs, all scalar
conversions, metadata placement, literals, object boundaries and CLI failures.
Failures must exit 1; a crash is never an accepted rejection. It specifically
checks signalling NaN and large positive integer literal preservation, beyond
the course fixtures. `check_api.cpp` checks compact types, IDs surviving pool
and interner growth, independent units, construction without text, model edits
observed by the writer, and rejection of invalid local instruction shape.

`build_checks.py` builds the tool and API with ASan/UBSan (including default
leak detection) outside the repository. `check_native.py` routes our generated
LowIR through the supplied backend, covers every allowed sum input, repeatedly
exercises aliased swaps and dynamic callbacks, and checks floating memory work.
The compiler implementation itself never invokes this backend.

`bench_inputs.py` fixes three compiler workload families at 6,000/24,000
functions and four native workloads with about 20 million iterations. Inputs
to helper calls come from volatile loop state; accumulator and exit checks
retain the executable work. There is no optimizer or speedup claim.

Reproduce the measurements with frozen builds of `66167cf72` and `01d39a2f6`
using the recorded GNU C++11/O3 flags, and put their binaries at the recorded
paths (or pass new paths when measuring):

```sh
python3 student.tests/pa8/benchmark.py measure \
  /tmp/pa8-evidence/lowir-initial /tmp/pa8-evidence/lowir-final \
  student.tests/pa8/performance.json
```

The JSON retains every raw observation and hash. `/tmp` holds generated inputs,
outputs, binaries and objects, which are deliberately not committed. The
verification command requires those artifacts; after cleanup, reproduce the
measurement to regenerate them. [Performance analysis](../../pa8/performance.md)
discloses regression, noise, native text accounting and the evidence limits.
