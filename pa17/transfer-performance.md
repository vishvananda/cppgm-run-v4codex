# PA17 transfer performance — loop 57

Frozen entry `69152966` versus code `794b150e`. Complete sources, hashes,
preflights, telemetry and every observation are in
[the campaign JSON](../student.tests/pa17/transfer-performance.json).
The [interrupted campaign](../student.tests/pa17/transfer-performance-before-fixed-receiver.json)
is preserved: its fixed-base template preflight exposed a layout-demand defect,
which was corrected before the complete campaign. It is not acceptance evidence.

Host build: g++ `-std=gnu++11 -Wall -O3`, course test runner enabled. Student
flags: `--emit-lowir -O0`; CPU affinity 31; serial timing, with no concurrent
build or test campaign. One warmup each, four A/A samples, then four ABBA blocks.
Preflight `--stats --validate-lowir` is outside timing. Report median within-block
B/A and full paired spread, wall-time ranges and maximum RSS. Runtime sources use
volatile loop bounds and checked live results. Each executable is checked before
timing; changed compiler-scale cases repeat those checked semantic shapes.

The supplied O0 backend is used only by validation, outside this compiler.
Bundle `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, SHA-256
`c3bae4acf3243d5a2fd6a15ef00e2d75d11a7d82715b1e4771f55165d0542490`.
It emits sectionless ELF: the text metric below is code **plus alignment**,
calculated as executable payload after the entry minus typed global bytes.
JSON also retains full payload, data and file sizes. Own native encoding and
self-hosting remain later-stage obligations.

| Compiler | SHA-256 | .text bytes |
|---|---|---:|
| Entry | `8f8dfd898edc50a919518cda43968b08e09f1e4255d128d391cfdef15c2628d7` | 1,777,798 |
| Final | `9104a568cf6bae039e37a213918f273d370badcf2d16e25bd0224fc6da064fae` | 1,780,870 |

Compiler text grows 3,072 bytes (0.173%). No new optional optimization pass is
introduced. Base reachability/layout records replace repeated path reconstruction;
the wide-array policy retains at most eight explicit lanes (at most 24 scalar
conversion/address/store instructions per expanded initializer). Larger arrays
retain bulk-copy/loop fallbacks. These structural work/growth bounds remain in
force. No mandated PA17/O0 numeric latency, RSS or executable-size ceiling exists.
The inherited +15%, +16 MiB and 5.5× targets are diagnostic, as accepted in the
previous audit; this campaign does not alter them or waive correctness/coverage.

Compiler observations, milliseconds and KiB:

| Input | A ms [range] | B ms [range] | Paired B/A [range] | A/A range ms | Peak RSS A/B |
|---|---:|---:|---:|---:|---:|
| common-partials-1500 | 119.41 [108.14–172.29] | 129.81 [115.35–161.66] | 1.0423 [0.9524–1.3587] | 141.28–180.11 | 22,072/22,404 |
| common-partials-6000 | 495.81 [485.84–546.86] | 500.01 [477.00–678.87] | 1.0199 [0.9654–1.1370] | 478.81–513.62 | 72,884/73,116 |
| common-loop-float-1500 | 166.35 [159.03–184.44] | 180.54 [174.39–209.05] | 1.0851 [1.0492–1.1205] | 168.09–191.64 | 30,080/30,248 |
| common-loop-float-6000 | 785.34 [684.72–1376.06] | 815.35 [728.06–1349.56] | 0.9210 [0.7433–1.4544] | 658.76–1013.45 | 104,092/104,204 |
| specialized-transfer-600 | 134.61 [122.26–188.30] | 141.82 [122.17–581.35] | 1.0853 [0.8719–2.1018] | 141.27–154.48 | 24,196/24,476 |
| qualified-path-600 | 84.88 [80.72–103.66] | 83.81 [80.69–115.90] | 1.0489 [0.8955–1.0827] | 75.03–82.67 | 15,384/16,216 |
| wide-array-600 | 46.30 [41.64–49.54] | 44.82 [41.43–61.12] | 1.0046 [0.9602–1.1395] | 46.43–328.15 | 10,784/10,684 |
| specialized-transfer-2400 | 526.52 [466.10–913.10] | 487.07 [478.53–549.38] | 0.9729 [0.7256–1.0272] | 519.47–907.97 | 80,080/81,152 |
| qualified-path-2400 | 286.86 [276.06–375.10] | 309.11 [283.85–795.19] | 1.0322 [0.9372–1.9313] | 278.85–302.63 | 44,476/46,800 |
| wide-array-2400 | 138.03 [134.93–178.86] | 140.58 [133.47–740.54] | 1.1087 [0.8505–3.1116] | 144.40–169.50 | 25,976/26,144 |

The largest additional peak RSS is 2,324 KiB in the 2,400-class qualified-path
case, which now retains explicit paths and qualifier boundaries. Its paired
compiler cost is about 3%. Wide-array-2400 observes +11% compiler wall time;
individual stores require additional lowering/output work. This is disclosed,
not a claim of faster compilation. Common-case spreads and A/A calibration do
not support a general compiler speed claim. Tiny runtime-source compiler times
are retained in JSON but are startup-sensitive.

Runtime observations (milliseconds), including generated code and data size:

| Input | A ms [range] | B ms [range] | Paired B/A [range] | A/A range ms | Code+alignment A/B | Data A/B |
|---|---:|---:|---:|---:|---:|---:|
| runtime-calls | 405.59 [379.13–583.61] | 400.98 [374.34–493.68] | 0.9466 [0.9048–1.0025] | 370.77–410.72 | 206/206 | 0/0 |
| runtime-memory | 220.01 [213.97–229.42] | 231.85 [218.71–298.64] | 1.1083 [1.0143–1.1734] | 227.64–271.93 | 434/434 | 0/0 |
| runtime-floating | 260.04 [257.35–281.86] | 267.37 [259.04–277.53] | 1.0299 [0.9777–1.0427] | 255.66–264.75 | 230/230 | 0/0 |
| runtime-sparse-transfer | 454.91 [436.78–553.43] | 164.90 [161.44–184.22] | 0.3612 [0.3476–0.3867] | 448.06–456.78 | 236/234 | 0/0 |
| runtime-qualified-path | 166.30 [162.75–322.13] | 165.56 [162.93–175.69] | 1.0078 [0.6669–1.0410] | 162.13–164.58 | 230/230 | 0/0 |
| runtime-wide-array-2 | 123.22 [122.30–128.33] | 122.22 [121.23–173.81] | 1.0130 [0.9888–1.1722] | 123.61–132.16 | 240/225 | 16/0 |
| runtime-wide-array-8 | 130.52 [129.94–133.56] | 126.30 [124.89–132.75] | 0.9672 [0.9622–0.9852] | 128.98–131.35 | 264/273 | 64/0 |
| runtime-wide-array-9 | 636.67 [627.52–640.13] | 637.68 [636.10–739.97] | 1.0067 [0.9999–1.0789] | 634.17–638.89 | 264/264 | 72/72 |

Sparse memberwise transfer is repeatably faster (paired ratios 0.348–0.387) with
two fewer code bytes. Both implementations produce the correct checked values;
no failed-correctness baseline is used for this claim. Wide-array-8 improves
about 3.3% in all four blocks, adds nine code bytes and removes 64 data bytes.
Wide-array-2 has no demonstrated runtime benefit; it removes 15 code bytes and
16 data bytes while preserving the course-required direct initializer shape.
Qualified-path machine code is byte-identical despite preserving both semantic
adjustments in LowIR. Calls, memory, floating and wide-array-9 binaries are also
identical; their timing differences therefore reflect execution noise, including
the disclosed +10.8% memory observation. No runtime-profit claim uses those
unchanged cases.

For 600→2,400 qualified receivers, final reachability paths/edge visits are
1,202→4,802 and cache hits are 2,400→9,600. No layout is demanded by candidate
viability; concrete layout totals are cached separately. This directly supports
linear growth in distinct queried pairs/edges. Other owner counters, memory,
source/IR bytes and phase times are retained. Correctness costs and later backend
constraints are disclosed under spec.md §9; no unsupported performance gate is
added to the implementation handoff.
