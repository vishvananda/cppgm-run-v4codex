# Performance178 — accumulated PA29 / O0 audit

This audit measures required semantics across the complete reviewed range and the
fixes at its end. No optional optimization or speedup is claimed. Acceptance uses
spec §9 at PA29/O0; correctness, coverage and mandated capacities remain gates.

## Frozen comparisons

The [manifest](../student.tests/pa29/evidence178/manifest.json) binds the code tip,
binary/source/script hashes and all inputs. The cumulative comparison uses the
previous reviewed compiler (SHA `21589b947593…`, also implementation175 entry;
its intervening commit changes records only) and final SHA `022c1bc2829a1…`.
The owner comparison uses audit entry SHA `8e292c04f171…` and that same final.
Compilation and checked execution are timed separately; host linking is outside
both timers. Each paired workload/mode has AAAA calibration followed by six ABBA
blocks. All observations, phase counters and image hashes are retained:

- [Cumulative raw data](../student.tests/pa29/evidence178/cumulative-performance.json): 224 samples, four 2,400-specialization workloads with loops/calls/memory, floating point, exceptions or unused functions.
- [Owner raw data](../student.tests/pa29/evidence178/owner-performance.json): 504 paired samples for source invocation, excluded declarations/inline storage and selection initialization at 600/1200/2400 specializations; 48 final-only samples for corrected unknown-bound array demand.
- [Historical review](../student.tests/pa29/evidence178/historical-review.json): all 816 implementation175–177 observations preserved with original identities and classifications.

Common flags are `-O0 -c --stats`; owner flags add `-std=c++11`. Owner executions
use runtime seed 17 and varying loop inputs for 24 million operations. Every run
checks an independently computed checksum; declaration/bound cases also check
initialization counts. The common runner checks its fixed expected outputs.
Builds and correctness suites ran outside these timers. There is no affinity
pinning or hardware-counter requirement; external scheduling remains uncontrolled.

## Four-dimensional results

Paired ratios below are medians of six block ratios, not ratios of the displayed
unpaired medians. Brackets retain the full paired range. RSS is the maximum of
timed ABBA observations (calibration RSS remains in the raw data).

| Workload | Compile A/B median s | Compile B/A [range] | Compiler RSS A/B KiB | Runtime A/B median s | Runtime B/A [range] | Text A/B bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.1635/0.1642 | 1.0085 [0.9484–1.0284] | 29536/29356 | 0.0513/0.0510 | 0.9880 [0.9320–1.0220] | 151633/151633 |
| floating | 0.1618/0.1633 | 1.0083 [0.6874–1.0474] | 29500/29448 | 0.0480/0.0479 | 1.0018 [0.9971–1.1125] | 151474/151474 |
| exceptions | 0.1678/0.1660 | 0.9996 [0.9170–1.6559] | 29512/29900 | 0.2858/0.2586 | 0.9553 [0.8075–1.0070] | 151781/151781 |
| pruning | 0.3576/0.3528 | 1.0199 [0.9386–1.0696] | 35588/35684 | 0.0508/0.0508 | 0.9993 [0.8867–1.0061] | 151633/151633 |

