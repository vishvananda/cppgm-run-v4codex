# PA24 final audit134 performance evidence

Frozen A is entry `63dc6c11`; B is repaired production `a8168480`.
All binaries, flags, inputs, raw wall/RSS observations, phase/work telemetry and
paired results remain under `/home/vishvananda/work/private/v4codex/artifacts/pa24-134/`.
[Validation134](validation134.json) binds the evidence to current source/binaries.
No timing observations, including historical misses and outliers, were removed.

Native measurements use `benchmark.py`, `-O0 --stats`, one permitted CPU, four
A/A calibration observations and six wall-time ABBA blocks. Compilation and
execution are timed separately. Every compiled workload executes successfully;
runtime programs check computed results at two argument counts before timing.
Loop bounds depend on argc, so these measure live computation. Binary/input
hashes and all ten executable pairs were reverified after measurement.

## Compiler latency and peak memory

Scalar, floating and wide inputs each have 4096 helpers with 64 operations.
ABI has 4096 mixed-call callers; EH adds handler/dynamic/aligned-frame work;
TLS has 4096 storage objects plus load/store helpers. Both A/B implementations
are correct on these fixed benchmark inputs.

| Input | A median s | B median s | Paired B/A median [range] | Peak KiB A / B | A/A range s |
|---|---:|---:|---|---:|---|
| scalar | 0.5406 | 0.5614 | 0.9826 [0.9200, 1.0709] | 88064 / 87852 | 0.5050–0.8788 |
| floating | 0.6823 | 0.6851 | 1.0011 [0.9813, 1.3146] | 88924 / 88936 | 0.7217–0.8496 |
| wide | 0.7875 | 0.8107 | 1.0096 [0.6937, 1.1787] | 88352 / 88344 | 0.7497–0.7758 |
| abi | 1.1225 | 1.1427 | 1.0260 [0.7531, 1.0978] | 130148 / 130100 | 1.0943–1.1821 |
| eh | 0.5436 | 0.5451 | 1.0026 [0.9710, 1.0355] | 78484 / 78484 | 0.5426–0.6219 |
| tls | 0.5292 | 0.5251 | 1.0012 [0.8550, 1.0246] | 83808 / 83820 | 0.5202–0.7019 |
| frontend templates | 0.7241 | 0.7149 | 0.9812 [0.8920, 1.0111] | 106908 / 107052 | 0.7082–0.7346 |

Native paired medians span 0.983–1.026; peak RSS is effectively unchanged.
The four-byte-per-block exit fact and constant-time publication/consumption add
no new analysis or iteration. These observations do not establish a compiler
speedup or repeatable regression: scalar unpaired and paired summaries even move
in opposite directions, and wide/floating samples include substantial outliers.
All samples and spread are retained; no 15% sample cutoff was used.

`frontend134.py` rechecks the fixed inherited 9600-specialization source from
PA23 performance126, using frozen **identical** frontend binaries. It uses the
same A/A/six-ABBA protocol for `-O0 --emit-lowir`, checks stats/audit output, then
lowers both outputs with our own native backend and executes them. Both LowIR
and ELF hashes match; text is **806082 bytes**. Timing variation between identical
frontends is noise, not a speedup. The short generated program is a correctness
check, not a runtime performance claim. Source/native combined-driver and
self-hosting benchmarks remain later-stage owners, not missing PA24 gates.

## Generated-program runtime and text

All ten frozen A/B images are **byte-identical**. Text bytes below exclude the
ELF header, data and alignment padding. Every runtime sample has 256 KiB peak RSS.

| Workload | A median s | B median s | Paired B/A median [range] | Text bytes A = B | A/A range s |
|---|---:|---:|---|---:|---|
| runtime | 0.6757 | 0.7131 | 1.0234 [0.9778, 1.0961] | 161 | 0.6036–0.6591 |
| memory-runtime | 0.6086 | 0.5767 | 0.9468 [0.7830, 1.2572] | 149 | 0.5369–0.7143 |
| floating-runtime | 0.8588 | 0.8773 | 1.0165 [0.9439, 1.1292] | 1522 | 0.8111–1.0591 |
| pressure-runtime | 2.6612 | 3.0624 | 1.1228 [0.6585, 1.3072] | 557 | 2.2676–2.5103 |
| wide-runtime | 0.6739 | 0.6580 | 0.9023 [0.7317, 1.2311] | 308 | 0.5391–0.8304 |
| wide-numeric-runtime | 2.2766 | 2.0392 | 0.9571 [0.7957, 1.0737] | 2211 | 1.6892–1.9595 |
| exception-runtime | 0.3694 | 0.3685 | 1.0177 [0.8036, 1.1394] | 726 | 0.3887–0.4020 |
| tls-runtime | 0.2779 | 0.2775 | 0.9842 [0.9364, 1.0667] | 247 | 0.2729–0.5289 |
| call-runtime | 0.4967 | 0.5028 | 1.0003 [0.9001, 1.0868] | 207 | 0.4575–0.4887 |
| mixed-abi-runtime | 0.3666 | 0.3370 | 0.9636 [0.9115, 1.0369] | 279 | 0.3627–0.3783 |

