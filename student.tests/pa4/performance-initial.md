# PA4 frozen compiler performance evidence

A: first complete implementation (`28279a9d0`); B: indexed argument slices, explicit prescan tasks and reusable spelling/scratch storage.

Both use the ordinary `g++ -std=gnu++11 -Wall -O3` build with the same course runner. Fresh processes, serial measurements, output to a temporary file, wall time including process startup and peak RSS from GNU time. Each workload has two A/A pairs, one B/B pair, two ABBA blocks and a separate B telemetry sample. All observations are below. Every output agrees within its workload. Temporary source paths affect output hashes, but are identical across each A/B group.

Platform: `Linux-7.0.0-1005-gcp-x86_64-with-glibc2.43`. Host: `g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0`.

Frozen SHA256 A: `b83ce0f7d97af3cbf4944fb1389d1d0dc129011ad773186a494d6c91c8054b00`; B: `dfd444275c7a39e75771b4e56854a00a6af95de355f3f2e67dc3ea00f40290b8`.

Generated executable runtime / generated text size: **N/A at PA4**. Host-tool text (GNU size, including read-only data): 160491 → 170699 bytes (6.36%).

Budgets fixed in pa4/plan.md: unaffected paired latency <=10% plus measured noise; RSS <=15% plus 1 MiB; host text growth <=15%; 4x flat input <6x wall time and <5x RSS. Indexed nesting captures <=3n tokens; generated spelling arena <=128 KiB. No runtime optimization or generated-code benefit is claimed.

| Workload | A median s | B median s | ABBA deltas % | A/A or B/B noise % | A RSS KiB | B RSS KiB |
| --- | ---: | ---: | --- | ---: | ---: | ---: |
| flat-4MiB | 0.745117 | 0.782914 | 4.64, 4.05 | 2.55% | 11476 | 11244 |
| flat-16MiB | 2.955044 | 3.107645 | 5.66, 2.01 | 0.13% | 34634 | 34638 |
| ordinary-C++ | 0.549403 | 0.569820 | 4.71, 4.27 | 1.65% | 7700 | 7582 |
| macro-reuse | 0.575179 | 0.620477 | 6.92, 8.76 | 0.32% | 4814 | 4816 |
| nested-arguments | 5.919123 | 0.109128 | -98.16, -98.16 | 1.08% | 137118 | 4568 |
| long-chain-reuse | 0.502328 | 0.559150 | 12.28, 12.12 | 2.43% | 10402 | 10712 |
| counter-spellings | 0.424313 | 0.430102 | 2.76, 1.87 | 2.61% | 8904 | 7696 |

## Manifests and telemetry

