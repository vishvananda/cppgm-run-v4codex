# PA6 final audit performance evidence

Final implementation: `85011bf5a`. Frozen compiler SHA-256:
`70571a477bac3857ec5c3988fe8798fa2a15b7db3cb89419344ee5964a27c075`.
The original twenty-workload campaign passes the complete protocol, hash,
output-equivalence, startup, latency, peak RSS, text and scaling verifier.
The added namespace-edge campaign is reported separately below.

## Controls, protocol and budgets

For AST mode, A is the completed PA5 stage base `9249196518...`. For types
mode, A is first complete PA6 `5749f43b4`; PA5 has no semantic mode. B is the
exact final implementation. The independent edge comparison uses checkpoint
`1edcbe5db` as A, so its improvement is attributable to this audit's changes.
Rebuilt historical binaries match the original recorded hashes exactly.

All binaries use `g++ -std=gnu++11 -Wall -O3`, with the course runner enabled.
Flags, source revisions, CPU/platform/compiler identity, binary/input/output
hashes and every observation are retained in
[`final-audit-performance.json`](../student.tests/pa6/final-audit-performance.json)
and [`final-edge-performance.json`](../student.tests/pa6/final-edge-performance.json).
No build or correctness suite ran during timing. Each subprocess shares the
same fixed CPU affinity and writes the same explicit dump. Four independent
primary TUs run per process (eight for nested syntax), releasing each graph at
TU end. Ordinary execution and telemetry execution are timed separately.

Every workload uses two A/A calibration pairs, one B/B pair and two ABBA blocks.
The original campaign contains 280 ordinary observations, 16 startup probes,
40 phase/work observations and 28 telemetry-overhead observations. All output
hashes agree between variants and telemetry modes. All ordinary workloads
exceed 20 times the measured startup maximum. No observation was discarded.
Tables report medians and full ABBA ranges, both paired B/A mean ratios and
A/A noise. Ratios below one mean faster compilation.

Budgets remain fixed: paired latency <=10% plus calibrated A/A noise, RSS
<=15% +1 MiB, host compiler text <=35% over PA5 and <=5% over first PA6,
4x workloads <6x wall and <5x RSS +1 MiB. The edge comparison uses the same
limits against its checkpoint control. The stage's original 25% feature-text
forecast was revised before checkpoint timing because the required semantic
surface exceeded it; this audit did not raise it or relax a failed gate.

## Final original-corpus results

Signature workloads improve 19–23% in paired latency versus first PA6. The
largest workload establishes 70 signature facts for 24,066 declarations/TU;
137 structural source/canonical types retain their separate roles. Completed
signatures reuse TypeIds. The large B range includes a 1.343-second observation;
it remains in both the raw data and paired analysis. The result is a compiler
latency benefit, not evidence about generated-program execution.

Other semantic pairs range from a 0.7% reduction to a 3.9% increase versus first
PA6, including the additional correctness checks. Inherited AST pairs range
from 1.5% faster to 1.3% slower; no broad AST speedup is claimed. The small nested
case has 10.09% A/A noise, which is disclosed rather than interpreted as a gain.

Largest template median peak RSS is 56,078 KiB versus 49,878 KiB for A (+12.4%);
largest namespace RSS is 62,632 versus 57,740 KiB (+8.5%). Both pass the original
15% +1 MiB budget in each ABBA block. Common entities remain 56 bytes, constants
16 bytes, and class-only demand records 32 bytes. The new Scope inline head and
edge/index storage are included in these measurements; no required fact was
removed to meet a memory limit.

Final host compiler `.text` is **277,702 bytes**: +33.18% versus PA5's 208,518,
+3.04% versus first PA6's 269,510, and +0.63% versus the audited checkpoint's
275,974. All static text budgets pass. Maximum final wall scaling is 4.148x
(constants), and maximum RSS scaling is 3.490x. The class corpus scales nested
class depth fourfold while keeping its 12,000 root object declarations fixed;
its 1.050x wall ratio is not a claim about quadrupling total source size.

Separate telemetry ABBA ratios add 0.33–1.19% for constants and 0.80–0.90% for
signatures, with equivalent output. Counters are observational: they do not
invoke additional semantic analyses.

