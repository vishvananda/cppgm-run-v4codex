# PA4 frozen compiler performance evidence

A: `60ef1b9df`; B: `54f4824ac` (stable prescan slabs, reusable argument/output buffers and retained delimiter-index scratch).

Both use the ordinary `g++ -std=gnu++11 -Wall -O3` build with the same course runner. Fresh processes, serial measurements, output to a temporary file, wall time including process startup and peak RSS from GNU time. Each workload has two A/A pairs, one B/B pair, two ABBA blocks and a separate B telemetry sample. All observations are below. Every output agrees within its workload. Temporary source paths affect output hashes, but are identical across each A/B group.

Platform: `Linux-7.0.0-1005-gcp-x86_64-with-glibc2.43`. Host: `g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0`.

Inherited CPU affinity: `0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31`.

Frozen SHA256 A: `15b6ea60b9d6722dca2e340f08ec8ba39fc6b13dbab54a18a85071108e9e7dfa`; B: `ba394efdfa3b6fd9e8c25522fa9a89ea08c1656efbc5c9618268349cae6f6c4b`.

Generated executable runtime / generated text size: **N/A at PA4**. Host-tool text (GNU size, including read-only data): 171378 → 168528 bytes (-1.66%).

Budgets fixed in pa4/plan.md: unaffected paired latency <=10% plus measured noise; RSS <=15% plus 1 MiB; host text growth <=15%; 4x flat input <6x wall time and <5x RSS. Indexed nesting captures <=3n tokens; generated spelling arena <=128 KiB. No runtime optimization or generated-code benefit is claimed.

| Workload | A median s | B median s | ABBA deltas % | A/A or B/B noise % | A RSS KiB | B RSS KiB |
| --- | ---: | ---: | --- | ---: | ---: | ---: |
| flat-4MiB | 0.789227 | 0.772884 | -2.07, -2.98 | 7.53% | 11480 | 11244 |
| flat-16MiB | 3.138684 | 3.065458 | -2.65, -2.62 | 1.49% | 34532 | 34578 |
| ordinary-C++ | 0.578575 | 0.560816 | -3.54, -3.07 | 4.22% | 7704 | 7686 |
| macro-reuse | 0.629724 | 0.595806 | -5.78, -5.74 | 0.60% | 4822 | 4820 |
| nested-arguments | 0.108902 | 0.094080 | -13.90, -12.35 | 3.94% | 4568 | 4804 |
| long-chain-reuse | 0.467457 | 0.477037 | 1.84, 1.42 | 1.19% | 10652 | 10726 |
| counter-spellings | 0.442001 | 0.435811 | -1.52, 1.59 | 3.75% | 7694 | 7600 |
| literal-locations | 0.229719 | 0.225211 | -2.77, -1.37 | 1.71% | 5564 | 5688 |

## Manifests and telemetry

