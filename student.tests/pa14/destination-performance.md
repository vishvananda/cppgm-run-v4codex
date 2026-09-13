# Conversion destination ownership

This continuation removes the redundant destination metadata identified by the
[mode campaign](modes-performance.md). Both frozen implementations are correct
on the comparable inputs. Constructor and conversion-function recipes now mark
whether they describe a source recipe, an actual temporary, or an existing
destination. The initialized object owns storage and lifetime; source prvalue
temporaries, required copies, access/deletion checks and destructor/virtual ABI
demands retain their separate owners. No executable optimization was added.

The [main data](destination-performance.json) contain 37 compiler and 12 native
workloads; the [repeat](destination-noise.json) contains 11 compiler and three
native workloads. Each campaign has one warmup per binary, four A/A observations
and two ABBA blocks: **882 observations**, **18,606 cumulative**. All binary,
input, flag, output, backend and harness identities are frozen by hash. Timing
ran without concurrent agent builds, tests, verification or compression. The
new borrowed-result runtime performs eight million volatile-loop iterations,
checks eight million required lvalue copies, zero live objects and checksum
28,000,000. Compiler and executable measurements are separate.

A SHA-256: `669c8cb9d6e9e8d1fb5a090445ecf657438c77ea0473b9ed4852ec610eec8d19`.
B SHA-256: `cd924522f01a357ba41c67ff7f349324be297e55b98f82dac12e86220cc3044c`.
Compiler `.text`: 1,369,862 → 1,371,014 bytes (**+1,152 / 0.0841%**).
All 18 main record sizes are unchanged, including Analyzer at 6,552 bytes.
ConversionObject/Conversion/ValueInitialization/UserConversion remain
52/28/8/80 bytes; the use markers consume existing padding.

| Workload | Main paired compiler latency change | Repeat paired change | Main median peak RSS A → B (KiB) | Repeat RSS A → B (KiB) |
|---|---|---|---|---|
| Constructor copy, K=128, M=8 | +2.18%, −1.42% | −0.66%, −1.35% | 24,388 → 24,096 | 24,656 → 24,592 |
| User conversion copy, K=128, M=8 | +1.54%, +0.73% | +3.06%, +1.42% | 25,408 → 25,156 | 25,424 → 25,096 |
| Constructor copy, N=64,000 | −0.24%, +0.81% | +0.12%, +2.45% | See raw data | See raw data |
| User conversion copy, N=64,000 | +0.29%, +2.67% | +0.78%, +1.42% | See raw data | See raw data |
| Constructor copy, Q=4,000 | −0.52%, +0.29% | +0.94%, +0.58% | See raw data | See raw data |
| Body, 4,000 statements × 128 instances | +6.63%, +1.18% | −0.22%, −5.64% | See raw data | See raw data |
| 1,000 declaration instances | +0.41%, −9.65% | −9.62%, −0.75% | See raw data | See raw data |
| Demand, 1,000 × 128 × 4 | −3.52%, −0.09% | −0.75%, +0.33% | See raw data | See raw data |

A/A wall ranges include constructor K=128: 0.1300–0.1328 seconds, repeated
0.1293–0.1328; user K=128: 0.1331–0.1342, repeated 0.1341–0.1360; body:
0.6012–0.6242, repeated 0.5978–0.6126; declarations: 0.5546–0.6988, repeated
0.5453–0.5623; and demand: 2.9914–3.1556, repeated 3.0278–3.1725.
All individual observations, RSS, context switches, medians and spreads remain
in the linked data. No compiler latency improvement is claimed. The positive
user-copy latency changes are disclosed; selection and per-use conversion work
are unchanged and no additional scan, search or optional transform explains
them. Their precise timing cause is unisolated.

Source recipes remain **2M**, concrete uses **2KM**, independent of unrelated N
declarations and Q repeated calls. Both constructor and user-conversion shapes
remove exactly **KM entities and scopes**. At K=128/M=8, constructor entities
fall 19,614→18,590 and scopes 1,549→525; user-conversion entities fall
19,622→18,598 and scopes 1,555→531. Each retains 1,032 required conversion
records, 16 source recipes and 2,048 uses. The removed destination allocations
are the identified avoidable cost. RSS decreases on both affected shapes in
both campaigns, though cross-campaign RSS does not establish that every byte
of the earlier increase has disappeared. Real temporary and typed conversion
records remain necessary correctness costs.

All **12 executable images are byte-identical**. Text payloads are 2360, 2072,
261, 206, 206, 434, 230, 300, 444, 1456, 1456 and 1704 bytes. The metric is the
payload after ELF entry for the supplied sectionless backend, as documented in
the shared harness. Copy runtime paired changes are −4.59%/+0.57%, repeated
−0.85%/−27.88%; initializer runtime +0.59%/−0.63%, repeated +0.07%/−26.79%;
user-copy runtime +0.19%/−1.62%, repeated +0.35%/+0.06%. Preserved outliers
include default runtime +61.02%/−25.28% with A/A 0.6612–0.7542 seconds,
return runtime +5.58%/+3.95% with A/A 1.7966–2.4000, and calls runtime
−8.06%/+0.49% with A/A 0.5042–0.6882. Their wall-stall cause is unknown.
Identical executable bytes provide no basis for attributing these changes to
the compiler correction. No native runtime improvement is claimed.

PA14 O0 acceptance requires correct, bounded source/key/use work and removal of
avoidable ownership duplication. The correction satisfies those requirements;
it adds no code-growth policy, search budget or optimization pass. Conversion
recipes remain immutable; concrete preparation is terminal per use and keeps
its target, source occurrence and access environment. Lowering requires the
correct use marker and destination and consumes retained choices. No global
cache invalidation or broader demand was introduced. There is no mandated
numerical latency/RSS/text cap at this stage. Native selection, allocation,
ELF encoding, optimized levels and self-hosting remain later-stage owners.
Historical diagnostic targets and noisy measurements are preserved without
creating additional exit gates. The five source default/query defects remain
required work and prevent final audit acceptance.

[Validation](destination-validation.json) records **73 passing groups**, both
required gates (**1935/1935**, all 14 stages), 349 entry/current and sanitizer
comparisons, inherited demand/lifecycle/virtual/ABI controls, both builds' 75
mode, 82 initializer and 64 statement controls, and six new checked native
destination programs per build. Two inspection scenarios per build distinguish
recipe/destination/temporary records and repeat finish 10,000 times with stable
counts. All 1,266 fixture/reference hashes are unchanged. No reference was
corrected. The lifecycle basis is N3485 [class.temporary]/3–5 and
[class.copy]/31–32; observation by another compiler is not the proof.

The first validation stopped while linking a sanitizer inspection executable
because storage was full. Its failed log and partial manifest remain intact.
Eight completed untimed probes were losslessly compressed with original and
archive hashes; the resume helper verified the identical harness and binaries,
reused 51 exact successful commands and ran the failed/unrun 22. Compression and
validation finished before timing. No successful result replaced a failed run
without recording the recovery. Raw artifacts are under
`$RALPH_ARTIFACT_DIR/pa14-final-audit/`.
