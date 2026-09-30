# PA24 handoff133 performance evidence

Entry `18721ea7` is frozen as `before`; initial implementation `1d73b01e` as
`after`; final implementation `e5b5dada` as `final`. Binaries, inputs, every
observation and hashes are retained in
`/home/vishvananda/work/private/v4codex/artifacts/pa24-133/`.
[Validation133](validation133.json) binds these artifacts and the tested sources.

Every experiment uses fixed binaries/inputs, `-O0 --stats`, one permitted CPU,
four A/A calibration samples and six ABBA blocks. Compiler and executable
measurements are separate. Both compilers' programs pass before timing; runtime
benchmarks check two argument counts. Loop counts depend on argc and results
are checked, preventing timing dead or constant workloads. Raw observations,
including noisy/outlying samples and the superseded frame policy, are preserved.

## Final compiler latency and peak RSS

Each scalar/floating/wide input has 4096 functions and 64 operations per function.
The added ABI input has 4096 callers, each with 64 mixed GPR/XMM calls, and one
shared conversion callee. Its checked entry computes the same 2080 sum.

| Input | A median (s) | B median (s) | Paired B/A [range] | Peak KiB A / B | A/A range (s) |
|---|---:|---:|---|---|---|
| scalar | 0.4953 | 0.4953 | 1.0027 [0.9164, 1.0107] | 88084 / 88008 | 0.4947–0.5035 |
| floating | 0.6661 | 0.6707 | 1.0087 [0.9932, 1.1632] | 88928 / 88988 | 0.6594–0.6702 |
| wide | 0.7335 | 0.7348 | 1.0106 [0.8715, 1.1208] | 88352 / 88304 | 0.7326–0.7629 |
| abi | 1.1025 | 1.1131 | 1.0104 [0.8748, 1.1646] | 135256 / 130116 | 1.0741–1.1520 |

Paired compiler medians increase by 0.3–1.1%; peak memory is effectively unchanged
or lower. The floating and ABI paired ranges include samples above 15%; their
medians and A/A spreads are disclosed, not suppressed. This is not a compiler
speedup claim. Absolute timings shift between the initial and final experiments;
only comparisons within frozen paired blocks support a change claim.

## Executable runtime and text

| Input | A median (s) | B median (s) | Paired B/A [range] | Text A / B |
|---|---:|---:|---|---|
| call-runtime | 0.2836 | 0.2738 | 0.9657 [0.9634, 0.9669] | 213 / 207 |
| mixed-abi-runtime | 0.1936 | 0.2094 | 1.0806 [1.0754, 1.0905] | 248 / 279 |

The indirect-call workload is 3.4% faster across all six blocks and loses six
text bytes. The useful fact is that setup does not overwrite the target carrier,
and the sole next comparison consumes the intact ABI result. Explicit argument
register/address read masks establish legality; fixed scratch or overwritten
carriers retain a safe copy/home. This saves executable work with bounded compiler
cost and no code growth. The workload checks forty million calls at the timed
input and sixty million at the second input.

The mixed-ABI workload is 8.1% slower and grows by 31 bytes. This is the cost of
the **required canonical shape**, not an optimization benefit: the canonical
mixed conversion callee has explicit floating parameter homes, dedicated integer
input carriers and preserved result registers. The entry callee's all-register
shape fails three mandatory fixtures. Both versions compute identical checked
results. Discarding those homes would reopen the required MIR failures; no
comparison rule or fixture was changed to avoid the cost. The workload performs
forty million mixed calls at the timed input and sixty million at the second.

All eight inherited final runtime images are **byte-identical to entry**:

| Input | Text bytes |
|---|---:|
| runtime | 161 |
| memory-runtime | 149 |
| floating-runtime | 1522 |
| pressure-runtime | 557 |
| wide-runtime | 308 |
| wide-numeric-runtime | 2211 |
| exception-runtime | 726 |
| tls-runtime | 247 |

`runtime-final-identity.json` binds final hashes and the two successful input
checks for every image. Their entry-side runtime observations remain applicable
because the final executable bytes are identical; no speedup is inferred from
noise. Runtime peak RSS is 256 KiB in every measured executable sample.

## Initial measurements and correction

The initial policy reserved scalar parameter capacity in every function. It
added seven bytes to unrelated scalar, memory and exception helpers, and changed
call-runtime from 213 to 221 bytes. This was an avoidable regression. Final code
reserves that capacity only for returning void boundaries, including the lowered
object-output ABI whose raw frame shape is mandatory. Scalar-return and
nonreturning functions use their actual frame requirements. Final inherited
images again match entry, and call-runtime becomes 207 bytes.

| Initial compiler | A median (s) | B median (s) | Paired B/A [range] | Peak KiB A / B | A/A range (s) |
|---|---:|---:|---|---|---|
| scalar | 0.9203 | 0.9217 | 1.0233 [0.9533, 1.0550] | 88084 / 88040 | 0.8870–1.0230 |
| floating | 0.7047 | 0.6909 | 0.9931 [0.9378, 1.0271] | 88932 / 88924 | 0.6913–1.4001 |
| wide | 0.7693 | 0.7662 | 0.9904 [0.9417, 1.0234] | 88348 / 88344 | 0.7554–0.9153 |