| Input | Final wall seconds (range) | Final RSS KiB | B/A paired wall | A/A noise |
| --- | --- | --- | --- | --- |
| ast-classes-1 | 0.316 (0.316–0.318) | 17064 | 1.003, 0.989 | 2.33% |
| ast-classes-4 | 0.332 (0.329–0.336) | 17742 | 1.005, 1.013 | 0.21% |
| ast-declarations-1 | 0.528 (0.527–0.538) | 22978 | 0.999, 1.008 | 0.78% |
| ast-declarations-4 | 2.152 (2.134–2.164) | 80118 | 0.988, 1.009 | 1.74% |
| ast-expressions-1 | 1.588 (1.585–1.598) | 62992 | 1.000, 0.993 | 1.43% |
| ast-expressions-4 | 6.367 (6.359–6.378) | 207874 | 1.000, 1.005 | 0.44% |
| ast-nested-1 | 0.263 (0.260–0.266) | 8578 | 0.995, 0.994 | 10.09% |
| ast-nested-4 | 1.012 (1.010–1.020) | 21394 | 0.985, 0.990 | 0.83% |
| ast-procedural-1 | 0.861 (0.857–0.865) | 33446 | 1.002, 1.005 | 0.78% |
| ast-procedural-4 | 3.443 (3.438–3.471) | 116716 | 1.003, 0.998 | 0.27% |
| ast-templates-1 | 0.523 (0.521–0.524) | 21196 | 1.003, 0.999 | 2.18% |
| ast-templates-4 | 2.104 (2.099–2.108) | 72582 | 1.002, 1.005 | 1.03% |
| types-constants-1 | 0.419 (0.418–0.426) | 23356 | 1.011, 1.003 | 1.75% |
| types-constants-4 | 1.739 (1.728–1.760) | 80646 | 1.035, 1.017 | 1.45% |
| types-namespaces-1 | 0.317 (0.312–0.318) | 18094 | 1.016, 0.993 | 0.46% |
| types-namespaces-4 | 1.282 (1.277–1.293) | 62632 | 1.039, 1.034 | 0.42% |
| types-signatures-1 | 0.313 (0.310–0.317) | 15146 | 0.778, 0.782 | 1.23% |
| types-signatures-4 | 1.260 (1.236–1.343) | 48088 | 0.812, 0.769 | 0.28% |
| types-templates-1 | 0.362 (0.360–0.368) | 17090 | 1.020, 1.028 | 1.85% |
| types-templates-4 | 1.491 (1.485–1.496) | 56078 | 1.024, 1.037 | 1.33% |

## Namespace-edge corpus

Two fixed sizes, 6,000 and 24,000 namespace declarations, each have two qualified
using directives per namespace. Qualification isolates direct lookup and edge
insertion from language-required unqualified nomination traversal. The old
implementation rescanned prior edges on insertion and inspected ordinary edges
even on qualified direct hits. The final flat pair index and inline adjacency
remove both scans without omitting declarations or changing lookup results.

The complete edge campaign passes every protocol, output, startup, latency,
RSS, text, scaling and work check: 28 ordinary observations, 16 startup probes
and four separate phase/work observations. Both ABBA blocks show about 75%
lower latency at 6,000 namespaces and 92.4% lower latency at 24,000, well beyond
1.85% and 0.36% A/A noise respectively. Large-workload median wall time falls
from 15.290 to 1.163 seconds; median RSS changes from 50,162 to 49,376 KiB.
The smaller workload uses 15,530 versus 15,328 KiB, a disclosed 1.3% increase.
Geometric allocation thresholds and peak lifetimes mean the large RSS decrease
is not a claim that the new indexes themselves consume no memory.

Final work is exactly 6,000/24,000 unique edges and 12,000/48,000 qualified
scope visits per TU, despite two requests per edge. Final 4x wall scaling is
4.034x and RSS scaling is 3.179x. The measured latency reduction establishes
profitability; counters and source review explain the removed quadratic work.
All whole-pipeline memory/text budgets remain satisfied.

| Input | Final wall seconds (range) | Final RSS KiB | B/A paired wall | A/A noise |
| --- | --- | --- | --- | --- |
| types-edges-1 | 0.288 (0.287–0.294) | 15530 | 0.247, 0.249 | 1.85% |
| types-edges-4 | 1.163 (1.162–1.177) | 49376 | 0.076, 0.076 | 0.36% |

## Applicable executable measurements and retained history

Generated-program runtime, generated text, spills/loops in generated machine
code and actual self-hosting are **N/A** at PA6. PA6 emits a semantic dump; the
procedural corpus exercises loop/call/memory/floating-point frontend syntax.
No external backend or host compiler was used to manufacture executable results.
The compiler `.text` above is a separate host-built artifact. Instantiation,
LowIR/MIR/ELF and optimization legality/profitability/growth policies must be
measured when those surfaces exist.

- `performance.json`: complete checkpoint campaign for `1edcbe5db`, independently
  reverified against hash-identical frozen binaries. It supports historical
  checkpoint claims, not final-code claims.
- `superseded-performance.json`: partial campaign for `78df00b67`, stopped when
  review found incorrect free-function body deferral. No final claim uses it.
- `pre-compact-performance.json`: campaign for `3f6de92f5`; template RSS exceeded
  the 15% +1 MiB gate. The class-state compaction followed that failure. All
  observations remain retained, and the failed gate was not relaxed.

Reproduction and standalone sanitizer commands are in the
[personal validation guide](../student.tests/pa6/README.md).
