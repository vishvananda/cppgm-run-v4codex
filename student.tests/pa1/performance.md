# PA1 compiler performance evidence

Run: 2026-09-07 UTC, implementation commit `78a0872d6`. This is a compiler
baseline and telemetry comparison, not an optimization or generated-code claim.

Host: Linux-7.0.0-1005-gcp-x86_64-with-glibc2.43; Intel(R) Xeon(R) CPU @ 2.20GHz.
Compiler: g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0; `-std=gnu++11 -Wall -O3`, supplied test runner enabled.
Frozen binary SHA-256: `bdb50c54f12d6ea1b58204edcaed46aeb298cfd5a987eae169c8238532ec03d5`.

A uses the default CLI; B uses the same frozen binary with `--stats`. Each
workload has two A/A calibration pairs followed by two ABBA blocks. Full output
SHA-256 matches between A and B before timing. Wall time includes startup, input
reading, scanning and required token formatting to `/dev/null`; RSS is measured
by `/usr/bin/time`. Output-hashing warmups are excluded from these times/RSS.

Generated-program runtime and text size: **N/A** for every row (PA1 emits tokens).

| Workload | Input bytes | A median [min, max], s | B median [min, max], s | Max RSS, KiB |
| --- | ---: | --- | --- | ---: |
| repeated-8 | 8388511 | 1.503379 [1.493619, 1.509029] | 1.495884 [1.493225, 1.500399] | 12096 |
| repeated-32 | 33554322 | 5.998776 [5.980678, 6.040731] | 5.986913 [5.956214, 6.039817] | 36668 |
| translations-8 | 8388579 | 0.646877 [0.642922, 0.648646] | 0.650390 [0.645059, 0.658675] | 12096 |
| self-source-8 | 8364960 | 1.065686 [1.052848, 1.257020] | 1.070881 [1.050439, 1.115105] | 12092 |
| unique-200000 | 2288891 | 0.219877 [0.211730, 0.222805] | 0.216234 [0.213934, 0.222599] | 19952 |

| Workload | A/A changes, % | Paired ABBA B/A changes, % |
| --- | --- | --- |
| repeated-8 | +1.629, +0.665 | -0.241, -0.557 |
| repeated-32 | -0.368, -0.516 | -0.490, +0.080 |
| translations-8 | +0.311, +1.146 | +0.006, +1.478 |
| self-source-8 | -18.041, -2.601 | -5.824, +0.080 |
| unique-200000 | -2.087, +7.895 | -0.529, -0.680 |

Telemetry has no repeatable measured cost above the observed noise here; negative
changes are not claimed as speedups. Self-source calibration was noisy (18%);
its -5.8% paired result does not establish a benefit. The unique-name workload
also showed 7.9% calibration variation. These limits preclude small-effect claims.

All predeclared envelopes passed: 4x repeated input took 3.9902x median latency
(limit 6x); decoding work stayed below `2 * source bytes + 64`; RSS stayed below
`4 * source bytes + 32 MiB`. Repeated-name identifier storage stayed at 618 bytes
for both sizes. No generated-program optimization, work or growth claim applies.

| Workload | Tokens incl. EOF | Decoded units | Copied spelling bytes | Unique IDs | ID storage bytes | Table storage growth events |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| repeated-8 | 4767572 | 8388511 | 0 | 14 | 618 | 11 |
| repeated-32 | 19070443 | 33554322 | 0 | 14 | 618 | 11 |
| translations-8 | 1417789 | 7679685 | 827043 | 3 | 170 | 5 |
| self-source-8 | 2864689 | 8364960 | 0 | 238 | 11170 | 24 |
| unique-200000 | 400002 | 2288891 | 0 | 200000 | 11370358 | 54 |

The repeated inputs cover template syntax, loops, calls, memory indexing and
floating-point spellings; those are lexical workloads only. Self-source input
concatenates the PA1 entry point and shared implementation headers/sources.
Translation input mixes UCNs, UTF-8, splices, comments and raw literals. The
unique-name input checks the geometric growth of the TU identifier table.

## Frozen inputs and equivalent outputs

