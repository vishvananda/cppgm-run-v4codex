# PA2 compiler performance evidence

Two completed campaigns retain 168 timed observations: seven fixed inputs,
two A/A calibration pairs and two ABBA blocks per input per campaign.
A is ordinary PA2 execution; B is the identical frozen binary with `--stats`.
The initial campaign preceded the invalid-character counter correction. The
final campaign uses the corrected implementation and pins CPU 0; it reuses all
seven initial input files byte-for-byte, including the frozen self-source text.
Both campaigns verify complete A/B stdout hashes before timing. No speedup or
generated-code optimization benefit is claimed. The incomplete PA2 starter is
not a performance baseline. Generated-program runtime and text size are **N/A**:
PA2 emits tokens. Host-tool text is reported separately below.

Compiler wall time includes process startup, source reading, conversion and
required output formatting to `/dev/null`. `/usr/bin/time` records the measured
compiler process peak RSS. Hashing/warmup telemetry is untimed and can inherit
the Python parent's RSS high-water mark; its RSS is not substituted for timed
process RSS. Samples are 0.2–3.3 seconds, longer than startup. The self-source
input includes directives, which PA2 reports as invalid tokens; it measures
tokenization only. These early-stage workloads do not measure template
instantiation or executable behavior. Every invocation has a 60-second timeout.

Explicit resource limits and their storage derivation are in [README](README.md).
Both campaigns pass every latency, work, capacity and measured RSS envelope.
No code transform, work-budget escalation or generated text growth is involved.
Telemetry overhead is noisy: signs differ across paired blocks/workloads and
A/A noise reaches double-digit percentages. Negative deltas are not evidence
that collecting counters speeds up the compiler. All positive deltas/regressions
remain visible below; no observation was discarded.

Host: Linux-7.0.0-1005-gcp-x86_64-with-glibc2.43. CPU: Intel(R) Xeon(R) CPU @ 2.20GHz.
Compiler: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0. Build flags: `-std=gnu++11 -Wall -O3`;
entry uses `-Dmain=test_runner_real_main`, with the inherited course runner linked.
The benchmark invokes the ordinary CLI, not its batch mode.

## Initial implementation

Artifacts: `obj/student-pa2/performance-final`.
Binary SHA-256: `28e97e5232c69bb0c30d8bdc3a2d49e9c7bffd0a9854954b71b844053b56f43e`.
Manifest SHA-256: `58db763305a9f487491081201acc4bb0b36c5a2a35966aba0e368e97ff0dea5a`.
Observations SHA-256: `e3fb50a2a2eb1135d0f3b5300919de3e4bc10bd18918bb33b73df3f4cee11299`.
Frozen harness SHA-256: `268e22c9d9e84f722e28cc747e51df5d9b343570e1b0e81435ff5887bcbaa7e2`.
Host-tool `size` (text/data/bss/total): `83226 4616 1272 89114`.
Affinity: `0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31`.

| Workload | Bytes | A median s [min, max] | B median s [min, max] | Peak RSS A/B KiB | A/A changes % | Paired B/A changes % |
| --- | ---: | --- | --- | --- | --- | --- |
| repeated-4 | 4194186 | 0.758621 [0.757037, 0.769163] | 0.761957 [0.758092, 0.793421] | 8028/8020 | -0.679, -3.584 | -0.832, 2.953 |
| repeated-16 | 16777161 | 3.008741 [3.006664, 3.055975] | 3.002577 [3.000250, 3.035605] | 20308/20308 | -0.040, -0.986 | -0.380, -0.267 |
| floating-4 | 4194300 | 0.600832 [0.587348, 0.611759] | 0.600767 [0.589580, 0.691338] | 8024/8024 | -3.774, 1.997 | 5.926, 0.841 |
| self-source-4 | 4193664 | 0.527794 [0.526355, 0.566931] | 0.524838 [0.520973, 0.527547] | 8020/8024 | 0.849, -0.078 | -4.467, -0.170 |
| unique-suffixes-200000 | 3488891 | 0.270723 [0.266934, 0.278388] | 0.273723 [0.268345, 0.358955] | 20844/20844 | 1.527, 1.383 | -0.539, 16.668 |
| raw-near-4 | 4456486 | 0.397069 [0.386058, 0.403708] | 0.400160 [0.396253, 0.403733] | 78176/78148 | 0.971, 0.706 | 0.736, 1.336 |
| concat-late-4 | 3728248 | 0.273687 [0.271814, 0.274304] | 0.280344 [0.274934, 0.284494] | 21916/21988 | 6.623, -1.091 | 2.358, 2.512 |

