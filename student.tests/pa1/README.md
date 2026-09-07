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

`check.py` explicitly runs 64 boundary/property cases, including a seeded
3,000-token composition, invalid UTF-8, translation ordering, escaped UCNs,
raw delimiters and directive context. It compiles `cursor.cpp` to check borrowed
source ranges, physical locations, canonical IDs across table growth and source
lifetimes, demand-driven tokenization, and allocation-free completed-name hits
at a table growth threshold. The complete template/declaration token trace,
2 MiB transformed name, raw delimiter near-matches, comments and splice runs
exercise ownership paths beyond the course fixtures. `--sanitize` compiles both
the cursor check and PA1 entry point with AddressSanitizer and UndefinedBehaviorSanitizer.
Generated test binaries stay in `obj/student-pa1/`.

`benchmark.py` freezes the current binary, flags, source/build/harness hashes and
ten input files under `obj/student-pa1/performance-TIMESTAMP/`. A is ordinary tokenization;
B is the identical binary with `--stats`. Before timing, complete output hashes
must agree. Each workload gets two A/A calibration pairs and two ABBA blocks.
Wall time includes process startup, source reading, scanning and required token
formatting to `/dev/null`; `/usr/bin/time` records executable-process peak RSS.
The verification runs that hash output through a pipe are untimed warmups;
their phase timings/RSS are not the latency or RSS comparison.

`--baseline PATH` compares a frozen earlier binary (A) with the current one (B),
both without telemetry. `--inputs DIRECTORY` reuses and verifies all inputs from
an earlier manifest, including the self-source input. For example:

```sh
taskset -c 0 python3 student.tests/pa1/benchmark.py \
  --baseline obj/student-pa1/performance-20260907-075606/pptoken-frozen \
  --inputs obj/student-pa1/audit-abba --out obj/student-pa1/audit-repeat
```

The example uses archived local artifacts; supply a separately frozen A binary
and input directory on a new checkout. CPU affinity is recorded in the manifest.
Each hashing or measured invocation has a 60-second timeout. The script retains
every timed observation, manifest, frozen input and binary.
The committed [performance report](performance.md) records the completed campaigns.
This is a compiler baseline and telemetry-overhead measurement, with explicit
linear-work/memory envelopes. The final audit additionally compares the completed
implementation before/after the table-growth fix, including capacity-boundary
hits, 200k/800k/3.2M unique names, an 8 MiB transformed name and raw delimiter
near-matches. Dense-name RSS budgets account for retained source, identifier and
scratch capacities; unique storage and fourfold latency growth have separate
bounds. See [the architecture audit](../../pa1/audit.md) for their derivation.
It makes no speedup claim against the incomplete starter.
All generated-program runtime and text-size entries are N/A because
PA1 emits preprocessing tokens. The template/loop/call/memory/floating-point
and self-source inputs exercise only lexical work at this stage; later stages
must add semantic compilation and executable benchmarks.
