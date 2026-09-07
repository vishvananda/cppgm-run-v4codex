# PA4 frozen compiler performance evidence

A: `60ef1b9df`; B: `c558c57d2` (stable prescan slabs and reusable depth-local argument/output buffers).

Both use the ordinary `g++ -std=gnu++11 -Wall -O3` build with the same course runner. Fresh processes, serial measurements, output to a temporary file, wall time including process startup and peak RSS from GNU time. Each workload has two A/A pairs, one B/B pair, two ABBA blocks and a separate B telemetry sample. All observations are below. Every output agrees within its workload. Temporary source paths affect output hashes, but are identical across each A/B group.

Platform: `Linux-7.0.0-1005-gcp-x86_64-with-glibc2.43`. Host: `g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0`.

Inherited CPU affinity: `0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31`.

Frozen SHA256 A: `15b6ea60b9d6722dca2e340f08ec8ba39fc6b13dbab54a18a85071108e9e7dfa`; B: `720620ab85cf7ef80823875393139638869421f73ed534fcee4148b19a16c86b`.

Generated executable runtime / generated text size: **N/A at PA4**. Host-tool text (GNU size, including read-only data): 171378 → 168444 bytes (-1.71%).

Budgets fixed in pa4/plan.md: unaffected paired latency <=10% plus measured noise; RSS <=15% plus 1 MiB; host text growth <=15%; 4x flat input <6x wall time and <5x RSS. Indexed nesting captures <=3n tokens; generated spelling arena <=128 KiB. No runtime optimization or generated-code benefit is claimed.

| Workload | A median s | B median s | ABBA deltas % | A/A or B/B noise % | A RSS KiB | B RSS KiB |
| --- | ---: | ---: | --- | ---: | ---: | ---: |
| flat-4MiB | 0.792505 | 0.781701 | -2.70, -1.89 | 1.16% | 11480 | 11470 |
| flat-16MiB | 3.154598 | 3.045117 | -3.51, -2.88 | 0.62% | 34620 | 34630 |
| ordinary-C++ | 0.577908 | 0.556397 | -3.67, -4.08 | 1.53% | 7704 | 7702 |
| macro-reuse | 0.629000 | 0.597949 | -4.75, -5.24 | 1.10% | 4824 | 4808 |
| nested-arguments | 0.108890 | 0.094400 | -13.60, -13.87 | 4.71% | 4552 | 4822 |
| long-chain-reuse | 0.468621 | 0.472237 | 1.07, -2.20 | 0.97% | 10714 | 10724 |
| counter-spellings | 0.440733 | 0.430097 | -2.63, -2.72 | 1.87% | 7678 | 7686 |
| literal-locations | 0.227541 | 0.224634 | -0.18, -1.39 | 0.93% | 5688 | 5688 |

## Manifests and telemetry

