# PA17 query/candidate performance — loop 58

Frozen entry `411ad00e` versus code `f284514b`. The [complete campaign](../student.tests/pa17/query-performance.json) retains all sources, hashes, telemetry, warmups and observations.

Host build: g++ `-std=gnu++11 -Wall -O3`, course runner enabled. Student flags: `--emit-lowir -O0`. CPU affinity 31. Serial timing, without another build/test campaign. One warmup per binary, four A/A calibration samples, then four ABBA blocks. Newly accepted inputs have six final-only observations; the entry rejection is recorded and is never a performance baseline. Preflight validation/telemetry and native translation occur outside compiler timing.

Runtime sources retain volatile loop bounds and checked live results. Common LowIR and executables are byte-identical. The supplied O0 backend is used only by the harness. It emits sectionless ELF, so code size is code **plus alignment**: executable payload minus typed global data. Full payload/data/file sizes and hashes remain in JSON. The compiler's own backend and self-hosting are later-stage obligations.

| Compiler | SHA-256 | .text bytes |
|---|---|---:|
| Entry | `9104a568cf6bae039e37a213918f273d370badcf2d16e25bd0224fc6da064fae` | 1,780,870 |
| Final | `d5ef823d5b20bbf448eac2e1869111c0f1e33b7c4e30c2c232a53f487f4676a6` | 1,786,182 |

Compiler text grows 5,312 bytes. Entity records remain 120 bytes in both binaries. No optional optimization or new output expansion is added. Fully deduced candidates preserve the existing single default-validation path; only candidates needing defaults acquire an active key. Work is O(head width) plus candidate/query work already required by the language, with average O(1) flat-key lookup and TU lifetime. Generated code growth on common inputs is zero.

PA17/O0 has no mandated numeric latency, RSS or code-size ceiling. Historical +15%, +16 MiB and 5.5× diagnostic targets remain diagnostic under spec.md §9; no measurements or correctness/coverage requirements are discarded. The structural work bound permits no whole-program retry, extra candidate expansion, grammar replay, or extra generated-code expansion.

Compiler observations (milliseconds and KiB):

| Input | A ms [range] | B ms [range] | Paired B/A [range] | A/A ms range | Peak RSS A/B |
|---|---:|---:|---:|---:|---:|
| common-partials-1500 | 109.40 [108.61–111.66] | 110.31 [109.87–114.65] | 1.0098 [1.0054–1.0246] | 108.51–110.80 | 22,284/22,320 |
| common-partials-6000 | 447.68 [441.64–456.83] | 452.83 [448.45–461.59] | 1.0123 [0.9972–1.0183] | 445.82–453.53 | 72,984/73,036 |
| common-loop-float-1500 | 152.61 [151.27–155.85] | 152.95 [151.15–153.86] | 0.9990 [0.9913–1.0078] | 151.76–154.82 | 30,244/30,164 |
| common-loop-float-6000 | 594.06 [588.91–641.70] | 602.93 [589.89–827.46] | 1.0209 [0.9658–1.2010] | 592.23–673.02 | 103,704/103,616 |
| common-candidate-defaults-1500 | 130.16 [125.60–132.56] | 131.12 [126.31–133.04] | 1.0066 [0.9973–1.0278] | 126.72–127.91 | 29,676/29,500 |
| common-candidate-defaults-6000 | 538.05 [528.80–549.11] | 546.00 [536.37–602.28] | 1.0191 [0.9984–1.0574] | 528.62–565.03 | 99,716/101,392 |
| common-candidate-deduced-1500 | 120.84 [117.92–122.86] | 120.96 [118.56–122.90] | 1.0001 [0.9910–1.0095] | 120.86–128.72 | 28,200/28,040 |
| common-candidate-deduced-6000 | 509.17 [497.16–523.29] | 504.66 [499.19–519.15] | 0.9975 [0.9869–1.0047] | 502.32–513.47 | 96,340/96,176 |
| recursive-candidate-200 | rejected | 52.08 [51.56–52.75] | final only | — | —/13,076 |
| recursive-candidate-800 | rejected | 201.03 [197.14–205.18] | final only | — | —/36,120 |
| failed-operator-query-600 | rejected | 45.43 [44.90–47.11] | final only | — | —/12,212 |
| failed-operator-query-2400 | rejected | 172.23 [167.46–329.83] | final only | — | —/31,376 |

Runtime observations (milliseconds), with generated code and data sizes:

| Input | A ms [range] | B ms [range] | Paired B/A [range] | A/A ms range | Code+alignment A/B | Data A/B |
|---|---:|---:|---:|---:|---:|---:|
| runtime-calls | 360.03 [358.81–366.27] | 362.37 [359.05–392.05] | 0.9998 [0.9950–1.0508] | 361.84–371.75 | 206/206 | 0/0 |
| runtime-memory | 211.10 [210.18–212.78] | 211.80 [210.26–216.07] | 1.0043 [0.9974–1.0099] | 210.20–211.35 | 434/434 | 0/0 |
| runtime-floating | 249.23 [248.65–250.01] | 249.14 [248.08–252.24] | 0.9998 [0.9974–1.0059] | 248.47–249.70 | 230/230 | 0/0 |
| runtime-recursive-candidate | rejected | 226.38 [225.12–227.69] | final only | — | —/184 | —/0 |

Common compiler paired medians range from 0.9975 to 1.0209; the largest additional measured peak RSS is 1,676 KiB in the 6,000-default-candidate input. The approximately 1.9% paired cost there accompanies required active default-query ownership, with no entity-size increase. The fully deduced path has no observed median regression. No speedup is claimed. Common runtime binaries are identical; their timing differences are execution noise. Compiler timing spreads and A/A calibration are disclosed above. Compilation of the tiny runtime sources is startup-sensitive and remains in JSON, rather than supporting a latency claim. Final-only workloads quantify the necessary new semantic cost, with no claim against a compiler that rejects the program.

Work scaling:

| Final input | Queries / computations | Candidate substitutions / cycle observations |
|---|---:|---:|
| recursive-candidate-200 | 1435/1435 | 400/200 |
| recursive-candidate-800 | 5635/5635 | 1600/800 |
| failed-operator-query-600 | 3015/3015 | 0/0 |
| failed-operator-query-2400 | 12015/12015 | 0/0 |

On these measured inputs, each canonical query is computed once. Four times as many recursive inputs produces four times as many candidate substitutions/cycle observations; failed-operator queries scale with distinct types. The active mark itself is not a cached failure: it is reset when that candidate returns, and later declaration/default controls remain valid.

Two complete earlier campaigns are preserved: [before field packing](../student.tests/pa17/query-performance-before-packing.json) measured an avoidable 128-byte entity, subsequently restored to 120 bytes; [before the fully deduced fast path](../student.tests/pa17/query-performance-before-fastpath.json) predates removal of redundant default validation. The final corpus adds two fully deduced call sizes. Earlier results remain observations, while this final implementation and its common-output checks supply acceptance evidence.
