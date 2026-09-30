# PA24 handoff132 performance evidence

Frozen entry: `bcf1e3b5` (`before`); initial implementation: `db965a86` (`after`); final: `03eeff9e` (`final`).
Binaries and all observations: `/home/vishvananda/work/private/v4codex/artifacts/pa24-132`.

All experiments use fixed inputs and `-O0 --stats`, a single permitted CPU,
four A/A calibration samples and six wall-time ABBA blocks. Compilation and
execution are measured separately. Generated outputs are checked before timing
and at both runtime argument counts. All samples, including outliers, remain.

## Final compiler latency and peak RSS

| Workload | A median (s) | B median (s) | Paired B/A [range] | Peak KiB A / B | A/A range (s) |
|---|---:|---:|---|---|---|
| scalar | 0.5639 | 0.5756 | 0.9934 [0.9431, 1.1778] | 88076 / 88044 | 0.4982–0.6412 |
| floating | 0.8019 | 0.8313 | 1.0455 [0.9423, 2.2002] | 88916 / 88900 | 0.7167–0.8998 |
| wide | 1.0030 | 0.9344 | 1.0019 [0.8663, 1.0570] | 88344 / 88308 | 0.8136–1.0978 |
| tls | 0.9044 | 0.9027 | 0.9938 [0.9924, 1.1805] | 83780 / 83788 | 0.8959–0.9092 |

Each input has 4096 functions and 64 arithmetic operations per function. TLS
adds 4096 distinct storage objects plus loads/stores. Its A and B are identical
copies of the corresponding completed compiler: the entry implementation rejects this surface.
That experiment measures absolute semantic cost and noise, not a speedup.

Final scalar/floating/wide paired medians are 0.993 / 1.045 / 1.002. The
floating median is 4.5% slower; its full spread and calibration are retained.
The initial wide run was 2.6% slower, while the final paired median is nearly
unchanged. These noisy results do not establish a speedup. Required extra TLS
classification/dispatch stays linear and bounded, with no optional transform;
final peak RSS is effectively unchanged. All paired medians stay within the
inherited diagnostic 15% latency/RSS target.

The initial measurements below remain evidence, including their unfavorable
samples. Final accessor bodies now use ordinary per-function MIR, dumping and
encoding. This also removes a separate demand walk; it adds no executable bytes.
Compilation was remeasured with the final frozen compiler. The final binary's
eight runtime images are byte-identical to the measured images, with both input
counts rechecked (`final-runtime-identity.json`).

### Initial compiler measurements (retained)

| Workload | A median (s) | B median (s) | Paired B/A [range] | Peak KiB A / B | A/A range (s) |
|---|---:|---:|---|---|---|
| scalar | 0.9548 | 0.9708 | 1.0188 [0.8245, 1.0840] | 88032 / 88040 | 0.9259–1.1014 |
| floating | 0.6681 | 0.6630 | 0.9948 [0.7681, 1.0617] | 88904 / 88904 | 0.6615–0.7053 |
| wide | 0.7322 | 0.7434 | 1.0256 [1.0090, 1.1814] | 88344 / 88340 | 0.7211–0.8858 |
| tls | 0.5341 | 0.5488 | 1.0083 [0.7364, 1.2116] | 83800 / 83808 | 0.5226–0.5504 |

## Executable runtime and text

| Workload | A median (s) | B median (s) | Paired B/A [range] | Text bytes A / B |
|---|---:|---:|---|---|
| runtime | 0.7326 | 0.7187 | 0.9763 [0.9211, 1.0643] | 161 / 161 |
| memory-runtime | 0.5560 | 0.5715 | 1.0230 [0.8864, 1.1557] | 149 / 149 |
| floating-runtime | 0.9870 | 0.9843 | 1.0128 [0.9644, 1.0303] | 1522 / 1522 |
| pressure-runtime | 1.3376 | 1.3312 | 0.9976 [0.9867, 1.0016] | 557 / 557 |
| wide-runtime | 0.4072 | 0.4299 | 1.0315 [0.8824, 1.4403] | 308 / 308 |
| wide-numeric-runtime | 1.1601 | 1.1858 | 1.0126 [0.9941, 1.0580] | 2211 / 2211 |
| exception-runtime | 0.1691 | 0.1651 | 0.9902 [0.9582, 1.0172] | 726 / 726 |
| tls-runtime | 0.1736 | 0.1762 | 1.0084 [0.9727, 1.0626] | 247 / 247 |

