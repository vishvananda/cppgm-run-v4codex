# PA4 frozen compiler performance evidence

A: first complete implementation (`28279a9d0`); B: `1af70fc0d` (indexed argument slices, explicit prescan tasks and reusable spelling/scratch storage).

Both use the ordinary `g++ -std=gnu++11 -Wall -O3` build with the same course runner. Fresh processes, serial measurements, output to a temporary file, wall time including process startup and peak RSS from GNU time. Each workload has two A/A pairs, one B/B pair, two ABBA blocks and a separate B telemetry sample. All observations are below. Every output agrees within its workload. Temporary source paths affect output hashes, but are identical across each A/B group.

Platform: `Linux-7.0.0-1005-gcp-x86_64-with-glibc2.43`. Host: `g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0`.

Frozen SHA256 A: `b83ce0f7d97af3cbf4944fb1389d1d0dc129011ad773186a494d6c91c8054b00`; B: `15b6ea60b9d6722dca2e340f08ec8ba39fc6b13dbab54a18a85071108e9e7dfa`.

Generated executable runtime / generated text size: **N/A at PA4**. Host-tool text (GNU size, including read-only data): 160491 → 171378 bytes (6.78%).

Budgets fixed in pa4/plan.md: unaffected paired latency <=10% plus measured noise; RSS <=15% plus 1 MiB; host text growth <=15%; 4x flat input <6x wall time and <5x RSS. Indexed nesting captures <=3n tokens; generated spelling arena <=128 KiB. No runtime optimization or generated-code benefit is claimed.

| Workload | A median s | B median s | ABBA deltas % | A/A or B/B noise % | A RSS KiB | B RSS KiB |
| --- | ---: | ---: | --- | ---: | ---: | ---: |
| flat-4MiB | 0.744355 | 0.795341 | 5.65, 12.08 | 5.64% | 11456 | 11378 |
| flat-16MiB | 2.947870 | 3.152966 | 7.10, 7.16 | 3.56% | 34624 | 34624 |
| ordinary-C++ | 0.549396 | 0.574176 | 4.40, 4.42 | 2.10% | 7584 | 7686 |
| macro-reuse | 0.574332 | 0.630549 | 9.65, 9.76 | 0.69% | 4804 | 4692 |
| nested-arguments | 5.860740 | 0.109026 | -98.14, -98.14 | 0.49% | 137092 | 4564 |
| long-chain-reuse | 0.492197 | 0.464521 | -5.80, -4.78 | 0.32% | 10438 | 10508 |
| counter-spellings | 0.419685 | 0.439046 | 4.23, 4.66 | 0.70% | 8900 | 7694 |
| literal-locations | 0.222739 | 0.230128 | 6.93, 3.54 | 6.18% | 5666 | 5682 |

## Manifests and telemetry