| Workload | Compile A/B median s | Compile B/A [range] | Compiler RSS A/B KiB | Runtime A/B median s | Runtime B/A [range] | Text A/B bytes |
|---|---:|---:|---:|---:|---:|---:|
| source600 | 0.0631/0.0630 | 0.9966 [0.9787–4.3029] | 15460/15188 | 0.1294/0.1285 | 0.9934 [0.9670–1.0013] | 46360/46360 |
| source1200 | 0.1195/0.1189 | 0.9919 [0.9444–1.0064] | 23792/23848 | 0.1304/0.1300 | 0.9973 [0.9901–1.6006] | 92560/92560 |
| source2400 | 0.2449/0.2392 | 0.9824 [0.7166–1.0113] | 40108/40192 | 0.4241/0.4235 | 0.9984 [0.9750–1.0391] | 184960/184960 |
| declaration600 | 0.1102/0.1087 | 0.9911 [0.7676–1.0096] | 21412/21684 | 0.1278/0.1279 | 0.9992 [0.9856–1.0344] | 82177/82177 |
| declaration1200 | 0.2200/0.2198 | 1.0072 [0.9873–1.4861] | 35336/35500 | 0.1299/0.1310 | 1.0020 [0.9749–1.0174] | 163777/163777 |
| declaration2400 | 1.1729/1.1637 | 0.9726 [0.9080–1.1416] | 62944/63308 | 0.7678/0.7470 | 0.9827 [0.8624–1.0209] | 326977/326977 |
| selection600 | 0.2855/0.2643 | 0.9767 [0.8340–1.1325] | 23960/24020 | 0.3761/0.3647 | 0.9496 [0.8504–1.0117] | 84760/84760 |
| selection1200 | 0.4977/0.5224 | 1.0060 [0.8723–1.1613] | 40412/40440 | 0.7111/0.7360 | 1.0309 [0.9133–1.1315] | 169360/169360 |
| selection2400 | 1.1802/1.1538 | 0.9456 [0.8738–1.1751] | 73092/73432 | 1.0896/1.1260 | 1.0266 [0.9451–1.0809] | 338560/338560 |

| Workload | Compile A/A range s | Runtime A/A range s |
|---|---:|---:|
| memory | 0.1650–0.1950 | 0.0508–0.0509 |
| floating | 0.1605–0.1765 | 0.0477–0.0495 |
| exceptions | 0.1620–0.1665 | 0.2507–0.2565 |
| pruning | 0.3492–0.3577 | 0.0509–0.0524 |
| source600 | 0.0624–0.0644 | 0.1280–0.1300 |
| source1200 | 0.1171–0.1187 | 0.1290–0.1376 |
| source2400 | 0.2364–0.2392 | 0.4216–0.5496 |
| declaration600 | 0.1083–0.1115 | 0.1274–0.1282 |
| declaration1200 | 0.2172–0.2339 | 0.1278–0.1315 |
| declaration2400 | 1.1190–1.1753 | 0.7098–0.8413 |
| selection600 | 0.2482–0.2858 | 0.3468–0.3944 |
| selection1200 | 0.4596–0.4757 | 0.6252–0.6715 |
| selection2400 | 0.9785–1.1607 | 1.0065–1.1477 |

Cumulative compiler paired medians range 0.9996–1.0199; owner medians 0.9456–1.0072.
Peak compiler RSS grows at most 1.31% in the cumulative comparison and 1.27%
in the owner comparison. Required dependency edges account for linear additional
storage where inline initializers are actually demanded. Neither timing set
establishes a repeatable compiler regression or speedup; outliers remain in the
record. In particular, selection1200/2400 runtime paired medians rise 3.09%/2.66%.
[Section comparisons](../student.tests/pa29/evidence178/elf-comparison.json) verify
identical `.text`, `.rodata`, `.data` and `.eh_frame` bytes for all 13 paired
executables. All four common objects/executables are fully byte-identical.
Some owner file hashes differ outside those sections; full-file equality is not
claimed. The section evidence and overlapping calibration/block spreads do not
attribute those runtime changes to changed generated instructions.

## Corrected definition demand and scaling

Entry fails the corrected array workload, so no A/B speed ratio is valid for it.
Its rejection or failed result is retained under `entry_checks` in the owner data.
The final compiler must run each demanded initializer exactly once even when
`sizeof` first requests an unknown bound; runtime verifies the count before the
checked loop. Eight compiler and eight runtime observations per size are retained.

| Specializations | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| 600 | 0.2485 [0.2005–0.2951] | 18568 | 0.2209 [0.2042–0.2338] | 1792 | 79798 |
| 1200 | 0.4023 [0.3726–0.5376] | 31072 | 0.1879 [0.1748–0.1961] | 1788 | 158998 |
| 2400 | 0.9032 [0.8079–0.9733] | 54420 | 0.2313 [0.2178–0.2647] | 2088 | 317398 |

