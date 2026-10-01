# PA29 handoff159 performance evidence

Code: `5d2b1657`; entry: `044d9627`. Required semantic capability at O0; no new optional optimization and no speedup claim. [Ownership and complexity](implementation159.md) accompany the measurements.

## Frozen protocol

[Manifest](../student.tests/pa29/evidence159/manifest.json) records compiler/script hashes, build flags, platform, CPU, affinity and all raw-data hashes. Host build uses `-std=gnu++11 -Wall -O3`; measured flags are `-O0 -c --stats`. CPU affinity is 0; `/usr/bin/time` supplies peak RSS. Host g++ only links generated objects. Compilation and checked execution are timed separately. No build/test process overlaps timing; external host scheduling and light bookkeeping remain uncontrolled.

| Compiler | Bytes | SHA256 |
|---|---:|---|
| A | 3935752 | `9ed4d6c3164faee5c4ba7f4478ca45a0cd318b28f051be5d5c88446b09dc1ff7` |
| preliminary_B | 3944808 | `63b34bb8a028dc1bbd1d0b57ef86dc6db9c0ae5024c66072cc89f98e76d187f9` |
| B | 3944808 | `8ce2c5e6f4c4f5eb45ea49d86667f49f2a4155dd97eeeb32f5cf08cf1491ec33` |

The preliminary binary predates the source-prvalue destructor fix. Its [first common](../student.tests/pa29/evidence159/preliminary-common.json), [confirmation common](../student.tests/pa29/evidence159/preliminary-common-confirm.json) and [affected](../student.tests/pa29/evidence159/preliminary-affected.json) observations are retained but do not certify the final endpoint. The final binary has two complete common and affected runs; the first had large timing variation, so one confirmation was made without changing binaries, inputs or flags. No observations were removed.

## Equivalent common workloads

Each common experiment retains 224 observations: an A/A calibration block then six ABBA blocks for four workloads in both modes. Inputs preserve the inherited checksums and 2,400 demanded templates; loops consume runtime argc. Paired ratios divide the two B observations by the two A observations in each block. Objects and executables are byte-identical between A and B for every workload in both runs. Runtime differences therefore cannot establish code improvement or degradation.

### First final run

[All observations](../student.tests/pa29/evidence159/common-first.json). Times are sample medians, RSS is maximum KiB; brackets are the full paired-block range.

| Workload | Compile A / B s | Paired compile B/A [range] | Compiler RSS A / B | Runtime A / B s | Paired runtime B/A [range] | Text bytes A = B |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.59966 / 0.56354 | 1.0051 [0.8986, 1.0585] | 29380 / 29264 | 0.08488 / 0.08222 | 0.9504 [0.9039, 1.0104] | 151633 |
| floating | 0.38990 / 0.39866 | 1.0207 [0.9828, 1.0590] | 29724 / 29660 | 0.10338 / 0.10214 | 1.0138 [0.9150, 1.1075] | 151474 |
| exceptions | 0.31822 / 0.32011 | 0.9538 [0.8533, 1.1492] | 29700 / 29600 | 0.46677 / 0.44748 | 0.9779 [0.6968, 0.9980] | 151781 |
| pruning | 0.50415 / 0.58609 | 1.0536 [0.8632, 1.1821] | 35292 / 35192 | 0.13373 / 0.12922 | 0.9858 [0.9568, 1.1664] | 151633 |

| Workload | A/A compile range s | A/A runtime range s | Runtime RSS A / B KiB |
|---|---:|---:|---:|
| memory | 0.29538–0.36600 | 0.08000–0.09240 | 1760 / 1764 |
| floating | 0.32393–0.40691 | 0.09632–0.10263 | 1760 / 1756 |
| exceptions | 0.34188–0.52468 | 0.41448–0.42623 | 3936 / 3936 |
| pruning | 0.74576–0.85313 | 0.12074–0.12912 | 1760 / 1760 |

### Final confirmation

[All observations](../student.tests/pa29/evidence159/common-performance.json). Times are sample medians, RSS is maximum KiB; brackets are the full paired-block range.