```json
[
  {
    "workload": "flat-4MiB",
    "bytes": 4194300,
    "input_sha256": "839f7d06615aebede01ae4ee50b219648cc4a2bf9a710beae9dfe74ae4a8b7c0",
    "output_sha256": "13b47f0df42825698aa1cc95f15b5ca405fa2ea577ba4f9b1ad2d3e2511ea4b8",
    "stats": {
      "preprocess_post_emit_ms": 777.944,
      "peak_rss_kib": 11408,
      "source_bytes": 4194300,
      "files": 1,
      "directives": 0,
      "invocations": 0,
      "argument_prescans": 0,
      "replacement_tokens": 0,
      "output_tokens": 1398100,
      "paste_bytes": 0,
      "arena_bytes": 131072,
      "context_nodes": 0,
      "max_pending": 0,
      "lex_tokens": 2446676,
      "captured_tokens": 0,
      "borrowed_arguments": 0,
      "max_prescan_depth": 0,
      "max_context_nodes": 0,
      "scratch_growths": 0,
      "identifiers": 12,
      "identifier_storage_bytes": 674
    }
  },
  {
    "workload": "flat-16MiB",
    "bytes": 16777212,
    "input_sha256": "25d66aba4b4fc9e3bbc0609785322c6c5e46abef589d133883796b5cffb18e71",
    "output_sha256": "e99fe1315ec41eb0f585aad3b734c6c8c7f40cb61d78e4020e9b394f54ab900d",
    "stats": {
      "preprocess_post_emit_ms": 3129.32,
      "peak_rss_kib": 34380,
      "source_bytes": 16777212,
      "files": 1,
      "directives": 0,
      "invocations": 0,
      "argument_prescans": 0,
      "replacement_tokens": 0,
      "output_tokens": 5592404,
      "paste_bytes": 0,
      "arena_bytes": 131072,
      "context_nodes": 0,
      "max_pending": 0,
      "lex_tokens": 9786708,
      "captured_tokens": 0,
      "borrowed_arguments": 0,
      "max_prescan_depth": 0,
      "max_context_nodes": 0,
      "scratch_growths": 0,
      "identifiers": 12,
      "identifier_storage_bytes": 674
    }
  },
  {
    "workload": "ordinary-C++",
    "bytes": 2592000,
    "input_sha256": "9420f4ef2269b65da14dc0ba7d9ddda577844a73c512f1594fa8ea3606a1abec",
    "output_sha256": "2df01574fa3d246b74879e2fe67fc8505b9d5023d0178273e5b2e438cb6a4e0a",
    "stats": {
      "preprocess_post_emit_ms": 571.304,
      "peak_rss_kib": 7680,
      "source_bytes": 2592000,
      "files": 1,
      "directives": 0,
      "invocations": 0,
      "argument_prescans": 0,
      "replacement_tokens": 0,
      "output_tokens": 1072000,
      "paste_bytes": 0,
      "arena_bytes": 131072,
      "context_nodes": 0,
      "max_pending": 0,
      "lex_tokens": 1536001,
      "captured_tokens": 0,
      "borrowed_arguments": 0,
      "max_prescan_depth": 0,
      "max_context_nodes": 0,
      "scratch_growths": 0,
      "identifiers": 28,
      "identifier_storage_bytes": 1340
    }
  },
  {
    "workload": "macro-reuse",
    "bytes": 750021,
    "input_sha256": "d43d80a2c9e8e4feceb80891609c70868ea93f2c6a95d04cef1f1ccb7eee394f",
    "output_sha256": "692c8f2177a3207c4c42ab04465d31132f405d78342184fcb5b7c5bda21cd4c8",
    "stats": {
      "preprocess_post_emit_ms": 621.762,
      "peak_rss_kib": 4580,
      "source_bytes": 750021,
      "files": 1,
      "directives": 1,
      "invocations": 150000,
      "argument_prescans": 150000,
      "replacement_tokens": 1050000,
      "output_tokens": 1050000,
      "paste_bytes": 0,
      "arena_bytes": 131072,
      "context_nodes": 4800000,
      "max_pending": 7,
      "lex_tokens": 750018,
      "captured_tokens": 450000,
      "borrowed_arguments": 0,
      "max_prescan_depth": 1,
      "max_context_nodes": 34,
      "scratch_growths": 5,
      "identifiers": 14,
      "identifier_storage_bytes": 826
    }
  },
  {
    "workload": "nested-arguments",
    "bytes": 230799,
    "input_sha256": "eec04db3cab0c61c916ffa5f177a5063fdfcc7ca8c5af6151e59612ead63f22f",
    "output_sha256": "4bad4b8075e87886d8bfa6408b264fba40a3f336527a7319fed4fcd2fa02160a",
    "stats": {
      "preprocess_post_emit_ms": 104.366,
      "peak_rss_kib": 4540,
      "source_bytes": 230799,
      "files": 1,
      "directives": 1,
      "invocations": 76800,
      "argument_prescans": 76800,
      "replacement_tokens": 76800,
      "output_tokens": 128,
      "paste_bytes": 0,
      "arena_bytes": 131072,
      "context_nodes": 2457600,
      "max_pending": 1,
      "lex_tokens": 230667,
      "captured_tokens": 230400,
      "borrowed_arguments": 76672,
      "max_prescan_depth": 600,
      "max_context_nodes": 19202,
      "scratch_growths": 153346,
      "identifiers": 14,
      "identifier_storage_bytes": 836
    }
  },
  {
    "workload": "long-chain-reuse",
    "bytes": 190647,
    "input_sha256": "41d6fde73b4343b2e4aa937120faf11d7bf6c69bbb983882f8c65ddd2515a889",
    "output_sha256": "795cc5e3da35987377d451490ec1b5e3de5114c1b39166ed3a5c197c60c19fc8",
    "stats": {
      "preprocess_post_emit_ms": 466.803,
      "peak_rss_kib": 10724,
      "source_bytes": 190647,
      "files": 1,
      "directives": 8002,
      "invocations": 512128,
      "argument_prescans": 64,
      "replacement_tokens": 1540160,
      "output_tokens": 4096,
      "paste_bytes": 0,
      "arena_bytes": 131072,
      "context_nodes": 16388096,
      "max_pending": 64,
      "lex_tokens": 88595,
      "captured_tokens": 320,
      "borrowed_arguments": 0,
      "max_prescan_depth": 1,
      "max_context_nodes": 256066,
      "scratch_growths": 264,
      "identifiers": 8016,
      "identifier_storage_bytes": 303348
    }
  },
  {
    "workload": "counter-spellings",
    "bytes": 3600000,
    "input_sha256": "454ff659f5023a54922fc5212cef8c4d0d5d55c70491724da8fa8249ab340faa",
    "output_sha256": "9138d81d63f7f87f63da2d8aa568c7114eba1a046748d6408186b4cbf72d32c9",
    "stats": {
      "preprocess_post_emit_ms": 474.593,
      "peak_rss_kib": 7692,
      "source_bytes": 3600000,
      "files": 1,
      "directives": 0,
      "invocations": 300000,
      "argument_prescans": 0,
      "replacement_tokens": 0,
      "output_tokens": 300000,
      "paste_bytes": 0,
      "arena_bytes": 131072,
      "context_nodes": 0,
      "max_pending": 0,
      "lex_tokens": 600002,
      "captured_tokens": 0,
      "borrowed_arguments": 0,
      "max_prescan_depth": 0,
      "max_context_nodes": 0,
      "scratch_growths": 0,
      "identifiers": 11,
      "identifier_storage_bytes": 738
    }
  },
  {
    "workload": "literal-locations",
    "bytes": 1480047,
    "input_sha256": "de53addfe427adccb6f943334ae2e9e5e3ba24efa2bc7b64fe382a991416a082",
    "output_sha256": "e81199c89ef1de7e1721fb743e9940fdad6c8b3a5c10e2055185c6681dd02915",
    "stats": {
      "preprocess_post_emit_ms": 221.155,
      "peak_rss_kib": 5448,
      "source_bytes": 1480047,
      "files": 1,
      "directives": 2,
      "invocations": 40000,
      "argument_prescans": 0,
      "replacement_tokens": 40000,
      "output_tokens": 240000,
      "paste_bytes": 0,
      "arena_bytes": 131072,
      "context_nodes": 1280000,
      "max_pending": 2,
      "lex_tokens": 400015,
      "captured_tokens": 0,
      "borrowed_arguments": 0,
      "max_prescan_depth": 0,
      "max_context_nodes": 34,
      "scratch_growths": 3,
      "identifiers": 18,
      "identifier_storage_bytes": 1250
    }
  }
]
```