```json
[
  {
    "workload": "flat-4MiB",
    "bytes": 4194300,
    "input_sha256": "839f7d06615aebede01ae4ee50b219648cc4a2bf9a710beae9dfe74ae4a8b7c0",
    "output_sha256": "4a6b193dc550865303cfded192c7bebe61be79a29bae8fa8b55a841719d95b21",
    "stats": {
      "preprocess_post_emit_ms": 760.24,
      "peak_rss_kib": 11460,
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
      "task_slabs": 0,
      "argument_growths": 0,
      "prescan_output_growths": 0,
      "identifiers": 12,
      "identifier_storage_bytes": 674
    }
  },
  {
    "workload": "flat-16MiB",
    "bytes": 16777212,
    "input_sha256": "25d66aba4b4fc9e3bbc0609785322c6c5e46abef589d133883796b5cffb18e71",
    "output_sha256": "909ca68f822c823108c335698beedab38e4f9c6427c54cc44a10c3f5c6af2e0c",
    "stats": {
      "preprocess_post_emit_ms": 3044.63,
      "peak_rss_kib": 34416,
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
      "task_slabs": 0,
      "argument_growths": 0,
      "prescan_output_growths": 0,
      "identifiers": 12,
      "identifier_storage_bytes": 674
    }
  },
  {
    "workload": "ordinary-C++",
    "bytes": 2592000,
    "input_sha256": "9420f4ef2269b65da14dc0ba7d9ddda577844a73c512f1594fa8ea3606a1abec",
    "output_sha256": "37c3d2444f47fcd62f8e50049f44f95590f64e926b76ba2c647e253f1facfe23",
    "stats": {
      "preprocess_post_emit_ms": 549.206,
      "peak_rss_kib": 7708,
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
      "task_slabs": 0,
      "argument_growths": 0,
      "prescan_output_growths": 0,
      "identifiers": 28,
      "identifier_storage_bytes": 1340
    }
  },
  {
    "workload": "macro-reuse",
    "bytes": 750021,
    "input_sha256": "d43d80a2c9e8e4feceb80891609c70868ea93f2c6a95d04cef1f1ccb7eee394f",
    "output_sha256": "895a562ae89b89565ec8a4820cf3840c8556f82955f8cce78b549822a094cf92",
    "stats": {
      "preprocess_post_emit_ms": 592.737,
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
      "task_slabs": 1,
      "argument_growths": 1,
      "prescan_output_growths": 2,
      "identifiers": 14,
      "identifier_storage_bytes": 826
    }
  },
  {
    "workload": "nested-arguments",
    "bytes": 230799,
    "input_sha256": "eec04db3cab0c61c916ffa5f177a5063fdfcc7ca8c5af6151e59612ead63f22f",
    "output_sha256": "5aded6d3bc5348a3965936e07fa788b7081706b05ac431999f33be4d89253c8b",
    "stats": {
      "preprocess_post_emit_ms": 92.8998,
      "peak_rss_kib": 4580,
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
      "scratch_growths": 1200,
      "task_slabs": 19,
      "argument_growths": 600,
      "prescan_output_growths": 1200,
      "identifiers": 14,
      "identifier_storage_bytes": 836
    }
  },
  {
    "workload": "long-chain-reuse",
    "bytes": 190647,
    "input_sha256": "41d6fde73b4343b2e4aa937120faf11d7bf6c69bbb983882f8c65ddd2515a889",
    "output_sha256": "3c54a132ae5d70659297d1aee5e54cabbb7afbd4e7f02effc520eea6437daeab",
    "stats": {
      "preprocess_post_emit_ms": 466.178,
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
      "scratch_growths": 12,
      "task_slabs": 1,
      "argument_growths": 1,
      "prescan_output_growths": 2,
      "identifiers": 8016,
      "identifier_storage_bytes": 303348
    }
  },
  {
    "workload": "counter-spellings",
    "bytes": 3600000,
    "input_sha256": "454ff659f5023a54922fc5212cef8c4d0d5d55c70491724da8fa8249ab340faa",
    "output_sha256": "aed07ea1ecd20495123bcb1d57f897714c6010cf90e4eb7d53ec34d2c72640b9",
    "stats": {
      "preprocess_post_emit_ms": 419.68,
      "peak_rss_kib": 7708,
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
      "task_slabs": 0,
      "argument_growths": 0,
      "prescan_output_growths": 0,
      "identifiers": 11,
      "identifier_storage_bytes": 738
    }
  },
  {
    "workload": "literal-locations",
    "bytes": 1480047,
    "input_sha256": "de53addfe427adccb6f943334ae2e9e5e3ba24efa2bc7b64fe382a991416a082",
    "output_sha256": "2b8fa0de9388d2b4737057fdc0c147dba6fd57ba956c1942995154b0ba51aee5",
    "stats": {
      "preprocess_post_emit_ms": 215.833,
      "peak_rss_kib": 5456,
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
      "task_slabs": 0,
      "argument_growths": 0,
      "prescan_output_growths": 0,
      "identifiers": 18,
      "identifier_storage_bytes": 1250
    }
  }
]
```

## Every observation