| Workload | Input SHA-256 | A/B output SHA-256 |
| --- | --- | --- |
| repeated-8 | `459d7af709027a678fa0c2ed27b86ba51abb340ecbdf366cf5bf4266de65697a` | `450ef8eb52b9098983995289322fa7a3bd6e55e94745e660ad524acf74be282b` |
| repeated-32 | `7719804e57fa7d3127ad47fc3cfeb98ae91eb55d9e3c8e716ce16ba1f4299e9a` | `230812a1ab8149a665a388c1ef9a657460112016560f0217848c3920d460f854` |
| translations-8 | `b7481e1da8429d9a50c8f98d5d9a80644c2032b44277ec66bcd789ed188c5b45` | `3bcb87f0de344e0dc80044a825976c6c29004b4e4034faeb288991c090caada8` |
| self-source-8 | `b0c15a6bbd2f156c9d5aec65f29e6c3414efb9b0515c744d6f30a0466eb16c42` | `05ef6ab33805328a33d6271a57f7c5f7d98b586c23584788f9bff07231735c51` |
| unique-200000 | `d95badc2ff7c09100d90c66097e21d7731a2088ab0a739d28c54a04735af07d1` | `2fcc157f41436c7eda2815513c3a52b1cbe85a46e6c9a9574f48926fef14d61b` |

## All wall-time/RSS observations

Each cell is `seconds / peak KiB`. Samples 0-3 are A/A calibration; samples
4-7 and 8-11 are ABBA blocks. No samples were discarded.

| Sample / mode | repeated-8 | repeated-32 | translations-8 | self-source-8 | unique-200000 |
| --- | --- | --- | --- | --- | --- |
| 0 / A | 1.485485385 / 12092 | 6.004854228 / 36668 | 0.649554305 / 12092 | 1.291414311 / 12088 | 0.219046384 / 19808 |
| 1 / A | 1.509677934 / 12084 | 5.982773704 / 36488 | 0.651575050 / 12096 | 1.058434084 / 11968 | 0.214474785 / 19836 |
| 2 / A | 1.490051111 / 12048 | 5.994785496 / 36664 | 0.638405715 / 11852 | 1.066981846 / 12088 | 0.218468237 / 19820 |
| 3 / A | 1.499954364 / 12064 | 5.963829665 / 36484 | 0.645720997 / 11908 | 1.039228768 / 12092 | 0.235715835 / 19672 |
| 4 / A | 1.493618971 / 12096 | 5.980677614 / 36528 | 0.648646218 / 12048 | 1.257020032 / 12072 | 0.222804960 / 19836 |
| 5 / B | 1.493224901 / 11956 | 5.981534028 / 36644 | 0.645059165 / 12096 | 1.115104514 / 12092 | 0.218300704 / 19836 |
| 6 / B | 1.493552731 / 11912 | 5.956214006 / 36636 | 0.646583764 / 11952 | 1.078079424 / 12088 | 0.213934460 / 19836 |
| 7 / A | 1.500387932 / 12088 | 6.015825325 / 36268 | 0.642922214 / 11964 | 1.071793328 / 11964 | 0.211729504 / 19748 |
| 8 / A | 1.509029041 / 11924 | 5.981726733 / 36668 | 0.646022737 / 12088 | 1.052848340 / 12076 | 0.217370054 / 19836 |
| 9 / B | 1.500398735 / 12072 | 5.992292643 / 36624 | 0.654196236 / 12068 | 1.063682863 / 12044 | 0.222598584 / 19924 |
| 10 / B | 1.498215494 / 11924 | 6.039817208 / 36652 | 0.658675199 / 12056 | 1.050438985 / 12088 | 0.214166775 / 19952 |
| 11 / A | 1.506369284 / 12092 | 6.040731019 / 36528 | 0.647732260 / 11952 | 1.059578884 / 11912 | 0.222384798 / 19792 |

Reproduce with `python3 student.tests/pa1/benchmark.py`. The complete machine
manifest, source hashes, JSON observations (including per-run phase counters),
input files and frozen binary remain in
`obj/student-pa1/performance-20260907-075606/`. Generated artifacts are untracked.
No baseline comparison against the incorrect PA1 stub was made.