Fourfold repeated input latency ratio A/B: 3.966064/3.940614 (limit 6x).

| Workload | Input SHA-256 | Equivalent output SHA-256 |
| --- | --- | --- |
| repeated-4 | `58e9f091c24a6ecc6fb1609c14a88203e2b8cc59d8a8e0617427ac980aa77198` | `a44c3327a7166ebff923ea1d5e15ab6657568bc5203dfc9fb8bc1a7fcc35ab2a` |
| repeated-16 | `cd2a9635a5adf29172bb951dcdd8504501d67ba8e04cbd1c796436944c3e6390` | `e63c4b01978e6a2601e43429d62afcc3aa2b96ba4b39941f458a9b99061eeaab` |
| floating-4 | `75c8fe2a6896574a44801adec67abb91502aeb1df1d477a4d6e53ae5d688eff2` | `f2ed391a4c6e3e6b1144ddcb355989d488b583acf6f1175a005be76d5a20cb64` |
| self-source-4 | `1d723504ae825b927e4abd76d3915628929345431e11c9b8929624fa1ee138d2` | `e76c7e8ff1b7dcf47757a3d7b5eee7e0b704b255567fbd952a531e093c13b940` |
| unique-suffixes-200000 | `f9ee06b508023b382d48c89bf5110a922ff8bc552e06a4e833c99830c75f8b12` | `34d8bb103a580c470b06336aa00868ec4cf0bd0fd6d31d676e8b07ad1c577f4b` |
| raw-near-4 | `bee214a174525902fed8e376bb96a39cc4e41031a1c18684efcb318b52be172d` | `e227366bbe3f80e34bf3170a66ee2919f7ff0a6b5d80d160f1dec35769ffb2fc` |
| concat-late-4 | `0f8b151a3b21c879d044ed9a80be46a236f5ddc97c16fc3e7340e29fddeea1d0` | `48bab2e0d95b73397f0407404ec1402a7902ffa6d2e4b1b552a8fded69db2dbc` |

All wall-time observations below retain the measured order `AAAA ABBA ABBA`.
The first four entries are the two calibration pairs. Times are seconds to
nanosecond resolution as recorded by the harness; RSS entries are KiB in the
same order. Optional per-sample telemetry is retained in `observations.json`.

| Workload | 12 wall times | 12 peak RSS values |
| --- | --- | --- |
| repeated-4 | 0.761342459, 0.756173299, 0.782605325, 0.754555958, 0.760138002, 0.758091627, 0.758485038, 0.769162821, 0.757104842, 0.793420589, 0.765427970, 0.757036676 | 8016, 7996, 8028, 8016, 7840, 7996, 8008, 8016, 7840, 8020, 8004, 8024 |
| repeated-16 | 3.033265408, 3.032045356, 3.052196703, 3.022099805, 3.006664124, 3.004002081, 3.035605266, 3.055974679, 3.010617622, 3.001151386, 3.000249818, 3.006863970 | 20064, 20308, 20300, 20128, 20308, 20308, 20292, 20308, 20304, 20300, 20296, 20308 |
| floating-4 | 0.618131799, 0.594801265, 0.594319361, 0.606186562, 0.597495080, 0.691337750, 0.589579635, 0.611758998, 0.604168022, 0.603829480, 0.597703605, 0.587347815 | 7976, 7844, 8024, 7848, 8012, 8024, 8020, 7768, 8004, 8020, 7848, 8024 |
| self-source-4 | 0.528240930, 0.532724427, 0.532112181, 0.531695268, 0.527494153, 0.520972724, 0.524564684, 0.566931179, 0.528093943, 0.525110762, 0.527547358, 0.526355283 | 7984, 7840, 7976, 8000, 8020, 7840, 8024, 8020, 8020, 7812, 8024, 7840 |
| unique-suffixes-200000 | 0.274415092, 0.278604429, 0.269626754, 0.273355094, 0.272840873, 0.270183130, 0.268344730, 0.268605244, 0.266934491, 0.277261878, 0.358955202, 0.278387975 | 20836, 20836, 20660, 20780, 20844, 20664, 20816, 20836, 20808, 20844, 20660, 20796 |
| raw-near-4 | 0.396477810, 0.400327587, 0.398638542, 0.401453625, 0.397370205, 0.396252534, 0.403733144, 0.396767753, 0.403707684, 0.399959264, 0.400361193, 0.386057866 | 78172, 78156, 77996, 77928, 77980, 78140, 77984, 78176, 78152, 78136, 78148, 78172 |
| concat-late-4 | 0.276921234, 0.295261221, 0.278354957, 0.275319291, 0.271813646, 0.284059644, 0.274934299, 0.274303718, 0.273749817, 0.284494065, 0.276628790, 0.273624023 | 21916, 21896, 21876, 21732, 21732, 21912, 21988, 21912, 21736, 21912, 21920, 21912 |