```json
[
  {
    "workload": "flat-4MiB",
    "bytes": 4194300,
    "input_sha256": "839f7d06615aebede01ae4ee50b219648cc4a2bf9a710beae9dfe74ae4a8b7c0",
    "output_sha256": "1e3691835865feac2eed05a273dce246317f2ef81ef9ef127060d5a9cdbf8350",
    "stats": {
      "preprocess_post_emit_ms": 773.346,
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
    "output_sha256": "162dd05df3e8c175bbf0ff57a035a2525720b264910e354f296bddd2e046adec",
    "stats": {
      "preprocess_post_emit_ms": 3072.33,
      "peak_rss_kib": 34652,
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
    "output_sha256": "263fb5dbdf6d36958b5c37bf97b29e6b732cfe65be04e1aa972b36e9a00b2f3b",
    "stats": {
      "preprocess_post_emit_ms": 560.004,
      "peak_rss_kib": 7468,
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
    "output_sha256": "8bc8418afca2899703e669506f35507942834430937520ca8bd077edfa8332be",
    "stats": {
      "preprocess_post_emit_ms": 614.053,
      "peak_rss_kib": 4824,
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
    "output_sha256": "698f5ca0cdcb7ee2957d74beaf9628acadf32b40abe9d136394509114ba222d4",
    "stats": {
      "preprocess_post_emit_ms": 104.864,
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
    "output_sha256": "008e9f88bb82ad3c5d13b8eaaef91b695db8ff35345909ba050ea222acdad837",
    "stats": {
      "preprocess_post_emit_ms": 559.171,
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
      "captured_tokens": 1024320,
      "borrowed_arguments": 64,
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
    "output_sha256": "1476095eadfdc6ff072db345505a57837009339ad50f34b3dec7b237eb7377ab",
    "stats": {
      "preprocess_post_emit_ms": 427.44,
      "peak_rss_kib": 7704,
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
| flat-4MiB | calibration | A | ordinary | 0.733505 | 11476 |
| flat-4MiB | calibration | A | ordinary | 0.752210 | 11232 |
| flat-4MiB | calibration | A | ordinary | 0.739805 | 11476 |
| flat-4MiB | calibration | A | ordinary | 0.740291 | 11464 |
| flat-4MiB | calibration | B | ordinary | 0.793622 | 11236 |
| flat-4MiB | calibration | B | ordinary | 0.782029 | 11480 |
| flat-4MiB | 1 | A | ordinary | 0.740556 | 11480 |
| flat-4MiB | 1 | B | ordinary | 0.783799 | 11488 |
| flat-4MiB | 1 | B | ordinary | 0.778269 | 11236 |
| flat-4MiB | 1 | A | ordinary | 0.752236 | 11480 |
| flat-4MiB | 2 | A | ordinary | 0.753501 | 11232 |
| flat-4MiB | 2 | B | ordinary | 0.779306 | 11248 |
| flat-4MiB | 2 | B | ordinary | 0.784799 | 11240 |
| flat-4MiB | 2 | A | ordinary | 0.749679 | 11476 |
| flat-4MiB | telemetry | B | stats | 0.783401 | 11476 |
| flat-16MiB | calibration | A | ordinary | 2.951028 | 34640 |
| flat-16MiB | calibration | A | ordinary | 2.954806 | 34628 |
| flat-16MiB | calibration | A | ordinary | 2.956453 | 34400 |
| flat-16MiB | calibration | A | ordinary | 2.955282 | 34640 |
| flat-16MiB | calibration | B | ordinary | 3.108702 | 34412 |
| flat-16MiB | calibration | B | ordinary | 3.108445 | 34648 |
| flat-16MiB | 1 | A | ordinary | 2.951904 | 34372 |
| flat-16MiB | 1 | B | ordinary | 3.134251 | 34632 |
| flat-16MiB | 1 | B | ordinary | 3.097900 | 34644 |
| flat-16MiB | 1 | A | ordinary | 2.946552 | 34408 |
| flat-16MiB | 2 | A | ordinary | 2.966295 | 34640 |
| flat-16MiB | 2 | B | ordinary | 3.106844 | 34656 |
| flat-16MiB | 2 | B | ordinary | 3.079984 | 34404 |
| flat-16MiB | 2 | A | ordinary | 3.098900 | 34648 |
| flat-16MiB | telemetry | B | stats | 3.094461 | 34652 |
| ordinary-C++ | calibration | A | ordinary | 0.559544 | 7700 |
| ordinary-C++ | calibration | A | ordinary | 0.551034 | 7700 |
| ordinary-C++ | calibration | A | ordinary | 0.557845 | 7700 |
| ordinary-C++ | calibration | A | ordinary | 0.548659 | 7688 |
| ordinary-C++ | calibration | B | ordinary | 0.567876 | 7464 |
| ordinary-C++ | calibration | B | ordinary | 0.568138 | 7708 |
| ordinary-C++ | 1 | A | ordinary | 0.546704 | 7472 |
| ordinary-C++ | 1 | B | ordinary | 0.569564 | 7460 |
| ordinary-C++ | 1 | B | ordinary | 0.578902 | 7688 |
| ordinary-C++ | 1 | A | ordinary | 0.550146 | 7704 |
| ordinary-C++ | 2 | A | ordinary | 0.547715 | 7456 |
| ordinary-C++ | 2 | B | ordinary | 0.570075 | 7476 |
| ordinary-C++ | 2 | B | ordinary | 0.572951 | 7708 |
| ordinary-C++ | 2 | A | ordinary | 0.548538 | 7700 |
| ordinary-C++ | telemetry | B | stats | 0.568204 | 7468 |
| macro-reuse | calibration | A | ordinary | 0.574695 | 4824 |
| macro-reuse | calibration | A | ordinary | 0.575976 | 4784 |
| macro-reuse | calibration | A | ordinary | 0.575663 | 4584 |
| macro-reuse | calibration | A | ordinary | 0.577488 | 4236 |
| macro-reuse | calibration | B | ordinary | 0.619233 | 4824 |
| macro-reuse | calibration | B | ordinary | 0.620339 | 4824 |
| macro-reuse | 1 | A | ordinary | 0.586987 | 4808 |
| macro-reuse | 1 | B | ordinary | 0.619090 | 4828 |
| macro-reuse | 1 | B | ordinary | 0.620616 | 4344 |
| macro-reuse | 1 | A | ordinary | 0.572444 | 4824 |
| macro-reuse | 2 | A | ordinary | 0.572949 | 4820 |
| macro-reuse | 2 | B | ordinary | 0.621712 | 4804 |
| macro-reuse | 2 | B | ordinary | 0.622714 | 4808 |
| macro-reuse | 2 | A | ordinary | 0.571288 | 4824 |
| macro-reuse | telemetry | B | stats | 0.622282 | 4824 |
| nested-arguments | calibration | A | ordinary | 5.904773 | 136952 |
| nested-arguments | calibration | A | ordinary | 5.933916 | 136868 |
| nested-arguments | calibration | A | ordinary | 5.915221 | 137120 |
| nested-arguments | calibration | A | ordinary | 5.908383 | 136924 |
| nested-arguments | calibration | B | ordinary | 0.109004 | 4296 |
| nested-arguments | calibration | B | ordinary | 0.110180 | 4572 |
| nested-arguments | 1 | A | ordinary | 5.928153 | 137176 |
| nested-arguments | 1 | B | ordinary | 0.108664 | 4568 |
| nested-arguments | 1 | B | ordinary | 0.109252 | 4572 |
| nested-arguments | 1 | A | ordinary | 5.936280 | 137136 |
| nested-arguments | 2 | A | ordinary | 5.923026 | 137116 |
| nested-arguments | 2 | B | ordinary | 0.109513 | 4568 |
| nested-arguments | 2 | B | ordinary | 0.108125 | 4324 |
| nested-arguments | 2 | A | ordinary | 5.906222 | 137196 |
| nested-arguments | telemetry | B | stats | 0.110169 | 4568 |
| long-chain-reuse | calibration | A | ordinary | 0.505492 | 10432 |
| long-chain-reuse | calibration | A | ordinary | 0.499451 | 10212 |
| long-chain-reuse | calibration | A | ordinary | 0.511042 | 10452 |
| long-chain-reuse | calibration | A | ordinary | 0.505206 | 10452 |
| long-chain-reuse | calibration | B | ordinary | 0.567502 | 10724 |
| long-chain-reuse | calibration | B | ordinary | 0.553704 | 10704 |
| long-chain-reuse | 1 | A | ordinary | 0.507337 | 10220 |
| long-chain-reuse | 1 | B | ordinary | 0.555530 | 10704 |
| long-chain-reuse | 1 | B | ordinary | 0.571711 | 10720 |
| long-chain-reuse | 1 | A | ordinary | 0.496578 | 9984 |
| long-chain-reuse | 2 | A | ordinary | 0.499242 | 10372 |
| long-chain-reuse | 2 | B | ordinary | 0.558876 | 10476 |
| long-chain-reuse | 2 | B | ordinary | 0.559425 | 10728 |
| long-chain-reuse | 2 | A | ordinary | 0.498144 | 10460 |
| long-chain-reuse | telemetry | B | stats | 0.565685 | 10724 |
| counter-spellings | calibration | A | ordinary | 0.426841 | 8672 |
| counter-spellings | calibration | A | ordinary | 0.415684 | 8904 |
| counter-spellings | calibration | A | ordinary | 0.433578 | 8672 |
| counter-spellings | calibration | A | ordinary | 0.444228 | 8916 |
| counter-spellings | calibration | B | ordinary | 0.429869 | 7480 |
| counter-spellings | calibration | B | ordinary | 0.433372 | 7704 |
| counter-spellings | 1 | A | ordinary | 0.418579 | 8916 |
| counter-spellings | 1 | B | ordinary | 0.430335 | 7704 |
| counter-spellings | 1 | B | ordinary | 0.428193 | 7708 |
| counter-spellings | 1 | A | ordinary | 0.416872 | 8912 |
| counter-spellings | 2 | A | ordinary | 0.422870 | 8904 |
| counter-spellings | 2 | B | ordinary | 0.427942 | 7456 |
| counter-spellings | 2 | B | ordinary | 0.436556 | 7688 |
| counter-spellings | 2 | A | ordinary | 0.425756 | 8900 |
| counter-spellings | telemetry | B | stats | 0.434356 | 7704 |

Output SHA256 is recorded once per workload above; every sample was checked against it.