Launcher median 0.00291 s, range 0.00269–0.00349 s (all eight retained).
Owner compiler/runtime medians exceed startup costs. The common workloads are
shorter, so their calibration and unchanged images limit the inference.
[All-sample analysis](../student.tests/pa29/evidence178/scaling.json) checks every
final owner compilation against exact equations at the three sizes:

| Family | Parsed nodes | Occurrence nodes | LowIR instructions | Native text bytes | Initializer computations / demands / edges |
|---|---:|---:|---:|---:|---:|
| source | 15N+253 | 60N+253 | 15N+43 | 77N−80 | 0 / 0 / 0 |
| declaration | 28N+339 | 103N+339 | 23N+56 | 136N+337 | N / N / N |
| selection | 15N+370 | 167N+370 | 22.5N+43 | 141N−80 | 0 / 0 / 0 |
| bound | 34N+283 | 79N+283 | 22N+49 | 132N+358 | N / N / 2N |

Executable text is native text plus 240 bytes on these inputs. Substitution
frames, body transitions and native instructions are also checked in the data.
Counters demonstrate the measured work scales with actual nodes, facts and
dependency edges; wall-time variation is not used to prove asymptotic complexity.
The 2,400-function declaration/selection runtime increase at a fixed operation
count is an inherited O0 limitation, disclosed without an unsupported profiler
attribution. It is later optimizer work, not a new PA29 exit gate.

## Legality, profitability, budgets and acceptance

The fixes preserve required semantics: source-address relocations remain live,
initializer definitions own their evaluation context, constant-only queries do
not emit runtime objects, and defaults receive complete template frames before
projection. Bound completion records one definition computation and one typed
dependency traversal. Flat identity indexes deduplicate edges; no whole-registry
retry, optional fixed-point transform or extra retained optimization body exists.
Lowering visits declaration lifecycle arrays at their fixed extent and drains
the separate source-string byte queue. Work follows actual emitted operations.

Optional transform work and code-growth budgets remain zero for this PA29/O0
change. Required branch selection is legal only after the constant-expression
fact is established; discarded template arms are not emitted, while non-template
discarded statements retain required diagnostics. Effects, lifetimes, ABI and
unwind facts remain conservative without a proof. The integrated MIR records
a 48-byte frame, a Guard slot, condition spill and saved return, preserving rbx;
no register-allocation profit is inferred from IR size. No analysis invalidation
is needed beyond newly published facts at their canonical owner.

The inherited blanket 15% latency/RSS and zero-growth targets remain diagnostic
under spec §9. Implementations175–177 already applied this classification; this
audit rechecks their raw evidence rather than imposing their historical misses
as permanent failures. All measurements are preserved. Necessary semantic costs
are documented, with no established avoidable regression on equivalent correct
inputs and no unprofitable optional transform to retain.

Mandated capacities remain: 1,048,576 generated elements, 1,000,000 constexpr
steps, depth 512, packed source-site identity below 2^31, native frame/data limit
0x70000000 and alignment 4096, plus unchanged course timeouts. Heavy hosted
runtime, broader optimization levels and self-hosting belong to later stages.
No capacity, correctness expectation, failure comparison or coverage was relaxed.

Reproduction uses the manifest binaries and frozen inherited inputs (their
generators and input hashes remain in evidence175–177):

```sh
python3 student.tests/pa27/performance147_common.py OUT/cumulative-performance RANGE_BASE FINAL
python3 student.tests/pa29/performance178.py OUT/owner-performance ENTRY FINAL
python3 student.tests/pa29/analyze178.py OUT
python3 student.tests/pa29/test178.py OUT/controls FINAL
python3 student.tests/pa29/bounds178.py OUT/bounds
python3 student.tests/pa29/inspect178.py OUT/inspection178
python3 student.tests/pa29/validate178.py OUT  # seed OUT/entry.json from evidence178
```