## Every observation

| Workload | Block | Binary | Mode | Wall seconds | Peak RSS KiB |
| --- | --- | --- | --- | ---: | ---: |
| flat-4MiB | calibration | A | ordinary | 0.736144 | 11460 |
| flat-4MiB | calibration | A | ordinary | 0.777626 | 11272 |
| flat-4MiB | calibration | A | ordinary | 0.767164 | 11236 |
| flat-4MiB | calibration | A | ordinary | 0.742486 | 11476 |
| flat-4MiB | calibration | B | ordinary | 0.796013 | 11252 |
| flat-4MiB | calibration | B | ordinary | 0.794670 | 11480 |
| flat-4MiB | 1 | A | ordinary | 0.747179 | 11232 |
| flat-4MiB | 1 | B | ordinary | 0.791362 | 11252 |
| flat-4MiB | 1 | B | ordinary | 0.786118 | 11276 |
| flat-4MiB | 1 | A | ordinary | 0.745896 | 11480 |
| flat-4MiB | 2 | A | ordinary | 0.734898 | 11452 |
| flat-4MiB | 2 | B | ordinary | 0.852547 | 11484 |
| flat-4MiB | 2 | B | ordinary | 0.803609 | 11480 |
| flat-4MiB | 2 | A | ordinary | 0.742813 | 11476 |
| flat-4MiB | telemetry | B | stats | 0.787971 | 11408 |
| flat-16MiB | calibration | A | ordinary | 3.050877 | 34640 |
| flat-16MiB | calibration | A | ordinary | 2.943831 | 34404 |
| flat-16MiB | calibration | A | ordinary | 2.951910 | 34648 |
| flat-16MiB | calibration | A | ordinary | 2.953137 | 34644 |
| flat-16MiB | calibration | B | ordinary | 3.150139 | 34628 |
| flat-16MiB | calibration | B | ordinary | 3.262254 | 34620 |
| flat-16MiB | 1 | A | ordinary | 2.954895 | 34408 |
| flat-16MiB | 1 | B | ordinary | 3.168516 | 34636 |
| flat-16MiB | 1 | B | ordinary | 3.147841 | 34412 |
| flat-16MiB | 1 | A | ordinary | 2.942926 | 34616 |
| flat-16MiB | 2 | A | ordinary | 2.929941 | 34420 |
| flat-16MiB | 2 | B | ordinary | 3.155793 | 34648 |
| flat-16MiB | 2 | B | ordinary | 3.135527 | 34620 |
| flat-16MiB | 2 | A | ordinary | 2.941030 | 34632 |
| flat-16MiB | telemetry | B | stats | 3.151774 | 34380 |
| ordinary-C++ | calibration | A | ordinary | 0.560786 | 7476 |
| ordinary-C++ | calibration | A | ordinary | 0.549020 | 7460 |
| ordinary-C++ | calibration | A | ordinary | 0.549771 | 7696 |
| ordinary-C++ | calibration | A | ordinary | 0.546971 | 7476 |
| ordinary-C++ | calibration | B | ordinary | 0.585350 | 7448 |
| ordinary-C++ | calibration | B | ordinary | 0.574666 | 7692 |
| ordinary-C++ | 1 | A | ordinary | 0.552708 | 7704 |
| ordinary-C++ | 1 | B | ordinary | 0.570521 | 7692 |
| ordinary-C++ | 1 | B | ordinary | 0.574612 | 7464 |
| ordinary-C++ | 1 | A | ordinary | 0.544188 | 7464 |
| ordinary-C++ | 2 | A | ordinary | 0.548552 | 7692 |
| ordinary-C++ | 2 | B | ordinary | 0.573525 | 7692 |
| ordinary-C++ | 2 | B | ordinary | 0.573740 | 7680 |
| ordinary-C++ | 2 | A | ordinary | 0.550117 | 7708 |
| ordinary-C++ | telemetry | B | stats | 0.579628 | 7680 |
| macro-reuse | calibration | A | ordinary | 0.573442 | 4812 |
| macro-reuse | calibration | A | ordinary | 0.577406 | 4596 |
| macro-reuse | calibration | A | ordinary | 0.573268 | 4824 |
| macro-reuse | calibration | A | ordinary | 0.571546 | 4824 |
| macro-reuse | calibration | B | ordinary | 0.630875 | 4796 |
| macro-reuse | calibration | B | ordinary | 0.630223 | 4588 |
| macro-reuse | 1 | A | ordinary | 0.574625 | 4552 |
| macro-reuse | 1 | B | ordinary | 0.631002 | 4820 |
| macro-reuse | 1 | B | ordinary | 0.628846 | 4580 |
| macro-reuse | 1 | A | ordinary | 0.574370 | 4824 |
| macro-reuse | 2 | A | ordinary | 0.577091 | 4796 |
| macro-reuse | 2 | B | ordinary | 0.628588 | 4812 |
| macro-reuse | 2 | B | ordinary | 0.635140 | 4580 |
| macro-reuse | 2 | A | ordinary | 0.574295 | 4580 |
| macro-reuse | telemetry | B | stats | 0.630241 | 4676 |
| nested-arguments | calibration | A | ordinary | 5.844457 | 137080 |
| nested-arguments | calibration | A | ordinary | 5.833425 | 137108 |
| nested-arguments | calibration | A | ordinary | 5.855666 | 137160 |
| nested-arguments | calibration | A | ordinary | 5.868496 | 137076 |
| nested-arguments | calibration | B | ordinary | 0.109166 | 4328 |
| nested-arguments | calibration | B | ordinary | 0.109696 | 4568 |
| nested-arguments | 1 | A | ordinary | 5.865814 | 136872 |
| nested-arguments | 1 | B | ordinary | 0.108858 | 4564 |
| nested-arguments | 1 | B | ordinary | 0.109465 | 4568 |
| nested-arguments | 1 | A | ordinary | 5.896554 | 137104 |
| nested-arguments | 2 | A | ordinary | 5.870542 | 137104 |
| nested-arguments | 2 | B | ordinary | 0.108885 | 4564 |
| nested-arguments | 2 | B | ordinary | 0.108153 | 4564 |
| nested-arguments | 2 | A | ordinary | 5.822351 | 137076 |
| nested-arguments | telemetry | B | stats | 0.109415 | 4540 |
| long-chain-reuse | calibration | A | ordinary | 0.494947 | 10212 |
| long-chain-reuse | calibration | A | ordinary | 0.493608 | 10456 |
| long-chain-reuse | calibration | A | ordinary | 0.492543 | 10216 |
| long-chain-reuse | calibration | A | ordinary | 0.490986 | 10448 |
| long-chain-reuse | calibration | B | ordinary | 0.463403 | 10472 |
| long-chain-reuse | calibration | B | ordinary | 0.462833 | 10720 |
| long-chain-reuse | 1 | A | ordinary | 0.495836 | 10252 |
| long-chain-reuse | 1 | B | ordinary | 0.463596 | 10500 |
| long-chain-reuse | 1 | B | ordinary | 0.465446 | 10540 |
| long-chain-reuse | 1 | A | ordinary | 0.490408 | 10432 |
| long-chain-reuse | 2 | A | ordinary | 0.491406 | 10448 |
| long-chain-reuse | 2 | B | ordinary | 0.467728 | 10224 |
| long-chain-reuse | 2 | B | ordinary | 0.468554 | 10516 |
| long-chain-reuse | 2 | A | ordinary | 0.491850 | 10444 |
| long-chain-reuse | telemetry | B | stats | 0.473177 | 10724 |
| counter-spellings | calibration | A | ordinary | 0.417101 | 8904 |
| counter-spellings | calibration | A | ordinary | 0.414199 | 8908 |
| counter-spellings | calibration | A | ordinary | 0.431342 | 8684 |
| counter-spellings | calibration | A | ordinary | 0.428995 | 8684 |
| counter-spellings | calibration | B | ordinary | 0.443485 | 7476 |
| counter-spellings | calibration | B | ordinary | 0.441364 | 7708 |
| counter-spellings | 1 | A | ordinary | 0.422270 | 8916 |
| counter-spellings | 1 | B | ordinary | 0.435447 | 7712 |
| counter-spellings | 1 | B | ordinary | 0.436728 | 7680 |
| counter-spellings | 1 | A | ordinary | 0.414544 | 8712 |
| counter-spellings | 2 | A | ordinary | 0.424148 | 8896 |
| counter-spellings | 2 | B | ordinary | 0.434059 | 7460 |
| counter-spellings | 2 | B | ordinary | 0.444168 | 7708 |
| counter-spellings | 2 | A | ordinary | 0.414950 | 8920 |
| counter-spellings | telemetry | B | stats | 0.481355 | 7692 |
| literal-locations | calibration | A | ordinary | 0.222830 | 5688 |
| literal-locations | calibration | A | ordinary | 0.221971 | 5648 |
| literal-locations | calibration | A | ordinary | 0.227516 | 5440 |
| literal-locations | calibration | A | ordinary | 0.222257 | 5688 |
| literal-locations | calibration | B | ordinary | 0.225223 | 5464 |
| literal-locations | calibration | B | ordinary | 0.239143 | 5688 |
| literal-locations | 1 | A | ordinary | 0.222647 | 5688 |
| literal-locations | 1 | B | ordinary | 0.227471 | 5688 |
| literal-locations | 1 | B | ordinary | 0.249095 | 5676 |
| literal-locations | 1 | A | ordinary | 0.223035 | 5684 |
| literal-locations | 2 | A | ordinary | 0.223913 | 5472 |
| literal-locations | 2 | B | ordinary | 0.229187 | 5460 |
| literal-locations | 2 | B | ordinary | 0.231070 | 5688 |
| literal-locations | 2 | A | ordinary | 0.220587 | 5460 |
| literal-locations | telemetry | B | stats | 0.227528 | 5448 |