| Workload | Block | Binary | Mode | Wall seconds | Peak RSS KiB |
| --- | --- | --- | --- | ---: | ---: |
| flat-4MiB | calibration | A | ordinary | 0.798368 | 11252 |
| flat-4MiB | calibration | A | ordinary | 0.790734 | 11468 |
| flat-4MiB | calibration | A | ordinary | 0.799214 | 11480 |
| flat-4MiB | calibration | A | ordinary | 0.789960 | 11240 |
| flat-4MiB | calibration | B | ordinary | 0.784749 | 11484 |
| flat-4MiB | calibration | B | ordinary | 0.781345 | 11484 |
| flat-4MiB | 1 | A | ordinary | 0.806227 | 11480 |
| flat-4MiB | 1 | B | ordinary | 0.782056 | 11484 |
| flat-4MiB | 1 | B | ordinary | 0.769164 | 11244 |
| flat-4MiB | 1 | A | ordinary | 0.787966 | 11484 |
| flat-4MiB | 2 | A | ordinary | 0.791284 | 11484 |
| flat-4MiB | 2 | B | ordinary | 0.770415 | 11456 |
| flat-4MiB | 2 | B | ordinary | 0.784630 | 11272 |
| flat-4MiB | 2 | A | ordinary | 0.793725 | 11480 |
| flat-4MiB | telemetry | B | stats | 0.769435 | 11460 |
| flat-16MiB | calibration | A | ordinary | 3.159231 | 34652 |
| flat-16MiB | calibration | A | ordinary | 3.139670 | 34592 |
| flat-16MiB | calibration | A | ordinary | 3.152397 | 34652 |
| flat-16MiB | calibration | A | ordinary | 3.156799 | 34428 |
| flat-16MiB | calibration | B | ordinary | 3.032988 | 34648 |
| flat-16MiB | calibration | B | ordinary | 3.043613 | 34624 |
| flat-16MiB | 1 | A | ordinary | 3.168029 | 34648 |
| flat-16MiB | 1 | B | ordinary | 3.060738 | 34164 |
| flat-16MiB | 1 | B | ordinary | 3.043022 | 34404 |
| flat-16MiB | 1 | A | ordinary | 3.157448 | 34404 |
| flat-16MiB | 2 | A | ordinary | 3.128479 | 34412 |
| flat-16MiB | 2 | B | ordinary | 3.046620 | 34636 |
| flat-16MiB | 2 | B | ordinary | 3.047551 | 34652 |
| flat-16MiB | 2 | A | ordinary | 3.146392 | 34652 |
| flat-16MiB | telemetry | B | stats | 3.067055 | 34416 |
| ordinary-C++ | calibration | A | ordinary | 0.585703 | 7476 |
| ordinary-C++ | calibration | A | ordinary | 0.576766 | 7684 |
| ordinary-C++ | calibration | A | ordinary | 0.578266 | 7712 |
| ordinary-C++ | calibration | A | ordinary | 0.576269 | 7704 |
| ordinary-C++ | calibration | B | ordinary | 0.560097 | 7704 |
| ordinary-C++ | calibration | B | ordinary | 0.555283 | 7700 |
| ordinary-C++ | 1 | A | ordinary | 0.577714 | 7704 |
| ordinary-C++ | 1 | B | ordinary | 0.554664 | 7500 |
| ordinary-C++ | 1 | B | ordinary | 0.558727 | 7708 |
| ordinary-C++ | 1 | A | ordinary | 0.578101 | 7708 |
| ordinary-C++ | 2 | A | ordinary | 0.576548 | 7704 |
| ordinary-C++ | 2 | B | ordinary | 0.556007 | 7708 |
| ordinary-C++ | 2 | B | ordinary | 0.556787 | 7464 |
| ordinary-C++ | 2 | A | ordinary | 0.583534 | 7704 |
| ordinary-C++ | telemetry | B | stats | 0.557516 | 7708 |
| macro-reuse | calibration | A | ordinary | 0.624656 | 4828 |
| macro-reuse | calibration | A | ordinary | 0.631546 | 4824 |
| macro-reuse | calibration | A | ordinary | 0.627560 | 4824 |
| macro-reuse | calibration | A | ordinary | 0.627804 | 4600 |
| macro-reuse | calibration | B | ordinary | 0.597509 | 4596 |
| macro-reuse | calibration | B | ordinary | 0.597189 | 4824 |
| macro-reuse | 1 | A | ordinary | 0.628405 | 4824 |
| macro-reuse | 1 | B | ordinary | 0.598039 | 4796 |
| macro-reuse | 1 | B | ordinary | 0.600239 | 4820 |
| macro-reuse | 1 | A | ordinary | 0.629595 | 4800 |
| macro-reuse | 2 | A | ordinary | 0.631367 | 4600 |
| macro-reuse | 2 | B | ordinary | 0.599455 | 4824 |
| macro-reuse | 2 | B | ordinary | 0.597859 | 4580 |
| macro-reuse | 2 | A | ordinary | 0.632126 | 4824 |
| macro-reuse | telemetry | B | stats | 0.601256 | 4680 |
| nested-arguments | calibration | A | ordinary | 0.112741 | 4568 |
| nested-arguments | calibration | A | ordinary | 0.108778 | 4332 |
| nested-arguments | calibration | A | ordinary | 0.108482 | 4568 |
| nested-arguments | calibration | A | ordinary | 0.108105 | 4296 |
| nested-arguments | calibration | B | ordinary | 0.098904 | 4824 |
| nested-arguments | calibration | B | ordinary | 0.094248 | 4824 |
| nested-arguments | 1 | A | ordinary | 0.111642 | 4552 |
| nested-arguments | 1 | B | ordinary | 0.096212 | 4820 |
| nested-arguments | 1 | B | ordinary | 0.094551 | 4824 |
| nested-arguments | 1 | A | ordinary | 0.109151 | 4344 |
| nested-arguments | 2 | A | ordinary | 0.108459 | 4552 |
| nested-arguments | 2 | B | ordinary | 0.093587 | 4584 |
| nested-arguments | 2 | B | ordinary | 0.093702 | 4796 |
| nested-arguments | 2 | A | ordinary | 0.109002 | 4568 |
| nested-arguments | telemetry | B | stats | 0.098034 | 4836 |
| long-chain-reuse | calibration | A | ordinary | 0.468763 | 10712 |
| long-chain-reuse | calibration | A | ordinary | 0.464221 | 10556 |
| long-chain-reuse | calibration | A | ordinary | 0.466130 | 10724 |
| long-chain-reuse | calibration | A | ordinary | 0.468479 | 10700 |
| long-chain-reuse | calibration | B | ordinary | 0.469302 | 10728 |
| long-chain-reuse | calibration | B | ordinary | 0.472881 | 10724 |
| long-chain-reuse | 1 | A | ordinary | 0.470506 | 10720 |
| long-chain-reuse | 1 | B | ordinary | 0.471594 | 10728 |
| long-chain-reuse | 1 | B | ordinary | 0.473250 | 10724 |
| long-chain-reuse | 1 | A | ordinary | 0.464357 | 10712 |
| long-chain-reuse | 2 | A | ordinary | 0.469173 | 10716 |
| long-chain-reuse | 2 | B | ordinary | 0.469272 | 10484 |
| long-chain-reuse | 2 | B | ordinary | 0.473545 | 10724 |
| long-chain-reuse | 2 | A | ordinary | 0.494815 | 10720 |
| long-chain-reuse | telemetry | B | stats | 0.472530 | 10724 |
| counter-spellings | calibration | A | ordinary | 0.436759 | 7460 |
| counter-spellings | calibration | A | ordinary | 0.444920 | 7456 |
| counter-spellings | calibration | A | ordinary | 0.440269 | 7692 |
| counter-spellings | calibration | A | ordinary | 0.440453 | 7704 |
| counter-spellings | calibration | B | ordinary | 0.428989 | 7704 |
| counter-spellings | calibration | B | ordinary | 0.435651 | 7684 |
| counter-spellings | 1 | A | ordinary | 0.441013 | 7704 |
| counter-spellings | 1 | B | ordinary | 0.434757 | 7708 |
| counter-spellings | 1 | B | ordinary | 0.431205 | 7688 |
| counter-spellings | 1 | A | ordinary | 0.448306 | 7664 |
| counter-spellings | 2 | A | ordinary | 0.437061 | 7708 |
| counter-spellings | 2 | B | ordinary | 0.427908 | 7464 |
| counter-spellings | 2 | B | ordinary | 0.428072 | 7460 |
| counter-spellings | 2 | A | ordinary | 0.442838 | 7456 |
| counter-spellings | telemetry | B | stats | 0.426548 | 7708 |
| literal-locations | calibration | A | ordinary | 0.228898 | 5692 |
| literal-locations | calibration | A | ordinary | 0.226758 | 5688 |
| literal-locations | calibration | A | ordinary | 0.227764 | 5692 |
| literal-locations | calibration | A | ordinary | 0.227318 | 5688 |
| literal-locations | calibration | B | ordinary | 0.224666 | 5688 |
| literal-locations | calibration | B | ordinary | 0.224603 | 5692 |
| literal-locations | 1 | A | ordinary | 0.225955 | 5688 |
| literal-locations | 1 | B | ordinary | 0.224113 | 5688 |
| literal-locations | 1 | B | ordinary | 0.226551 | 5676 |
| literal-locations | 1 | A | ordinary | 0.225521 | 5680 |
| literal-locations | 2 | A | ordinary | 0.227837 | 5444 |
| literal-locations | 2 | B | ordinary | 0.226803 | 5672 |
| literal-locations | 2 | B | ordinary | 0.223159 | 5692 |
| literal-locations | 2 | A | ordinary | 0.228472 | 5664 |
| literal-locations | telemetry | B | stats | 0.222531 | 5456 |