| Workload | Compile A / B s | Paired compile B/A [range] | Compiler RSS A / B | Runtime A / B s | Paired runtime B/A [range] | Text bytes A = B |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.16263 / 0.16267 | 1.0095 [0.9828, 2.0600] | 29912 / 29776 | 0.05793 / 0.05882 | 1.0199 [0.9775, 1.1991] | 151633 |
| floating | 0.24676 / 0.25460 | 1.0150 [0.6750, 1.0743] | 29704 / 29540 | 0.05788 / 0.05609 | 0.9957 [0.9389, 1.0388] | 151474 |
| exceptions | 0.16710 / 0.20824 | 1.0021 [0.9688, 1.3088] | 29540 / 29452 | 0.25596 / 0.25815 | 0.9967 [0.9237, 1.2664] | 151781 |
| pruning | 0.19523 / 0.19573 | 1.0033 [0.9873, 1.0472] | 35500 / 35364 | 0.05283 / 0.05240 | 0.9893 [0.8444, 1.0029] | 151633 |

| Workload | A/A compile range s | A/A runtime range s | Runtime RSS A / B KiB |
|---|---:|---:|---:|
| memory | 0.16331–0.19598 | 0.05781–0.06432 | 1756 / 1760 |
| floating | 0.18574–0.20111 | 0.05163–0.05839 | 1764 / 1764 |
| exceptions | 0.26108–0.26348 | 0.25117–0.25923 | 3932 / 3936 |
| pruning | 0.19572–0.28322 | 0.05217–0.05266 | 1756 / 1760 |

The final confirmation paired compiler medians are 1.0021–1.0150, but several blocks have large outliers. The first run gives 0.9538–1.0536. This is insufficient for a small speed claim, and all spread is disclosed. Common-workload semantic and native work counts are unchanged; the new trait work counters are zero. No new pass, retry or body demand explains the variable timing. Compiler RSS is slightly lower in these samples; no memory improvement is claimed.

## New trait capability costs

The entry compiler rejects these inputs; failure latency is not an equivalent correct baseline. Each final experiment retains eight compile and eight runtime observations at each size plus six launcher calibrations. Every specialization contributes to a checked checksum; 60 million runtime calls consume argc.

### First final run

[All observations](../student.tests/pa29/evidence159/affected-first.json).

| Specializations | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| 600 | 0.31233 [0.27521, 0.57224] | 26792 | 0.54906 [0.50350, 0.67810] | 1764 | 42146 |
| 1200 | 0.64423 [0.61676, 0.70402] | 46248 | 0.59271 [0.56584, 0.64203] | 1756 | 84146 |
| 2400 | 1.09129 [0.59542, 2.33377] | 85592 | 0.29082 [0.28979, 0.29234] | 1764 | 168146 |

### Final confirmation

[All observations](../student.tests/pa29/evidence159/affected-performance.json).

| Specializations | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| 600 | 0.12184 [0.11956, 0.12666] | 26812 | 0.29543 [0.29321, 0.29863] | 1756 | 42146 |
| 1200 | 0.24737 [0.24352, 0.25459] | 46284 | 0.29505 [0.29271, 0.29923] | 1756 | 84146 |
| 2400 | 0.50607 [0.49752, 0.53317] | 85468 | 0.29269 [0.29141, 0.29660] | 1764 | 168146 |

| Specializations | Class completions | Body transitions | Type-query work | Legacy type / member work | Native instructions |
|---|---:|---:|---:|---:|---:|
| 600 | 1200 | 600 | 3615 | 2400 / 1800 | 8453 |
| 1200 | 2400 | 1200 | 7215 | 4800 / 3600 | 16853 |
| 2400 | 4800 | 2400 | 14415 | 9600 / 7200 | 33653 |

Confirmation launcher median is 0.00448 s [0.00427, 0.00478]. Compilation and runtime dominate startup. Doubling demand approximately doubles compile time and incremental memory; work counters are linear. Source and result destruction checking is necessary semantic work, with no unrelated template body demand.

## Acceptance, budgets and limitations

PA29 accepts the bounded cost of required language/hosted semantics. There is no optional transform to justify by runtime profit or remove: newly introduced optional optimization work and code-growth budgets are **zero**. Compiler binary growth is 9056 bytes; common generated text growth is zero. New-capability text grows with emitted functions, as shown above. Existing constexpr million-step/depth-512 limits, frame/alignment constraints and course timeouts remain unchanged.

No structural performance regression is hidden by the noisy timings: common input work and output bytes are equal, caches remain bounded by demanded identities, and the new workload grows linearly. Measurements do not prove a percent-level latency guarantee; none is mandated for this correctness increment. Historical blanket 15% latency/RSS and zero-growth targets remain self-selected diagnostics under spec §9, not permanent gates. Earlier evidence is preserved. O1–O3 allocation/loop work belongs to PA32/33 and self-hosting to PA34. PA29 remains incomplete at 321/403; accepting this group does not accept the remaining stage failures.
