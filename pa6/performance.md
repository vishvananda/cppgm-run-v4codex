# PA6 compiler performance evidence

Final implementation: `1edcbe5dbbc7af3300ffd117cf05191571e4395d`.
The complete frozen campaign passes the protocol, hash, output-equivalence,
latency, peak RSS, compiler-text and scaling verifier.

## Method and scope

For `--emit-ast`, A is the PA6 entry binary (`9249196518...`, completed PA5).
For `--emit-types`, A is the first complete PA6 implementation (`5749f43b4`).
B is the final implementation in both comparisons. The entry binary cannot
emit types, so it is not used as a semantic timing control. All binaries use
`g++ -std=gnu++11 -Wall -O3` with the course test runner enabled. Full binary
hashes, host compiler version/platform, CPU affinity, flags, source revisions,
input/output hashes and every observation are in
[`performance.json`](../student.tests/pa6/performance.json).

Twenty fixed workloads cover inherited declarations, expressions, templates,
nested syntax, classes and procedural loops/calls/memory/floating-point syntax,
plus semantic constants, template parameter environments, namespace graphs and
compound signatures. Each has 1x/4x input sizes. Use four primary-file repetitions
per process (eight for nested syntax). Every ordinary workload exceeds 20 times
the measured startup maximum. Builds/tests were stopped during timing.

Each comparison has two A/A calibration pairs, one B/B pair, then two ABBA
blocks: 280 ordinary observations, 16 startup probes, 40 separate phase/work
observations and 28 telemetry-overhead observations. The table reports final
medians and full paired-run ranges, both paired B/A latency ratios, and A/A
noise. Ratios below one mean faster compilation. Every output hash agrees across
A/B and ordinary/telemetry runs. No observation was dropped.

Budgets were fixed before the accepted campaign: paired latency <=10% plus
A/A noise, RSS <=15% +1 MiB, final host compiler text <=35% above the stage base
and <=5% above first PA6, fourfold input <6x wall and <5x RSS +1 MiB. The original
25% feature-text forecast was revised to 35% before timing because the required
new semantic surface exceeded it. The failed memory gate was not relaxed.

## Results and limits

Repeated compound-signature compilation improves about 22% in both blocks,
well beyond A/A noise. The largest signature workload completes
70 distinct signature facts for
24066 source declarations per TU; repeated requests
reuse the completed identities. The direct API additionally checks repeated
requests and deep pointer-chain growth.

The other semantic groups cost roughly 1–4% more compiler time than the first
PA6 binary while adding the audited lookup, declaration-point, class-layout and
constant-fact behavior. This is not a broad frontend speedup claim. All inherited
PA5 workloads remain within their preservation budgets; small nested/template
workloads have about 10–11% A/A noise, which is disclosed rather than interpreted
as a speedup.

The largest template workload now uses 55,350 KiB median peak RSS versus
49,874 KiB for A (about 11% growth, within budget). Common entities occupy 56
bytes instead of the previous candidate's 96; constants occupy 16 instead of 24.
Class-only layout/constructor state is one indexed 32-byte record per class.
No fact or language check was removed to reduce memory.

Final compiler `.text` is 275,974 bytes:
+32.35% versus PA5's 208,518,
+2.40% versus first PA6's 269,510.
Worst fourfold wall scaling is 4.123x (types-templates-4); all RSS scaling
checks pass. Telemetry adds 1.5–1.9% on large constant workloads and between
noise and 1.4% on large signature workloads; output remains identical.

Generated-program runtime, generated text and actual self-hosting are **N/A**:
PA6 emits a semantic dump. Host compiler text above is reported separately and
is not presented as generated-program code size. No executable optimization or
runtime profitability is claimed.

| Input | Final wall seconds (range) | Final RSS KiB | B/A paired wall | A/A noise |
| --- | --- | --- | --- | --- |
| ast-classes-1 | 0.317 (0.315–0.318) | 16992 | 0.987, 0.997 | 0.99% |
| ast-classes-4 | 0.330 (0.329–0.332) | 17640 | 1.001, 1.001 | 1.54% |
| ast-declarations-1 | 0.526 (0.522–0.531) | 23140 | 0.996, 1.005 | 0.83% |
| ast-declarations-4 | 2.138 (2.135–2.152) | 80100 | 0.999, 1.007 | 0.70% |
| ast-expressions-1 | 1.588 (1.585–1.614) | 62890 | 1.008, 0.996 | 0.57% |
| ast-expressions-4 | 6.329 (6.311–6.338) | 207884 | 0.997, 0.998 | 0.69% |
| ast-nested-1 | 0.260 (0.259–0.261) | 8448 | 0.992, 0.991 | 10.37% |
| ast-nested-4 | 1.013 (1.010–1.019) | 21474 | 0.991, 0.996 | 1.35% |
| ast-procedural-1 | 0.856 (0.853–0.860) | 33440 | 1.002, 1.000 | 0.25% |
| ast-procedural-4 | 3.447 (3.435–3.454) | 116704 | 1.000, 1.000 | 1.19% |
| ast-templates-1 | 0.523 (0.520–0.526) | 21202 | 1.002, 1.002 | 11.00% |
| ast-templates-4 | 2.117 (2.107–2.122) | 72706 | 0.997, 1.002 | 0.15% |
| types-constants-1 | 0.426 (0.424–0.427) | 23342 | 1.029, 1.018 | 0.94% |
| types-constants-4 | 1.748 (1.738–1.759) | 80686 | 1.015, 1.018 | 0.83% |
| types-namespaces-1 | 0.321 (0.319–0.324) | 18430 | 1.025, 1.022 | 0.82% |
| types-namespaces-4 | 1.287 (1.279–1.293) | 60608 | 1.032, 1.038 | 0.53% |
| types-signatures-1 | 0.313 (0.313–0.316) | 15128 | 0.776, 0.784 | 1.56% |
| types-signatures-4 | 1.246 (1.243–1.251) | 48204 | 0.778, 0.780 | 0.91% |
| types-templates-1 | 0.360 (0.359–0.363) | 16494 | 1.011, 1.010 | 3.90% |
| types-templates-4 | 1.485 (1.480–1.488) | 55350 | 1.023, 1.030 | 0.72% |

## Retained unsuccessful campaigns

- `superseded-performance.json`: partial campaign for `78df00b67`, explicitly
  stopped when review found incorrect free-function body deferral. It supports
  no final-binary performance claim.
- `pre-compact-performance.json`: complete campaign for `3f6de92f5`; the largest
  template RSS increased about 20% and failed the 15% +1 MiB gate. The compact
  entity/class-state correction followed this failure. All observations remain
  available; the current campaign uses the corrected frozen binary throughout.

Reproduction and standalone sanitizer commands are in
[`student.tests/pa6/README.md`](../student.tests/pa6/README.md).