Output SHA256 is recorded once per workload above; every sample was checked against it.

## Startup calibration

```json
[
  {
    "workload": "startup",
    "mode": "ordinary",
    "binary": "B",
    "seconds": 0.00549479853361845,
    "rss_kib": 3560,
    "output_sha256": "d0e92e5931c813d451a5abfd207a21849322b1cb97093eecbbbc53af348b981f"
  },
  {
    "workload": "startup",
    "mode": "ordinary",
    "binary": "B",
    "seconds": 0.004380813799798489,
    "rss_kib": 3568,
    "output_sha256": "d0e92e5931c813d451a5abfd207a21849322b1cb97093eecbbbc53af348b981f"
  },
  {
    "workload": "startup",
    "mode": "ordinary",
    "binary": "B",
    "seconds": 0.003840062767267227,
    "rss_kib": 3544,
    "output_sha256": "d0e92e5931c813d451a5abfd207a21849322b1cb97093eecbbbc53af348b981f"
  },
  {
    "workload": "startup",
    "mode": "ordinary",
    "binary": "B",
    "seconds": 0.0038139279931783676,
    "rss_kib": 3568,
    "output_sha256": "d0e92e5931c813d451a5abfd207a21849322b1cb97093eecbbbc53af348b981f"
  },
  {
    "workload": "startup",
    "mode": "ordinary",
    "binary": "B",
    "seconds": 0.003954092040657997,
    "rss_kib": 3544,
    "output_sha256": "d0e92e5931c813d451a5abfd207a21849322b1cb97093eecbbbc53af348b981f"
  },
  {
    "workload": "startup",
    "mode": "ordinary",
    "binary": "B",
    "seconds": 0.004396681673824787,
    "rss_kib": 3544,
    "output_sha256": "d0e92e5931c813d451a5abfd207a21849322b1cb97093eecbbbc53af348b981f"
  },
  {
    "workload": "startup",
    "mode": "ordinary",
    "binary": "B",
    "seconds": 0.003917468711733818,
    "rss_kib": 3548,
    "output_sha256": "d0e92e5931c813d451a5abfd207a21849322b1cb97093eecbbbc53af348b981f"
  },
  {
    "workload": "startup",
    "mode": "ordinary",
    "binary": "B",
    "seconds": 0.004183543846011162,
    "rss_kib": 3520,
    "output_sha256": "d0e92e5931c813d451a5abfd207a21849322b1cb97093eecbbbc53af348b981f"
  }
]
```

Median startup: 0.004069 s; fastest workload: 23.2x startup.