In particular, the pressure median ratio of 1.123 and broad wide/memory ranges
are disclosed. Identical executable bytes establish that they are host timing
variation, not different spill or loop code. No new runtime improvement is
claimed. The useful historical forward-edge, floating-register and indirect-call
selections retain their measured final text sizes and checked behavior.

The affected phi reducer is deliberately excluded from A/B speed claims because
entry computes the wrong answer. Its corrected image grows **265→269 bytes**,
with one additional typed frame load, unchanged 64-byte total frames and six
parameter-flow visits. This is bounded required semantic cost. The repair adds
no optional transform and does not alter the ten equivalent runtime workloads.

## Stage-scoped acceptance and inherited evidence

- All 37 historical observation/identity manifests in performance127–133 were
  rehashed successfully (`inherited-manifests.json`). Those measurements, including
  rejected initial designs and noisy misses, remain intact.
- Forward-edge retention previously measured paired runtime 0.592 with
  176→161 bytes; XMM reuse measured 0.458 with 2169→1522 bytes. Each improved
  across all six frozen ABBA blocks. This audit reviewed their interval/effect
  legality and preserves their output. These are historical claims, not inferred
  from the present noisy measurements or from IR node counts.
- Handoff133 measured the indirect-call setup benefit (0.966 paired runtime,
  213→207 bytes). Its required canonical mixed-ABI policy costs 8.1% runtime
  and 248→279 bytes. The all-register entry policy fails mandatory shape cases;
  removing those homes would violate the contract. The avoidable broad leaf-frame
  reservation was already narrowed to returning-void boundaries. Both the original
  measurements and corrected images remain preserved.
- The inherited 15% compiler latency/RSS and zero optional text-growth targets
  are **diagnostic**, not handout or spec exit gates. Current compiler medians
  meet those diagnostic targets, but this is not the basis for waiving outliers
  or correctness. The required MIR comparisons/envelopes, focused relationships,
  finite work/growth bounds and all behavior remain mandatory and pass.
- No extra O1–O3 pass, global optimization, body cloning or iterative rescanning
  was introduced. O0 composition remains O(N + E log E), fixed register pools,
  at most fourteen ABI carriers, three 64-instruction carry probes, six-bit
  monotonic parameter flow and linear code/storage. The new fact is one word per
  block. Host object/EH integration and self-hosting are later-stage constraints.

## Retained raw manifests

Each manifest retains every sample, paired value and min/max spread; the tables
are rounded summaries, not filtered observations.

| Manifest | SHA-256 |
|---|---|
| `bench-scalar/observations.json` | `2b345d17244cd1659e73667afb297a2a5164b88fbee2c4ed06614c825367f98e` |
| `bench-floating/observations.json` | `9ed4727d2adc6bc6d8abbd3ff70e8ce07e01d79e99cd76ec9751b6cbc056c8f4` |
| `bench-wide/observations.json` | `72af26cf2152e03e37506a75f1f31bd615a6dfa2c779e78a78c274e8603e70dc` |
| `bench-abi/observations.json` | `8eebf152c25124c8a75a16c9e1c32f1e4f8980de8d740980b87aaa28359fd758` |
| `bench-eh/observations.json` | `043156a440440217e414bfb8ff53d169f1d9d2bd3bcce7b15c797b330f2ed337` |
| `bench-tls/observations.json` | `27d07416f8d638e69aacf19dca5741022f099424592e6f5f6394e4292efd014f` |
| `bench-frontend/observations.json` | `e0832766b949e2ad32d9ebd3d406eff879759607cae8138d68a853448960e448` |
| `inherited-manifests.json` | `5c6d8985994b33449890f0ed75d7a1a159b2c2302dd74f118ec00e4fcdaad114` |