## Final implementation, CPU 0

Artifacts: `obj/student-pa2/performance-verified`.
Binary SHA-256: `4c50a636dd26988e09134222743ec39f8062949479befb1ef298afefb7ee99fb`.
Manifest SHA-256: `f9de0b17c27fd3d519c0840e8f89cae9164fd49ea9c7018adfe95d25cd37f2be`.
Observations SHA-256: `a849f24e3e5de2e3cf3aaacff821d6813cf50ddf47bdfa0feb2153bfda27bb9d`.
Frozen harness SHA-256: `268e22c9d9e84f722e28cc747e51df5d9b343570e1b0e81435ff5887bcbaa7e2`.
Host-tool `size` (text/data/bss/total): `83290 4616 1272 89178`.
Affinity: `0`.

| Workload | Bytes | A median s [min, max] | B median s [min, max] | Peak RSS A/B KiB | A/A changes % | Paired B/A changes % |
| --- | ---: | --- | --- | --- | --- | --- |
| repeated-4 | 4194186 | 0.797030 [0.794661, 0.798896] | 0.772421 [0.756545, 0.791387] | 8024/8024 | 4.917, 4.000 | -2.852, -3.099 |
| repeated-16 | 16777161 | 3.130805 [3.024660, 3.146225] | 3.113837 [3.046032, 3.285940] | 20312/20308 | 1.454, 0.013 | 2.217, -0.196 |
| floating-4 | 4194300 | 0.607338 [0.598615, 0.613891] | 0.602693 [0.595730, 0.606244] | 8020/8020 | -0.631, 1.359 | -0.549, -1.079 |
| self-source-4 | 4193664 | 0.529244 [0.527623, 0.545970] | 0.534176 [0.529094, 0.564347] | 8024/8024 | -0.721, -1.517 | 0.018, 2.749 |
| unique-suffixes-200000 | 3488891 | 0.302952 [0.279242, 0.312850] | 0.272656 [0.270897, 0.280070] | 20840/20824 | -12.757, 0.345 | -10.573, -6.360 |
| raw-near-4 | 4456486 | 0.400116 [0.398041, 0.405321] | 0.406924 [0.404353, 0.428228] | 78172/78172 | -9.928, 8.972 | 4.036, 1.296 |
| concat-late-4 | 3728248 | 0.276147 [0.274413, 0.284866] | 0.278721 [0.277023, 0.280249] | 21912/22024 | -0.773, 2.031 | 1.738, -1.140 |

Fourfold repeated input latency ratio A/B: 3.928087/4.031271 (limit 6x).

| Workload | Input SHA-256 | Equivalent output SHA-256 |
| --- | --- | --- |
| repeated-4 | `58e9f091c24a6ecc6fb1609c14a88203e2b8cc59d8a8e0617427ac980aa77198` | `a44c3327a7166ebff923ea1d5e15ab6657568bc5203dfc9fb8bc1a7fcc35ab2a` |
| repeated-16 | `cd2a9635a5adf29172bb951dcdd8504501d67ba8e04cbd1c796436944c3e6390` | `e63c4b01978e6a2601e43429d62afcc3aa2b96ba4b39941f458a9b99061eeaab` |
| floating-4 | `75c8fe2a6896574a44801adec67abb91502aeb1df1d477a4d6e53ae5d688eff2` | `f2ed391a4c6e3e6b1144ddcb355989d488b583acf6f1175a005be76d5a20cb64` |
| self-source-4 | `1d723504ae825b927e4abd76d3915628929345431e11c9b8929624fa1ee138d2` | `e76c7e8ff1b7dcf47757a3d7b5eee7e0b704b255567fbd952a531e093c13b940` |
| unique-suffixes-200000 | `f9ee06b508023b382d48c89bf5110a922ff8bc552e06a4e833c99830c75f8b12` | `34d8bb103a580c470b06336aa00868ec4cf0bd0fd6d31d676e8b07ad1c577f4b` |
| raw-near-4 | `bee214a174525902fed8e376bb96a39cc4e41031a1c18684efcb318b52be172d` | `e227366bbe3f80e34bf3170a66ee2919f7ff0a6b5d80d160f1dec35769ffb2fc` |
| concat-late-4 | `0f8b151a3b21c879d044ed9a80be46a236f5ddc97c16fc3e7340e29fddeea1d0` | `48bab2e0d95b73397f0407404ec1402a7902ffa6d2e4b1b552a8fded69db2dbc` |

