# PA24 handoff131 performance evidence

Frozen entry: `8c10ee78` (`before`). Final implementation: `c7587f72`
(`final`). `after` is the first implementation binary; the final change adds
the reserved exception objects to MIR output. All binaries and all eight
observation manifests remain under `/home/vishvananda/work/private/v4codex/artifacts/pa24-131`.

Measurements use fixed inputs and `-O0 --stats`, one permitted CPU, four A/A
calibration samples, then six wall-time ABBA blocks. Compilation and execution
are timed separately; generated results are checked at two argument counts.
The final compiler measurements use the final frozen binary. Final executable
identity is checked against the images used in the longer runtime experiment.

## Compiler latency and peak RSS

| Workload | A median (s) | B median (s) | Paired B/A median [range] | Peak KiB A / B |
|---|---:|---:|---|---:|
| scalar | 0.9230 | 0.9000 | 0.9710 [0.9529, 1.1048] | 88044 / 88040 |
| floating | 1.9084 | 2.1290 | 1.0757 [0.8183, 1.4695] | 88928 / 88928 |
| wide | 1.3789 | 1.3446 | 0.9876 [0.8394, 1.1962] | 88312 / 88308 |
| runtime | 0.9568 | 0.9529 | 1.0171 [0.9258, 1.1226] | 78468 / 78472 |

Scalar, floating and wide inputs each contain 4096 functions with 64 typed
arithmetic operations. The new runtime compiler input adds handler regions,
dynamic stack allocation and aligned storage to 4096 functions. Its A and B are
**the same final binary**: these samples describe absolute costs and noise,
not an improvement over an entry implementation that rejected this surface.

There is substantial host noise. In the final floating experiment A/A spans
1.053–1.199 s; paired B/A spans 0.818–1.469. Final scalar and wide paired medians
are 0.971 and 0.988; floating is 1.076. These do not establish a compiler speedup
or a repeatable regression. RSS is effectively unchanged. The initial scalar
experiment was quieter (paired median 1.00065); all initial floating/wide and
runtime samples, including noisy observations, are retained rather than filtered.

## Executable runtime and text

| Program | A median (s) | B median (s) | Paired B/A | Native text bytes |
|---|---:|---:|---:|---:|
| runtime | 0.4091 | 0.4099 | 1.0011 | 161 |
| memory-runtime | 0.3105 | 0.3100 | 0.9985 | 149 |
| floating-runtime | 0.5377 | 0.5374 | 1.0006 | 1522 |
| pressure-runtime | 1.9433 | 1.6951 | 0.8777 | 557 |
| wide-runtime | 0.4555 | 0.4193 | 1.0251 | 308 |
| wide-numeric-runtime | 1.1171 | 1.1160 | 0.9993 | 2211 |
| exception-runtime | 0.1663 | 0.1661 | 0.9993 | 726 |

All six inherited A/B runtime images are byte-identical, so their ratio
variation is noise rather than a code-generation change. Runtime peak RSS is
256 KiB for every sample. `final-runtime-identity.json` binds each final image
to those measured bytes and rechecks both input counts.

The new exception workload executes eight million calls at the timed input;
half throw across a function boundary. Each call uses aligned frame storage
and dynamic allocation, and the caller checks the input-dependent accumulated
sum. Its A/B images are identical copies of the new implementation. Its median
is about 0.166 s and its 726-byte text is a necessary new semantic cost. No
optimization gain is claimed. The two-input correctness check also exercises
twelve million calls.

## Acceptance and bounded work

- No optional optimization was added. All inherited runtime text sizes stay
  unchanged; the diagnostic zero optional-growth target is met.
- New selection/frame work is linear in values, instructions and exceptional
  registrations. Existing six-bit parameter flow remains monotonic and bounded.
  Native selection and emission consume the same flat MIR and frame facts.
- Each handler installation uses one 80-byte dynamic record. Normal pop reclaims
  it immediately when no allocation intervenes. Allocations keep function
  lifetime, including after catch/pop; the selected frame floor prevents reuse
  before return. Frame and call alignment add at most alignment-minus-one
  padding each. Unwind transfers are constant-size native sequences.
- The image owns a pointer-sized top field in a 16-byte region and a 16-byte
  scalar payload. Only EH inputs reserve them; non-EH functions receive no
  handler state, and only functions combining EH and allocation track a floor.
- Existing 15% compiler latency/RSS targets are diagnostics, not new exit gates.
  Correctness, course MIR bounds, comparison rules, finite work and code-growth
  budgets remain binding. The noisy data creates no performance claim to waive.
- Integrated source-driver, host EH objects and self-hosting benchmarks remain
  owned by later stages. The native stage maintains the existing scalar, memory,
  floating, call and wide workloads and adds executable EH/frame coverage.

## Retained manifests

| Manifest | SHA-256 |
|---|---|
| `bench-scalar/observations.json` | `6d0b3d69df8092c8bdb6adb175ccb5f8e9f9c45f92d3eac77a328d43a1f772e5` |
| `bench-floating/observations.json` | `933f31ae1c128d31229a3175610ad563174f69b6a66b4b0098bb21d01997531d` |
| `bench-wide/observations.json` | `8b5586119209ef41df76bcdbf773dafd20a25cb79a490fdf38571df63268a955` |
| `bench-runtime/observations.json` | `d885fae8d04b732554ebe405066a21a3d2354a4b2666175148efed80cb89eb0f` |
| `final-scalar/observations.json` | `ce76d89fae604620d6ce842c27e6e28f01eb843ef6768e4bd7b762281bbd3931` |
| `final-floating/observations.json` | `a92e1abe140400963067529c46e2cce4a7a1728828bf0660bb1528228e997c1a` |
| `final-wide/observations.json` | `b35259320dbe04cc9b6d7a5dde3f1e86c6b1cf92d3e0ddf69321cff6b1d396db` |
| `final-runtime/observations.json` | `57e691ef69638de6ffeb465bcfd85a0da296aa3ff1adef9c68cab7807215af9a` |
