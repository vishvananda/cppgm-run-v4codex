# PA4 frozen compiler performance evidence

A: first complete implementation (`28279a9d0`); B: `957b47c37` (indexed argument slices, explicit prescan tasks and reusable spelling/scratch storage).

Both use the ordinary `g++ -std=gnu++11 -Wall -O3` build with the same course runner. Fresh processes, serial measurements, output to a temporary file, wall time including process startup and peak RSS from GNU time. Each workload has two A/A pairs, one B/B pair, two ABBA blocks and a separate B telemetry sample. All observations are below. Every output agrees within its workload. Temporary source paths affect output hashes, but are identical across each A/B group.

Platform: `Linux-7.0.0-1005-gcp-x86_64-with-glibc2.43`. Host: `g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0`.

Frozen SHA256 A: `b83ce0f7d97af3cbf4944fb1389d1d0dc129011ad773186a494d6c91c8054b00`; B: `a3b823c1718cec4b18af573355238d484b9cd0996eb1efcfdd30e7938acc4ad0`.

Generated executable runtime / generated text size: **N/A at PA4**. Host-tool text (GNU size, including read-only data): 160491 → 171058 bytes (6.58%).

Budgets fixed in pa4/plan.md: unaffected paired latency <=10% plus measured noise; RSS <=15% plus 1 MiB; host text growth <=15%; 4x flat input <6x wall time and <5x RSS. Indexed nesting captures <=3n tokens; generated spelling arena <=128 KiB. No runtime optimization or generated-code benefit is claimed.

| Workload | A median s | B median s | ABBA deltas % | A/A or B/B noise % | A RSS KiB | B RSS KiB |
| --- | ---: | ---: | --- | ---: | ---: | ---: |
| flat-4MiB | 0.747970 | 0.787239 | 4.83, 6.70 | 0.53% | 11478 | 11346 |
| flat-16MiB | 2.932705 | 3.132644 | 6.17, 4.81 | 0.33% | 34540 | 34636 |
| ordinary-C++ | 0.550768 | 0.574702 | 2.86, 4.54 | 5.62% | 7700 | 7704 |
| macro-reuse | 0.576311 | 0.625324 | 8.50, 7.03 | 1.55% | 4822 | 4806 |
| nested-arguments | 5.904823 | 0.109711 | -98.11, -98.16 | 0.91% | 137100 | 4564 |
| long-chain-reuse | 0.501224 | 0.467768 | -6.67, -6.41 | 2.46% | 10440 | 10706 |
| counter-spellings | 0.417079 | 0.437759 | 2.47, 4.57 | 4.39% | 8908 | 7704 |

## Manifests and telemetry

```json
[
  {
    "workload": "flat-4MiB",
    "bytes": 4194300,
    "input_sha256": "839f7d06615aebede01ae4ee50b219648cc4a2bf9a710beae9dfe74ae4a8b7c0",
    "output_sha256": "e357d2209eb5cf9ec7644003c857a89131209d32458b1404a995b55b2bd17388",
    "stats": {
      "preprocess_post_emit_ms": 774.391,
      "peak_rss_kib": 11476,
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
    "output_sha256": "520114f7df0e2e9ae9f1f7d48e97763ed74a1ac5467b83699ef18ee4ba37f41e",
    "stats": {
      "preprocess_post_emit_ms": 3176.57,
      "peak_rss_kib": 34648,
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
    "output_sha256": "8181e04f48b36da79e980beda5af1f5b80a022acf9bba02a3c3ecc504fadc3af",
    "stats": {
      "preprocess_post_emit_ms": 568.814,
      "peak_rss_kib": 7460,
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
    "output_sha256": "e6daf9d49ad0856aa49fc47af4eeb9200a284e8c536069bac8c1b0a7c0c1ca51",
    "stats": {
      "preprocess_post_emit_ms": 617.504,
      "peak_rss_kib": 4800,
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
    "output_sha256": "b8fbbb42a1d3be2acb21cbeade55a74b0eacecf3870d6c6214b7665d2ee2bcfe",
    "stats": {
      "preprocess_post_emit_ms": 108.296,
      "peak_rss_kib": 4568,
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
    "output_sha256": "8b04ed49c21634f68bee2de6b48240302b26322009f1bda9085af99766dc315f",
    "stats": {
      "preprocess_post_emit_ms": 466.894,
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
    "output_sha256": "8c35edfe8c0d28cc7d328474ae876e8d6a56e1171cfdb2246aaf013acf09d190",
    "stats": {
      "preprocess_post_emit_ms": 435.381,
      "peak_rss_kib": 7676,
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
  }
]
```