All wall-time observations below retain the measured order `AAAA ABBA ABBA`.
The first four entries are the two calibration pairs. Times are seconds to
nanosecond resolution as recorded by the harness; RSS entries are KiB in the
same order. Optional per-sample telemetry is retained in `observations.json`.

| Workload | 12 wall times | 12 peak RSS values |
| --- | --- | --- |
| repeated-4 | 0.762368448, 0.799853841, 0.790612410, 0.822234440, 0.798713465, 0.756545417, 0.791387295, 0.794660823, 0.798895567, 0.772425159, 0.772416452, 0.795347113 | 8016, 7840, 8004, 8024, 7992, 7996, 8024, 8020, 8000, 7996, 8020, 8020 |
| repeated-16 | 3.128356971, 3.173835277, 3.151383666, 3.151805328, 3.146225263, 3.285940160, 3.133490214, 3.133977898, 3.127631193, 3.094184564, 3.046032164, 3.024659837 | 20312, 20312, 20164, 20312, 20312, 20304, 20308, 20312, 20308, 20308, 20308, 20308 |
| floating-4 | 0.606765593, 0.602936964, 0.589786489, 0.597800328, 0.598614521, 0.595730418, 0.599160932, 0.602869214, 0.611806130, 0.606244316, 0.606225357, 0.613891090 | 8020, 7988, 8020, 8008, 7840, 7992, 8020, 8000, 8020, 7992, 7984, 7980 |
| self-source-4 | 0.526161823, 0.522365831, 0.535797146, 0.527671001, 0.530517978, 0.529589485, 0.529094224, 0.527970726, 0.545970431, 0.538763316, 0.564346526, 0.527622668 | 7976, 7844, 8024, 7848, 7988, 8012, 7840, 7840, 7996, 8000, 8024, 7996 |
| unique-suffixes-200000 | 0.307514399, 0.268285323, 0.275493176, 0.276443364, 0.305406656, 0.270947415, 0.270896895, 0.300498177, 0.279242473, 0.280069785, 0.274363672, 0.312850180 | 20840, 20824, 20796, 20816, 20808, 20820, 20664, 20660, 20816, 20660, 20824, 20832 |
| raw-near-4 | 0.441410968, 0.397588901, 0.401824560, 0.437876938, 0.399248224, 0.428228366, 0.408817088, 0.405321071, 0.400983727, 0.405030371, 0.404353278, 0.398041073 | 78148, 78156, 78172, 77988, 78160, 78148, 78172, 78168, 78172, 78172, 77988, 78144 |
| concat-late-4 | 0.277955999, 0.275806558, 0.278956706, 0.284622623, 0.275142825, 0.280249240, 0.278856985, 0.274413403, 0.277151201, 0.278585486, 0.277023380, 0.284866370 | 21908, 21896, 21912, 21912, 21896, 21900, 21988, 21912, 21912, 21900, 22024, 21912 |

## Final work and retained storage

| Workload | Post tokens / invalid | Number / literal bytes | Decoded elements | String parts | Encoded string bytes | Post scratch bytes / growths | Name storage bytes |
| --- | --- | --- | ---: | ---: | ---: | --- | ---: |
| repeated-4 | 1569049/0 | 362088/0 | 0 | 0 | 0 | 30/0 | 618 |
| repeated-16 | 6276349/0 | 1448388/0 | 0 | 0 | 0 | 30/0 | 618 |
| floating-4 | 613801/0 | 3580500/0 | 0 | 0 | 0 | 30/0 | 92 |
| self-source-4 | 848385/7808 | 75648/300864 | 224384 | 27072 | 243456 | 646/11 | 22204 |
| unique-suffixes-200000 | 200001/0 | 3288890/0 | 0 | 0 | 0 | 30/0 | 12935068 |
| raw-near-4 | 2/0 | 0/4456485 | 4456448 | 1 | 4456449 | 79429669/44 | 64 |
| concat-late-4 | 2/0 | 0/3262217 | 932061 | 466031 | 1864126 | 14286848/56 | 64 |

Final binary and all implementation/build-source hashes were compared against
the current worktree after measurement. Every frozen input and both binary
hashes were rechecked. The final 4/16 MiB stream retains the same 618 bytes of
identifier storage and 30 bytes of post-token scratch, independent of total
token count. A single long string legitimately retains its decoded sequence
and output until the next cursor pull. That workload stays within the declared
linear scratch/RSS limits; this report makes no reduction claim for that storage.