```json
[
  {
    "workload": "flat-4MiB",
    "bytes": 4194300,
    "input_sha256": "839f7d06615aebede01ae4ee50b219648cc4a2bf9a710beae9dfe74ae4a8b7c0",
    "output_sha256": "97e691c8583320ab44cafa3c1109d94e563c5f02726ac0b892c2b81bb7ee980c",
    "stats": {
      "preprocess_post_emit_ms": 767.363,
      "peak_rss_kib": 11484,
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
    "output_sha256": "a304419b3423d459cd9c347efb427f24d5b104a350b928a5fc92f041d4e81789",
    "stats": {
      "preprocess_post_emit_ms": 3043.46,
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
    "output_sha256": "0dd621a5dcbf1ecac98b1851a4293d29358273ddc31fe17ea1a003310eaf47a7",
    "stats": {
      "preprocess_post_emit_ms": 552.609,
      "peak_rss_kib": 7696,
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
    "output_sha256": "e2ca2dcf6d2e8dabcb0a6f2ec64d455951eb0b02bdf00b22ffeff0e88886c7ec",
    "stats": {
      "preprocess_post_emit_ms": 587.588,
      "peak_rss_kib": 4584,
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
    "output_sha256": "d557887d48c44d50ca671afdaed57d10a847f06a22637c5f2a72df63ba71515a",
    "stats": {
      "preprocess_post_emit_ms": 89.1435,
      "peak_rss_kib": 4804,
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
    "output_sha256": "e7014bd138305ac0f87084b944a4d9efb66e1cdfc575ba75d624099182232125",
    "stats": {
      "preprocess_post_emit_ms": 474.219,
      "peak_rss_kib": 10700,
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
    "output_sha256": "7c8e5f9b93c57cdd49159c9a3f3460ab0e1a88e75988e42f006ed117324d69ed",
    "stats": {
      "preprocess_post_emit_ms": 428.847,
      "peak_rss_kib": 7688,
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
    "output_sha256": "72bb954802e2d1e217b9cffbaf75f471d9086d6b5ae72a8c71749966baadca82",
    "stats": {
      "preprocess_post_emit_ms": 221.361,
      "peak_rss_kib": 5688,
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
| flat-4MiB | calibration | A | ordinary | 0.781978 | 11480 |
| flat-4MiB | calibration | A | ordinary | 0.794359 | 11484 |
| flat-4MiB | calibration | A | ordinary | 0.791516 | 11484 |
| flat-4MiB | calibration | A | ordinary | 0.851118 | 11236 |
| flat-4MiB | calibration | B | ordinary | 0.781851 | 11240 |
| flat-4MiB | calibration | B | ordinary | 0.779170 | 11484 |
| flat-4MiB | 1 | A | ordinary | 0.789347 | 11480 |
| flat-4MiB | 1 | B | ordinary | 0.776349 | 11232 |
| flat-4MiB | 1 | B | ordinary | 0.769419 | 11236 |
| flat-4MiB | 1 | A | ordinary | 0.789108 | 11468 |
| flat-4MiB | 2 | A | ordinary | 0.785503 | 11476 |
| flat-4MiB | 2 | B | ordinary | 0.765360 | 11476 |
| flat-4MiB | 2 | B | ordinary | 0.761685 | 11248 |
| flat-4MiB | 2 | A | ordinary | 0.788396 | 11480 |
| flat-4MiB | telemetry | B | stats | 0.776986 | 11484 |
| flat-16MiB | calibration | A | ordinary | 3.127619 | 34648 |
| flat-16MiB | calibration | A | ordinary | 3.156202 | 34648 |
| flat-16MiB | calibration | A | ordinary | 3.133334 | 34408 |
| flat-16MiB | calibration | A | ordinary | 3.136712 | 34624 |
| flat-16MiB | calibration | B | ordinary | 3.084966 | 34404 |
| flat-16MiB | calibration | B | ordinary | 3.039041 | 34648 |
| flat-16MiB | 1 | A | ordinary | 3.147096 | 34640 |
| flat-16MiB | 1 | B | ordinary | 3.036881 | 34420 |
| flat-16MiB | 1 | B | ordinary | 3.076225 | 34508 |
| flat-16MiB | 1 | A | ordinary | 3.132712 | 34420 |
| flat-16MiB | 2 | A | ordinary | 3.140657 | 34440 |
| flat-16MiB | 2 | B | ordinary | 3.075167 | 34648 |
| flat-16MiB | 2 | B | ordinary | 3.055749 | 34648 |
| flat-16MiB | 2 | A | ordinary | 3.155450 | 34424 |
| flat-16MiB | telemetry | B | stats | 3.065099 | 34652 |
| ordinary-C++ | calibration | A | ordinary | 0.588700 | 7488 |
| ordinary-C++ | calibration | A | ordinary | 0.577031 | 7704 |
| ordinary-C++ | calibration | A | ordinary | 0.571739 | 7704 |
| ordinary-C++ | calibration | A | ordinary | 0.595856 | 7704 |
| ordinary-C++ | calibration | B | ordinary | 0.563455 | 7688 |
| ordinary-C++ | calibration | B | ordinary | 0.561477 | 7684 |
| ordinary-C++ | 1 | A | ordinary | 0.578431 | 7708 |
| ordinary-C++ | 1 | B | ordinary | 0.557061 | 7476 |
| ordinary-C++ | 1 | B | ordinary | 0.560155 | 7464 |
| ordinary-C++ | 1 | A | ordinary | 0.579736 | 7676 |
| ordinary-C++ | 2 | A | ordinary | 0.578719 | 7684 |
| ordinary-C++ | 2 | B | ordinary | 0.557793 | 7704 |
| ordinary-C++ | 2 | B | ordinary | 0.563610 | 7704 |
| ordinary-C++ | 2 | A | ordinary | 0.578228 | 7704 |
| ordinary-C++ | telemetry | B | stats | 0.561289 | 7696 |
| macro-reuse | calibration | A | ordinary | 0.630966 | 4584 |
| macro-reuse | calibration | A | ordinary | 0.629898 | 4592 |
| macro-reuse | calibration | A | ordinary | 0.625258 | 4580 |
| macro-reuse | calibration | A | ordinary | 0.627239 | 4824 |
| macro-reuse | calibration | B | ordinary | 0.600075 | 4820 |
| macro-reuse | calibration | B | ordinary | 0.596445 | 4820 |
| macro-reuse | 1 | A | ordinary | 0.629155 | 4828 |
| macro-reuse | 1 | B | ordinary | 0.594365 | 4312 |
| macro-reuse | 1 | B | ordinary | 0.595770 | 4820 |
| macro-reuse | 1 | A | ordinary | 0.633928 | 4820 |
| macro-reuse | 2 | A | ordinary | 0.629549 | 4824 |
| macro-reuse | 2 | B | ordinary | 0.595843 | 4824 |
| macro-reuse | 2 | B | ordinary | 0.593375 | 4824 |
| macro-reuse | 2 | A | ordinary | 0.632123 | 4824 |
| macro-reuse | telemetry | B | stats | 0.595725 | 4684 |
| nested-arguments | calibration | A | ordinary | 0.113413 | 4568 |
| nested-arguments | calibration | A | ordinary | 0.108944 | 4344 |
| nested-arguments | calibration | A | ordinary | 0.109299 | 4568 |
| nested-arguments | calibration | A | ordinary | 0.108586 | 4572 |
| nested-arguments | calibration | B | ordinary | 0.093978 | 4804 |
| nested-arguments | calibration | B | ordinary | 0.093784 | 4580 |
| nested-arguments | 1 | A | ordinary | 0.108860 | 4568 |
| nested-arguments | 1 | B | ordinary | 0.093666 | 4820 |
| nested-arguments | 1 | B | ordinary | 0.094183 | 4824 |
| nested-arguments | 1 | A | ordinary | 0.109318 | 4568 |
| nested-arguments | 2 | A | ordinary | 0.108322 | 4336 |
| nested-arguments | 2 | B | ordinary | 0.095331 | 4800 |
| nested-arguments | 2 | B | ordinary | 0.094769 | 4804 |
| nested-arguments | 2 | A | ordinary | 0.108567 | 4568 |
| nested-arguments | telemetry | B | stats | 0.094367 | 4804 |
| long-chain-reuse | calibration | A | ordinary | 0.470406 | 10480 |
| long-chain-reuse | calibration | A | ordinary | 0.467366 | 10696 |
| long-chain-reuse | calibration | A | ordinary | 0.467548 | 10720 |
| long-chain-reuse | calibration | A | ordinary | 0.469219 | 10696 |
| long-chain-reuse | calibration | B | ordinary | 0.472247 | 10728 |
| long-chain-reuse | calibration | B | ordinary | 0.477848 | 10728 |
| long-chain-reuse | 1 | A | ordinary | 0.466628 | 10488 |
| long-chain-reuse | 1 | B | ordinary | 0.477940 | 10712 |
| long-chain-reuse | 1 | B | ordinary | 0.471776 | 10548 |
| long-chain-reuse | 1 | A | ordinary | 0.465961 | 10488 |
| long-chain-reuse | 2 | A | ordinary | 0.467260 | 10716 |
| long-chain-reuse | 2 | B | ordinary | 0.476227 | 10728 |
| long-chain-reuse | 2 | B | ordinary | 0.481015 | 10724 |
| long-chain-reuse | 2 | A | ordinary | 0.476570 | 10608 |
| long-chain-reuse | telemetry | B | stats | 0.480686 | 10700 |
| counter-spellings | calibration | A | ordinary | 0.442554 | 7704 |
| counter-spellings | calibration | A | ordinary | 0.441447 | 7468 |
| counter-spellings | calibration | A | ordinary | 0.461083 | 7676 |
| counter-spellings | calibration | A | ordinary | 0.443789 | 7700 |
| counter-spellings | calibration | B | ordinary | 0.430711 | 7704 |
| counter-spellings | calibration | B | ordinary | 0.436547 | 7708 |
| counter-spellings | 1 | A | ordinary | 0.435499 | 7708 |
| counter-spellings | 1 | B | ordinary | 0.431832 | 7480 |
| counter-spellings | 1 | B | ordinary | 0.435074 | 7708 |
| counter-spellings | 1 | A | ordinary | 0.444769 | 7680 |
| counter-spellings | 2 | A | ordinary | 0.436870 | 7704 |
| counter-spellings | 2 | B | ordinary | 0.448900 | 7496 |
| counter-spellings | 2 | B | ordinary | 0.440655 | 7460 |
| counter-spellings | 2 | A | ordinary | 0.438777 | 7688 |
| counter-spellings | telemetry | B | stats | 0.436051 | 7688 |
| literal-locations | calibration | A | ordinary | 0.228178 | 5464 |
| literal-locations | calibration | A | ordinary | 0.229269 | 5672 |
| literal-locations | calibration | A | ordinary | 0.228331 | 5460 |
| literal-locations | calibration | A | ordinary | 0.232226 | 5692 |
| literal-locations | calibration | B | ordinary | 0.226229 | 5688 |
| literal-locations | calibration | B | ordinary | 0.225765 | 5688 |
| literal-locations | 1 | A | ordinary | 0.230170 | 5452 |
| literal-locations | 1 | B | ordinary | 0.223220 | 5672 |
| literal-locations | 1 | B | ordinary | 0.223063 | 5688 |
| literal-locations | 1 | A | ordinary | 0.228827 | 5664 |
| literal-locations | 2 | A | ordinary | 0.230333 | 5676 |
| literal-locations | 2 | B | ordinary | 0.232486 | 5416 |
| literal-locations | 2 | B | ordinary | 0.224657 | 5692 |
| literal-locations | 2 | A | ordinary | 0.233149 | 5448 |
| literal-locations | telemetry | B | stats | 0.227949 | 5688 |

Output SHA256 is recorded once per workload above; every sample was checked against it.

## Startup calibration

```json
[
  {
    "workload": "startup",
    "mode": "ordinary",
    "binary": "B",
    "seconds": 0.006043397821485996,
    "rss_kib": 3516,
    "output_sha256": "c1efcf072f579bbdece90d442dfd4d0f29332c871911e368239be76e6216e76c"
  },
  {
    "workload": "startup",
    "mode": "ordinary",
    "binary": "B",
    "seconds": 0.0043559689074754715,
    "rss_kib": 3544,
    "output_sha256": "c1efcf072f579bbdece90d442dfd4d0f29332c871911e368239be76e6216e76c"
  },
  {
    "workload": "startup",
    "mode": "ordinary",
    "binary": "B",
    "seconds": 0.004394608549773693,
    "rss_kib": 3528,
    "output_sha256": "c1efcf072f579bbdece90d442dfd4d0f29332c871911e368239be76e6216e76c"
  },
  {
    "workload": "startup",
    "mode": "ordinary",
    "binary": "B",
    "seconds": 0.0039906855672597885,
    "rss_kib": 3544,
    "output_sha256": "c1efcf072f579bbdece90d442dfd4d0f29332c871911e368239be76e6216e76c"
  },
  {
    "workload": "startup",
    "mode": "ordinary",
    "binary": "B",
    "seconds": 0.004034037701785564,
    "rss_kib": 3540,
    "output_sha256": "c1efcf072f579bbdece90d442dfd4d0f29332c871911e368239be76e6216e76c"
  },
  {
    "workload": "startup",
    "mode": "ordinary",
    "binary": "B",
    "seconds": 0.004108062945306301,
    "rss_kib": 3544,
    "output_sha256": "c1efcf072f579bbdece90d442dfd4d0f29332c871911e368239be76e6216e76c"
  },
  {
    "workload": "startup",
    "mode": "ordinary",
    "binary": "B",
    "seconds": 0.0041724760085344315,
    "rss_kib": 3516,
    "output_sha256": "c1efcf072f579bbdece90d442dfd4d0f29332c871911e368239be76e6216e76c"
  },
  {
    "workload": "startup",
    "mode": "ordinary",
    "binary": "B",
    "seconds": 0.004297158680856228,
    "rss_kib": 3564,
    "output_sha256": "c1efcf072f579bbdece90d442dfd4d0f29332c871911e368239be76e6216e76c"
  }
]
```

Median startup: 0.004235 s; fastest workload: 22.2x startup.

## Acceptance and tradeoffs

The final candidate hash matches the ordinary `dev/preproc` binary. Recompute
all observations and budgets with:

```sh
python3 student.tests/pa4/verify_performance.py student.tests/pa4/final-audit-performance.md dev/preproc
```

Nested compiler work improves 13.90% and 12.35% in the two paired blocks, beyond
3.94% calibration noise. Its median latency falls from 0.108902 to 0.094080 s;
median peak RSS rises 236 KiB (4568 → 4804 KiB) because reusable capacities
remain with the expander. Telemetry reports 76,800 prescans, 19 task slabs,
600 argument-buffer growths and 1,200 prescan-output growths. Independent
allocator interception additionally proves zero allocation calls on a warmed
20,000-deep invocation; counters alone are not the profitability evidence.

Repeated argument use improves 5.74–5.78%, beyond 0.60% noise. The 16 MiB flat
workload improves 2.62–2.65%, beyond 1.49% noise. Other apparently positive
results do not consistently clear their calibration: no separate speedup is
claimed for 4 MiB flat text, ordinary C++, counters or literal locations.
Long chains regress 1.42–1.84% in both blocks, with 1.19% noise; their median
latency rises 0.467457 → 0.477037 s and RSS rises 74 KiB. This regression is
retained and fits the predeclared <=10% plus noise budget.

Host text including read-only data falls 1.66% (171378 → 168528 bytes).
Fourfold source growth takes 3.9663x latency and 3.0752x RSS. All paired
latency, memory, host text, work and scaling budgets pass. The fastest workload
is 22.2x median measured startup. The nested improvement justifies the bounded
capacity retention and disclosed helper-chain cost. PA4 generates no program:
generated executable runtime and text size remain N/A; no generated-code
improvement is inferred from allocation or token counts.

The same frozen entry A was compared to the intermediate `c558c57d2` binary in
[performance-pooled.md](performance-pooled.md), retaining all 120 workload and
eight startup observations. It independently showed a 13.60–13.87% nested
benefit beyond 4.71% noise. A subsequent allocator assertion identified the
remaining delimiter-index temporary; this final campaign includes its fix.
Both campaigns use identical input bytes/hashes and ordinary build flags.
Historical implementation campaigns remain in `performance*.md`; none was
removed or relabeled as a measurement of a different candidate.
