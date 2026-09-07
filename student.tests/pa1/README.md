# PA1 personal validation

Run from the repository root:

```sh
make test-pa1
python3 student.tests/pa1/check.py
python3 student.tests/pa1/check.py --sanitize
python3 student.tests/pa1/benchmark.py
make test-report-through-pa1
perl scripts/cppgm_file_audit.pl --stage pa1 --paths dev/src
```

`check.py` explicitly runs 59 boundary/property cases, including a seeded
3,000-token composition, invalid UTF-8, translation ordering, escaped UCNs,
raw delimiters and directive context. It compiles `cursor.cpp` to check borrowed
source ranges, physical locations, canonical IDs across table growth and source
lifetimes, and demand-driven tokenization. `--sanitize` compiles both the cursor
check and PA1 entry point with AddressSanitizer and UndefinedBehaviorSanitizer.
Generated test binaries stay in `obj/student-pa1/`.

`benchmark.py` freezes the current binary, flags, source hashes and five input
files under `obj/student-pa1/performance-TIMESTAMP/`. A is ordinary tokenization;
B is the identical binary with `--stats`. Before timing, complete output hashes
must agree. Each workload gets two A/A calibration pairs and two ABBA blocks.
Wall time includes process startup, source reading, scanning and required token
formatting to `/dev/null`; `/usr/bin/time` records executable-process peak RSS.
The verification runs that hash output through a pipe are untimed warmups;
their phase timings/RSS are not the latency or RSS comparison.

The script retains every timed observation, manifest, frozen input and binary.
The committed [performance report](performance.md) records the completed run.
This is a compiler baseline and telemetry-overhead measurement, with explicit
linear-work/memory envelopes; it makes no speedup claim against the incomplete
starter. All generated-program runtime and text-size entries are N/A because
PA1 emits preprocessing tokens. The template/loop/call/memory/floating-point
and self-source inputs exercise only lexical work at this stage; later stages
must add semantic compilation and executable benchmarks.