All seven inherited executable pairs are byte-identical. Their runtime ratios
are noise rather than generated-code changes. Runtime peak RSS is 256 KiB for
every sample. The TLS pair is also identical (both from the initial completed compiler),
and matches the final compiler byte for byte.
It executes forty million accessor/increment calls at the timed input and
sixty million at the second checked input. Its roughly 0.175 s runtime and
247-byte text describe new required behavior, not an optimization benefit.

## Bounds, acceptance and architecture

- The immutable LowIR unit owns storage and wrapper identities. Dense unit maps
  carry those facts into per-function `tls_addr` MIR and typed image fixups.
  No text transport, mangled-name lookup, cloned semantic nodes or global retry.
- Selection/encoding remain linear apart from inherited O(E log E) edge ordering.
  TLS adds two global-layout passes, one wrapper classification per owner and a
  shared linear demanded-helper walk. Each accessor is emitted once through a
  typed Function, including the MIR dump, frame policy and ordinary encoding.
- Each TLS address uses one FS:0 load and one displacement LEA (at most 17 bytes);
  a declared accessor adds one return byte. Only its selected destination changes.
  Alignment padding is bounded by alignment-minus-one. The image adds an
  eight-byte initial-thread self pointer and one constant-size startup sequence.
- Standalone initial-thread storage is owned by the RW image. Host object TLS
  allocation/relocation and threading integration belong to later hosted stages.
  The current address form already follows the executing thread pointer.
- No optional optimization was added. Existing executable bytes/text stay fixed.
  The diagnostic zero optional-growth target is met. Correctness, course MIR
  bounds and comparison rules remain mandatory; the diagnostic 15% targets
  do not create extra exit gates or waive any required limit.
- Existing source/frontend template evidence remains applicable; `trace132.cpp`
  additionally executes a demanded template and TLS object through explicit
  LowIR/native tool boundaries. The integrated source driver and self-hosting
  remain later-stage work. Debug and view/native identity checks pass.

## Retained manifests

| Manifest | SHA-256 |
|---|---|
| `bench-scalar/observations.json` | `64be76eaf61441ccc1750a507405057c35961b4c2582c543104d289f43b45819` |
| `bench-floating/observations.json` | `b8f2794050c901e510df00eadd4b1ab3c84e22701b3ad9dac0638ac7c1f5b749` |
| `bench-wide/observations.json` | `837b61f02dde9468bc64f5dd5f6fbe8d1ebb1920950a22dab951957bb564eb3b` |
| `bench-tls/observations.json` | `ff3cacb1d8777fa7d7f2688785c77f52abe095313b4833e3a5921ac1a5696a8a` |
| `final-scalar-bench/observations.json` | `e36deda5f175f04d08d4613f486e1cd203c3e8bf9294bbd033db14fa0520e630` |
| `final-floating-bench/observations.json` | `b5fcda0721205fbc7d2ed87d463c5c710eddd86b6bb144a3cd1c7f9602bd64a2` |
| `final-wide-bench/observations.json` | `fe27163f55215ee6f141244242d70a8151cd10577e1e7695a3c986cd565c9df5` |
| `final-tls-bench/observations.json` | `fcccbb27fbefe503b8dd476f0469a52f15c939df1100a007a5d82e3059ac2e0a` |
| `final-runtime-identity.json` | `27c32e9da7f212b35736608204ad336dcf2335032c30624085d8c5fb3ecc7540` |