## Every observation

| Workload | Block | Binary | Mode | Wall seconds | Peak RSS KiB |
| --- | --- | --- | --- | ---: | ---: |
| flat-4MiB | calibration | A | ordinary | 0.746092 | 11480 |
| flat-4MiB | calibration | A | ordinary | 0.749848 | 11480 |
| flat-4MiB | calibration | A | ordinary | 0.763081 | 11480 |
| flat-4MiB | calibration | A | ordinary | 0.767146 | 11464 |
| flat-4MiB | calibration | B | ordinary | 0.789148 | 11480 |
| flat-4MiB | calibration | B | ordinary | 0.786433 | 11240 |
| flat-4MiB | 1 | A | ordinary | 0.765218 | 11480 |
| flat-4MiB | 1 | B | ordinary | 0.787944 | 11236 |
| flat-4MiB | 1 | B | ordinary | 0.791198 | 11480 |
| flat-4MiB | 1 | A | ordinary | 0.741179 | 11476 |
| flat-4MiB | 2 | A | ordinary | 0.736862 | 11468 |
| flat-4MiB | 2 | B | ordinary | 0.786533 | 11452 |
| flat-4MiB | 2 | B | ordinary | 0.785487 | 11240 |
| flat-4MiB | 2 | A | ordinary | 0.736455 | 11248 |
| flat-4MiB | telemetry | B | stats | 0.783550 | 11476 |
| flat-16MiB | calibration | A | ordinary | 2.925934 | 34400 |
| flat-16MiB | calibration | A | ordinary | 2.926877 | 34644 |
| flat-16MiB | calibration | A | ordinary | 2.937507 | 34676 |
| flat-16MiB | calibration | A | ordinary | 2.927904 | 34436 |
| flat-16MiB | calibration | B | ordinary | 3.151612 | 34648 |
| flat-16MiB | calibration | B | ordinary | 3.143804 | 34652 |
| flat-16MiB | 1 | A | ordinary | 2.953310 | 34416 |
| flat-16MiB | 1 | B | ordinary | 3.123581 | 34652 |
| flat-16MiB | 1 | B | ordinary | 3.117589 | 34624 |
| flat-16MiB | 1 | A | ordinary | 2.925407 | 34644 |
| flat-16MiB | 2 | A | ordinary | 3.019463 | 34400 |
| flat-16MiB | 2 | B | ordinary | 3.141708 | 34368 |
| flat-16MiB | 2 | B | ordinary | 3.115686 | 34404 |
| flat-16MiB | 2 | A | ordinary | 2.950547 | 34644 |
| flat-16MiB | telemetry | B | stats | 3.199486 | 34648 |
| ordinary-C++ | calibration | A | ordinary | 0.563357 | 7700 |
| ordinary-C++ | calibration | A | ordinary | 0.550654 | 7700 |
| ordinary-C++ | calibration | A | ordinary | 0.548580 | 7692 |
| ordinary-C++ | calibration | A | ordinary | 0.579386 | 7472 |
| ordinary-C++ | calibration | B | ordinary | 0.574702 | 7704 |
| ordinary-C++ | calibration | B | ordinary | 0.568313 | 7708 |
| ordinary-C++ | 1 | A | ordinary | 0.577461 | 7704 |
| ordinary-C++ | 1 | B | ordinary | 0.572836 | 7704 |
| ordinary-C++ | 1 | B | ordinary | 0.585593 | 7680 |
| ordinary-C++ | 1 | A | ordinary | 0.548811 | 7700 |
| ordinary-C++ | 2 | A | ordinary | 0.550883 | 7700 |
| ordinary-C++ | 2 | B | ordinary | 0.574701 | 7460 |
| ordinary-C++ | 2 | B | ordinary | 0.575858 | 7708 |
| ordinary-C++ | 2 | A | ordinary | 0.549757 | 7680 |
| ordinary-C++ | telemetry | B | stats | 0.577199 | 7460 |
| macro-reuse | calibration | A | ordinary | 0.582650 | 4824 |
| macro-reuse | calibration | A | ordinary | 0.576608 | 4824 |
| macro-reuse | calibration | A | ordinary | 0.576015 | 4588 |
| macro-reuse | calibration | A | ordinary | 0.575654 | 4824 |
| macro-reuse | calibration | B | ordinary | 0.633664 | 4568 |
| macro-reuse | calibration | B | ordinary | 0.623836 | 4812 |
| macro-reuse | 1 | A | ordinary | 0.575738 | 4612 |
| macro-reuse | 1 | B | ordinary | 0.623680 | 4800 |
| macro-reuse | 1 | B | ordinary | 0.625810 | 4824 |
| macro-reuse | 1 | A | ordinary | 0.575886 | 4580 |
| macro-reuse | 2 | A | ordinary | 0.588430 | 4824 |
| macro-reuse | 2 | B | ordinary | 0.624839 | 4596 |
| macro-reuse | 2 | B | ordinary | 0.628084 | 4820 |
| macro-reuse | 2 | A | ordinary | 0.582228 | 4820 |
| macro-reuse | telemetry | B | stats | 0.625932 | 4800 |
| nested-arguments | calibration | A | ordinary | 5.882284 | 137132 |
| nested-arguments | calibration | A | ordinary | 5.903119 | 137104 |
| nested-arguments | calibration | A | ordinary | 5.930999 | 137084 |
| nested-arguments | calibration | A | ordinary | 5.906528 | 137092 |
| nested-arguments | calibration | B | ordinary | 0.109214 | 4568 |
| nested-arguments | calibration | B | ordinary | 0.110208 | 4324 |
| nested-arguments | 1 | A | ordinary | 5.879469 | 137108 |
| nested-arguments | 1 | B | ordinary | 0.110966 | 4568 |
| nested-arguments | 1 | B | ordinary | 0.111590 | 4560 |
| nested-arguments | 1 | A | ordinary | 5.915649 | 137096 |
| nested-arguments | 2 | A | ordinary | 5.910942 | 137108 |
| nested-arguments | 2 | B | ordinary | 0.108383 | 4568 |
| nested-arguments | 2 | B | ordinary | 0.109005 | 4544 |
| nested-arguments | 2 | A | ordinary | 5.874543 | 137080 |
| nested-arguments | telemetry | B | stats | 0.113668 | 4568 |
| long-chain-reuse | calibration | A | ordinary | 0.507042 | 10224 |
| long-chain-reuse | calibration | A | ordinary | 0.494552 | 10460 |
| long-chain-reuse | calibration | A | ordinary | 0.509670 | 10184 |
| long-chain-reuse | calibration | A | ordinary | 0.502400 | 10456 |
| long-chain-reuse | calibration | B | ordinary | 0.476782 | 10704 |
| long-chain-reuse | calibration | B | ordinary | 0.468857 | 10484 |
| long-chain-reuse | 1 | A | ordinary | 0.500469 | 10232 |
| long-chain-reuse | 1 | B | ordinary | 0.465436 | 10724 |
| long-chain-reuse | 1 | B | ordinary | 0.467409 | 10708 |
| long-chain-reuse | 1 | A | ordinary | 0.499095 | 10456 |
| long-chain-reuse | 2 | A | ordinary | 0.501978 | 10428 |
| long-chain-reuse | 2 | B | ordinary | 0.466433 | 10724 |
| long-chain-reuse | 2 | B | ordinary | 0.468127 | 10480 |
| long-chain-reuse | 2 | A | ordinary | 0.496572 | 10452 |
| long-chain-reuse | telemetry | B | stats | 0.473180 | 10724 |
| counter-spellings | calibration | A | ordinary | 0.415699 | 8900 |
| counter-spellings | calibration | A | ordinary | 0.415452 | 8920 |
| counter-spellings | calibration | A | ordinary | 0.422256 | 8676 |
| counter-spellings | calibration | A | ordinary | 0.416995 | 8924 |
| counter-spellings | calibration | B | ordinary | 0.440528 | 7704 |
| counter-spellings | calibration | B | ordinary | 0.459849 | 7704 |
| counter-spellings | 1 | A | ordinary | 0.426779 | 8672 |
| counter-spellings | 1 | B | ordinary | 0.435077 | 7712 |
| counter-spellings | 1 | B | ordinary | 0.429736 | 7708 |
| counter-spellings | 1 | A | ordinary | 0.417162 | 8892 |
| counter-spellings | 2 | A | ordinary | 0.417657 | 8916 |
| counter-spellings | 2 | B | ordinary | 0.440441 | 7704 |
| counter-spellings | 2 | B | ordinary | 0.431268 | 7684 |
| counter-spellings | 2 | A | ordinary | 0.415934 | 8916 |
| counter-spellings | telemetry | B | stats | 0.442737 | 7676 |

Output SHA256 is recorded once per workload above; every sample was checked against it.
