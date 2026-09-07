# Independent PA9 final audit protocol

Frozen before audit implementation changes or timing. A is the completed
checkpoint `6ec119239` (implementation `ea87f4ba7`); B will be the committed
audit fixes, built with the same g++ and `-std=gnu++11 -Wall -O3`. The existing
eight workloads and sizes in `benchmark.py` remain fixed. Copy both binaries,
record hashes, check equal output, then run AAAA calibration and two wall-time
ABBA blocks per workload, pinned to one allowed CPU without concurrent builds
or tests. Retain every observation, report paired ratios and spread, external
peak RSS and compiler text. Generated runtime/text is N/A: PA9 emits names.

Also run a separate complete 96-observation comparison against the independently
rebuilt first-correct `df7dbb00a` binary, using the same B and fixed inputs.
This checks whole-stage latency/RSS budgets directly rather than combining
ratios from separate historical runs. Retain the checkpoint comparison as the
audit delta; do not pool the two experiments or select the more favorable one.

Keep the existing wall (1.25x), RSS (1.20x + 16 MiB), fourfold wall (5.5x), work
(4.5x), RSS (5x), and compiler text (+100 KiB) gates. Also retain the original
100 KiB total text-growth budget relative to first-correct `df7dbb00a` (172418
bytes). Correctness repairs have no speedup requirement; disclose regressions.
Any speed claim must repeat in both paired blocks and exceed calibration
spread. No claim follows from fewer nodes or substitutions alone.

Review all stage commits from `affdafd23` through `6ec119239`, including the
previously unaudited serializer and evidence handoffs. Trace graph construction,
canonical identities, substitution order, contexts/external entities, optional
serialization and release. Add independent ownership-path and nesting probes,
run them explicitly (also with ASan/UBSan), then run PA9, the cumulative report
and file audit. Commit fixes before timing to give B immutable provenance.

Previous goal state: completed checkpoint and its log provide evidence, but
not an independent final audit. This turn makes progress by reconstructing
source ownership and reproducing defects. The saved through log says 904/904;
the final report must use fresh authoritative counts.
