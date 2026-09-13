# PA14 explicit-initializer audit evidence

Entry: `2ab55111` (body publication). Frozen A/B binaries, flags, CPU, sources, backend, telemetry, warmups and every sample are in [initializer-performance.json](initializer-performance.json) and [initializer-noise.json](initializer-noise.json). Each campaign uses one warmup per binary, four A/A observations and two ABBA blocks. The main campaign contains 27 compiler workloads and ten executables; calibration repeats six compiler workloads and the new executable. **616 observations; 16,996 cumulative**, with no concurrent builds, tests or verifiers during timing.

All benchmark LowIR is equivalent under the unchanged course comparison; all ten complete executable hashes are identical across A/B. Runtime work includes loops, calls, memory, floating point, virtual dispatch, destruction, defaults and initialization. The new executable checks eight million iterations, sixteen million conversion effects, zero surviving objects and a checksum of 48,000,000. The supplied PA8 backend is only an observation tool. PA14 production ends at O0 LowIR; native selection, ELF writing and self-hosting belong to later stages.

Compiler `.text`: **1,348,742 → 1,368,518 bytes**, +19,776 (+1.466%). Entity remains 112 bytes and the other 16 measured node/container records are unchanged. Analyzer grows **6,376 → 6,520 bytes** for four TU-owned flat indexes and two counters; [24 transitive headers and layouts](initializer-layout.json) are frozen. Selected calls/conversions stay in existing semantic stores. Per-use object/lifetime facts remain concrete.

For the N/K/M/Q initializer workload, source recipe work is **2M**, independent of unrelated declarations N, specializations K and repeated calls Q. Concrete applications are **2KM**. At K=128 and M=8, candidate visits fall **2,176 → 144**; at K=1, 17 → 17, and at Q=4,000, 4,016 → 4,016. This proves bounded sharing, not a runtime improvement. Every demanded function has one body and one lifetime check.

## Compiler observations

| Campaign / workload | Paired B/A change (%) | A/A seconds | Median RSS A → B (KiB) |
|---|---:|---:|---:|
| main / virtual-runtime | +1.91 / -1.35 | 0.0061–0.0064 | 5384 → 5460 |
| main / destructor-runtime | +0.47 / -0.88 | 0.0060–0.0062 | 5348 → 5388 |
| main / body-run-4000-128 | +1.87 / +1.71 | 0.6092–0.6338 | 94756 → 94544 |
| main / declaration-instances-1000 | -0.44 / -4.91 | 0.5510–0.6539 | 78426 → 78328 |
| main / demand-uses-1000-128-4 | -13.33 / +1.90 | 3.4838–8.0349 | 346138 → 346192 |
| main / demand-runtime | -1.12 / -0.38 | 0.0185–0.0203 | 5564 → 5562 |
| main / region-runtime | +3.09 / +21.43 | 0.0140–0.0141 | 5476 → 5680 |
| main / calls-runtime | +1.18 / -15.45 | 0.0091–0.0102 | 5288 → 5236 |
| main / memory-runtime | +3.54 / +4.08 | 0.0089–0.0094 | 5206 → 5296 |
| main / floating-runtime | +1.11 / -2.11 | 0.0093–0.0111 | 5428 → 5446 |
| main / default-16000-1-1-1 | -2.00 / +1.11 | 0.1679–0.1792 | 15642 → 15656 |
| main / default-64000-1-1-1 | +2.43 / +1.55 | 0.6718–0.6925 | 47262 → 47286 |
| main / default-16000-512-1-1 | -2.18 / -5.78 | 0.2428–0.2664 | 22292 → 22328 |
| main / default-16000-1-32-1 | -4.79 / +7.33 | 0.1723–0.2317 | 15670 → 15686 |
| main / default-16000-1-32-4000 | +3.10 / +1.10 | 0.2904–0.3057 | 24808 → 24752 |
| main / default-16000-1-1-4000 | +0.36 / -3.61 | 0.2815–0.3308 | 24914 → 24916 |
| main / default-runtime | -6.55 / +2.86 | 0.0096–0.0110 | 5254 → 5290 |
| main / return-16000-1-8-1 | +2.01 / -0.69 | 0.1691–0.2109 | 15632 → 15576 |
| main / return-64000-1-8-1 | -4.82 / +1.78 | 0.6720–0.7204 | 47300 → 47318 |
| main / return-16000-128-8-1 | -2.09 / -0.91 | 0.2242–0.2327 | 21058 → 20996 |
| main / return-16000-1-8-4000 | -2.17 / +10.12 | 0.3508–0.3656 | 30356 → 30390 |
| main / return-runtime | -19.09 / -4.89 | 0.0089–0.0101 | 5270 → 5382 |
| main / init-16000-1-8-1 | +0.93 / +2.35 | 0.2112–0.2464 | 15570 → 15576 |
| main / init-64000-1-8-1 | +0.74 / +1.55 | 0.3166–0.3435 | 47326 → 47288 |
| main / init-16000-128-8-1 | -0.32 / -0.74 | 0.1287–0.1305 | 24424 → 24418 |
| main / init-16000-1-8-4000 | -0.49 / -1.26 | 0.1561–0.1587 | 26756 → 26200 |
| main / init-runtime | +0.35 / -0.67 | 0.0059–0.0063 | 5270 → 5408 |
| repeat / body-run-4000-128 | +0.48 / +0.12 | 0.6023–0.6767 | 94948 → 95418 |
| repeat / declaration-instances-1000 | +4.34 / -1.70 | 0.5573–0.6057 | 78454 → 74778 |
| repeat / demand-uses-1000-128-4 | -0.04 / +0.05 | 3.0229–3.3273 | 346188 → 346196 |
| repeat / init-16000-128-8-1 | -1.20 / -0.25 | 0.1308–0.1321 | 24442 → 24340 |
| repeat / init-16000-1-8-4000 | +0.59 / -0.48 | 0.1536–0.1568 | 26178 → 26688 |
| repeat / init-runtime | +0.51 / -0.32 | 0.0060–0.0062 | 5226 → 5408 |