Output SHA256 is recorded once per workload above; every sample was checked against it.

## Startup calibration

Eight fresh final-B processes on one empty UTF-8 source, with the same GNU-time/output-file command and binary hash as above. Every output agrees. All observations:

```json
[
  {
    "seconds": 0.004711652174592018,
    "rss_kib": 3524,
    "output_sha256": "ad3bb49d9edc1b5c0124bf75d463fc278bf1105ca60786cea28fc9b7de55b964"
  },
  {
    "seconds": 0.004665793851017952,
    "rss_kib": 3544,
    "output_sha256": "ad3bb49d9edc1b5c0124bf75d463fc278bf1105ca60786cea28fc9b7de55b964"
  },
  {
    "seconds": 0.004630683921277523,
    "rss_kib": 3540,
    "output_sha256": "ad3bb49d9edc1b5c0124bf75d463fc278bf1105ca60786cea28fc9b7de55b964"
  },
  {
    "seconds": 0.004343844018876553,
    "rss_kib": 3552,
    "output_sha256": "ad3bb49d9edc1b5c0124bf75d463fc278bf1105ca60786cea28fc9b7de55b964"
  },
  {
    "seconds": 0.004123060964047909,
    "rss_kib": 3568,
    "output_sha256": "ad3bb49d9edc1b5c0124bf75d463fc278bf1105ca60786cea28fc9b7de55b964"
  },
  {
    "seconds": 0.0040969569236040115,
    "rss_kib": 3544,
    "output_sha256": "ad3bb49d9edc1b5c0124bf75d463fc278bf1105ca60786cea28fc9b7de55b964"
  },
  {
    "seconds": 0.004039487801492214,
    "rss_kib": 3540,
    "output_sha256": "ad3bb49d9edc1b5c0124bf75d463fc278bf1105ca60786cea28fc9b7de55b964"
  },
  {
    "seconds": 0.0041516125202178955,
    "rss_kib": 3544,
    "output_sha256": "ad3bb49d9edc1b5c0124bf75d463fc278bf1105ca60786cea28fc9b7de55b964"
  }
]
```

