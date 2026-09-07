# PA8 personal validation

Run explicitly; course discovery is unchanged:

```sh
python3 student.tests/pa8/check.py
python3 student.tests/pa8/check_native.py
python3 student.tests/pa8/build_checks.py /tmp/pa8-final-audit/sanitize-final
python3 student.tests/pa8/benchmark.py verify student.tests/pa8/final-audit-performance.json
python3 student.tests/pa8/frontend_benchmark.py verify /tmp/pa8-final-audit/cppgm-frontend student.tests/pa8/final-frontend-performance.json
```

`check.py` covers 24 valid and 56 invalid cases, writer fixed points, helper-only
units, multifile identity, forward references, parallel phi inputs, scalar
conversions, metadata placement, literals, object boundaries and CLI failures.
Rejections must exit 1; crashes are never accepted. Literal probes preserve
signalling NaNs and large signed/unsigned integer spellings beyond the fixtures.
The final audit adds f80 phi rejection, ordinary edges into handlers/cleanup,
and registrations appearing after their target blocks.

`check_api.cpp` checks compact types, IDs surviving pool/interner growth,
independent units, construction without text, model edits observed by the
writer, local shape restrictions, handler fact invalidation after edits, and
rejection of interleaved builder slices. `build_checks.py` builds the API and
standalone tool with ASan/UBSan and default leak detection outside the repo.
`check_native.py` routes our generated LowIR through the supplied PA8 backend,
covers every allowed sum input, aliased swaps, callbacks and floating memory
work. The compiler implementation never invokes this backend. The unchanged
PA7 `check_audit.py` and `check_api.cpp` were also run in the final audit.

## Frozen final measurements

The incoming LowIR A is `7313af05a` (same implementation and binary hash as
checkpoint B, `01d39a2f6`); final B's implementation is `bc2cd043d`. Build each
checkout with `make -C dev lowir`, using the recorded GNU C++11/O3 flags and
separate object roots/checkouts, and copy binaries to the frozen paths below.
The unchanged frontend is built with `make -C dev cppgm++` and copied to
`/tmp/pa8-final-audit/cppgm-frontend`. The measurement harness/protocol was
frozen in `2f21a26ac`; current JSON records implementation and harness hashes.

```sh
python3 student.tests/pa8/benchmark.py measure \
  /tmp/pa8-final-audit/lowir-A /tmp/pa8-final-audit/lowir-B \
  student.tests/pa8/final-audit-performance.json \
  --base-count=12000 --runtime-factor=40 --base-commit=7313af05a
python3 student.tests/pa8/frontend_benchmark.py measure \
  /tmp/pa8-final-audit/cppgm-frontend \
  student.tests/pa8/final-frontend-performance.json
```

Run timing campaigns sequentially, without simultaneous builds/tests. Choose a
new destination filename for follow-ups to retain the original observations.
The fixed compiler corpus covers integer/call, memory/floating, loop/phi and
handler/cleanup families at 1x/4x sizes. Native workloads have about 40 million
iterations, runtime volatile inputs and checked accumulated results. The
frontend wrapper reuses the unchanged PA7 template-demand corpus, protocol,
work assertions and budgets, measuring one frozen compiler under both labels.

[Protocol and budgets](final-audit-protocol.md) were recorded before timing.
[Final report](../../pa8/performance.md) gives latency, RSS, runtime, text,
paired spread, noise, invalidation/work bounds and provenance limits. There is
no optimizer or speedup claim. All raw observations remain in the JSON files.

Generated inputs, outputs, objects, frozen binaries and logs live under `/tmp`
and are not committed. Verification requires those artifacts. After cleanup,
rebuild frozen checkouts and rerun measurements into new records; this produces
new observations, not verification of measurements whose artifacts are gone.
The historical `performance.json` (first working `66167cf72` versus `01d39a2f6`)
is preserved, but its temporary artifacts were absent at final-audit entry and
its results were not reused as a fresh verification gate. Its generator is
available in `7313af05a`; the final generator additionally covers handlers.