The main campaign experienced large timing variation: demand A/A widened to 3.484–8.035 seconds, versus 3.023–3.327 on repeat. The cause of host variation is unknown; no concurrent agent build/test/verifier explains it. The main body changes (+1.87/+1.71%) repeat at +0.48/+0.12%; demand repeats at −0.04/+0.05%. The initializer K=128 pairs improve by 0.32/0.74%, repeated at 1.20/0.25%, but those small changes do not support a robust latency claim. The Q=4,000 RSS change reverses from −556 to +510 KiB. No broad latency or memory improvement is claimed.

## Executable observations

| Campaign / executable | Paired runtime change (%) | Native payload bytes A = B |
|---|---:|---:|
| main / virtual-runtime | +0.45 / -0.01 | 2360 |
| main / destructor-runtime | -0.43 / -0.99 | 2072 |
| main / demand-runtime | -0.82 / +0.87 | 261 |
| main / region-runtime | -1.18 / -3.06 | 206 |
| main / calls-runtime | -1.44 / +19.61 | 206 |
| main / memory-runtime | +0.42 / -2.45 | 434 |
| main / floating-runtime | -0.58 / -0.14 | 230 |
| main / default-runtime | -6.87 / -0.58 | 300 |
| main / return-runtime | +0.21 / +10.44 | 444 |
| main / init-runtime | -0.64 / +0.50 | 1456 |
| repeat / init-runtime | -0.00 / -0.49 | 1456 |

The main call-runtime +19.61% and return-runtime +10.44% pairs occur on byte-identical executables and remain in the record. The new initializer runtime is −0.64/+0.50%, repeated at −0.00/−0.49%. No runtime optimization or profit claim is made. Native payload size denotes the sectionless backend executable payload after its ELF entry, not an invented ELF .text section.

## Acceptance and correctness

PA14/O0 mandates semantic correctness and bounded work, with no numeric frontend latency, RSS or compiler-text ceiling. These checks add necessary definition-time legality and retain source decisions for concrete consumers. No optional runtime transform is introduced. Each source/key is checked once, source aggregate mapping is linear in consumed fields/clauses, scratch candidate vectors are bounded by actual arguments/candidates, and generated code growth is zero on comparable correct inputs. Historical self-selected targets remain diagnostics; all observations, mandated limits and comparison rules remain intact.

[Validation](initializer-validation.json) contains 67 successful checks: both required gates, 349 exact checkpoint/current and release/sanitizer comparisons, 35 existing native programs, 82 new initialization controls and 64 statement controls per build, inherited query/default/lifecycle/virtual/demand checks, and PA13 ABI/lifetime controls. It freezes 227 source/control files and all 1,266 unchanged fixture/reference files. The initial personal harness had an artifact-name collision; [the refresh harness](initializer_refresh_validation.py) preserves that manifest and script, gives the positive case a unique name, and reruns both 82-case groups on unchanged binaries. The other 65 check results remain valid.

[Source proofs](initializer-proofs.json) show **71 entry-accepted invalid programs**, eleven valid native controls and one valid constexpr control rejected at entry. N3485 [dcl.constexpr]/9 requires const qualification on the source object; preserving it fixes the source/concrete type mismatch. Four additional demand controls run under entry, release and sanitizer compilers and distinguish unused constructor/default bodies from required definitions ([temp.inst]/10). No reference output changed. Compiler agreement is not the proof.

This checkpoint records the explicit-initializer producer/consumer path, not a whole-stage completion certificate. Four reduced remaining errors in the proof manifest cover default initialization, discarded callable expressions and query-only braced returns. Further reduced copy/list-mode and partial aggregate-mapping defects, query publication/reuse, fixed condition recipes and the remaining full-stage review stay open in [the audit](../../pa14/audit.md).