Median startup is 0.004248 s; the fastest final workload median (0.109026 s) is 25.7x larger. Workload timings dominate process startup. This is compiler execution, not a generated-program runtime.

## Acceptance and limitations

The saved observations and actual final binary hash pass
`python3 student.tests/pa4/verify_performance.py student.tests/pa4/performance.md dev/preproc`.
The 4x flat-input ratios are 3.9643x compiler wall time and 3.0431x peak RSS.
Nested argument processing improves 98.14% in both blocks; its compiler RSS
falls from 137092 to 4564 KiB. Long chains improve 4.78–5.80%. Host-tool text
increases 6.78%, below the predeclared 15% budget.

The ordinary compiler regressions are real and retained: the repeated-argument
case is +9.65–9.76%; other ordinary paired deltas are +3.54–7.16%, apart from
one +12.08% flat-input block. That workload's 5.64% calibration noise gives a
15.64% predeclared acceptance limit. Every individual paired block and RSS
budget passes; the nested benefit is far larger than calibration noise.
The initial and parameterless-candidate campaigns are preserved separately,
including the long-chain regression that led to the parameterless fast path.

This evidence establishes bounded, faster nested preprocessing and discloses
its cost elsewhere. It establishes no semantic-template, native-code,
self-hosting, generated-runtime or generated-text-size improvement; those
measurements belong to later assignments with executable output.