| Initial runtime | A median (s) | B median (s) | Paired B/A [range] | Text A / B |
|---|---:|---:|---|---|
| runtime | 0.7260 | 0.7044 | 0.9411 [0.8757, 0.9999] | 161 / 168 |
| memory-runtime | 0.5023 | 0.5786 | 1.0969 [1.0392, 1.1402] | 149 / 156 |
| floating-runtime | 0.9155 | 0.9075 | 1.0409 [0.9472, 1.1719] | 1522 / 1522 |
| pressure-runtime | 1.5308 | 1.6456 | 1.0856 [0.7632, 1.4365] | 557 / 557 |
| wide-runtime | 0.3654 | 0.3651 | 1.0042 [0.9839, 1.0147] | 308 / 308 |
| wide-numeric-runtime | 1.1254 | 1.1270 | 1.0048 [0.9912, 1.0163] | 2211 / 2211 |
| exception-runtime | 0.1704 | 0.1737 | 1.0041 [0.9082, 1.0516] | 726 / 733 |
| tls-runtime | 0.1668 | 0.1668 | 0.9977 [0.9840, 1.7294] | 247 / 247 |
| call-runtime | 0.2841 | 0.3052 | 1.0718 [1.0275, 1.0811] | 213 / 221 |
| mixed-abi-runtime | 0.1941 | 0.2083 | 1.0880 [1.0598, 1.1073] | 248 / 279 |

## Work, ownership and stage acceptance

- External LowIR is validated once. Unit-owned compact IDs/indexes feed
  per-function value facts, ABI placement and flat MIR. Encoding consumes that
  MIR directly; dumps are views. No text phase transport, external assembler,
  semantic re-resolution or name-based recovery was added.
- Parameter/mixed-domain classification shares the existing linear initialization
  walk. Boolean aliases use the known 0/1 producer fact and preserve the complete
  use interval; one-bit and wide representations keep explicit conversions.
  Adjacent elided scalar conversion debug locations attach to the producing
  normalization. All state releases with the function.
- Each call stages O(arguments) typed fragments. At most fourteen GPR/XMM
  assignments use bounded parallel scheduling (at most 14 squared dependency
  probes per round, at most twenty-eight transfer/cycle rounds). Stack transfers are
  linear and remain early when setup would destroy a value or address dependency.
  The fixed pool, cycle scratch and original conservative spill fallback remain.
- Stable ABI homes are distinct from private scratch-carry windows. Each scalar
  stack parameter and stack-call result gets at most one home at its definition.
  Conversion classification adds at most eight scalar XMM parameter stores and
  five preserved GPRs per function; pressure gets ordinary bounded frame homes.
- Frame finalization makes one linear walk for the void-return policy. Native
  prologues and epilogues consume the same stack and preserve facts as the dump.
  Overall work remains O(N + E log E) with inherited fixed-size reload windows;
  code and storage remain linear in consumed/produced IR. No body cloning,
  inlining, fixed-point rescanning, global invalidation or new optimization level.
- The inherited 15% compiler latency/RSS targets remain **diagnostic**, not extra
  exit gates. Final paired medians meet them. Zero **optional** text growth is
  met by the useful forwarding change; mandatory canonical ABI costs are reported
  separately. Required behavior, exact/canonical comparisons, MIR envelopes and
  finite work/growth limits remain mandatory. PA25 integration, hosted object/EH
  metadata and PA34 self-hosting remain later-stage owners.
- `trace133.cpp` follows `Mixer<7>::value` and the demanded `combine` member from
  streaming tokens and canonical semantic facts to typed LowIR, MIR and ELF.
  Telemetry records one class completion, one demanded template region and two
  emitted functions; the unused dependent member is absent. Both runtime inputs,
  MIR/debug identity and ELF/disassembly checks pass. The trace emits 214 text
  bytes. The explicit source/LowIR tool adapter is the PA24 boundary, not a new
  production text transport. Independent whole-stage review remains pending.

## Retained observation manifests

| Manifest | SHA-256 |
|---|---|
| `bench-scalar/observations.json` | `af649a5146826267b6a2150cdfa45e282375edbee358d1131850ee0e5ba71fe8` |
| `bench-floating/observations.json` | `c1616689d65126d9baad0ee4f0434f9eb16a6c519693e5745740e2c4723fd96e` |
| `bench-wide/observations.json` | `50cebbb0f20d57583afb320d833fd207dbcf2f5823a386e77e862897844deb06` |
| `final-scalar/observations.json` | `0f98feb6a1093e25795d052ba8b59d663198c778a459f61748ce4aeba9bb6041` |
| `final-floating/observations.json` | `a4f801467624959c5a5615fe7802a452cae869839b461d03d025eeceb8258f75` |
| `final-wide/observations.json` | `fa5c24f025e6eed7b20e2eb30f207a95ce5bd0247377c85841edd14e799651ea` |
| `final-abi/observations.json` | `7efcc552b75611950bd7fd5f51ab2869c2527d8102de0f578a3a7165c90e394b` |
| `runtime-final-identity.json` | `a0c1ed0eb359ac90abf55c1173a254267d3f1c7aabd34b035078ada2782e2b1b` |
