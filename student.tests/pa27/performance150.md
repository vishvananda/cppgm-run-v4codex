# PA27 final audit150 performance

Code tip: `2fba819daa6e8a79a38dc0d35a3f8f942ff01bb7`.
Final compiler SHA-256: `74d142d3f7cac9931ece646a63ad3d71689fe1174fd0d5fb596e1e45d151b06d`.

## Frozen comparisons and protocol

[Common](evidence150/common-performance.json),
[storage](evidence150/storage-performance.json) and
[signature/hosted](evidence150/hosted-performance.json) records preserve binaries,
input hashes, flags, host tool version, every observation, phase/work counters,
RSS, statuses and image sizes. All final measurements use the frozen binary
above, `-O0 -c --stats`, CPU 0 and the same Linux x86-64 host. Compiler and
executable wall time are measured separately; host linking is outside timing.
No build/test suite overlapped these measurements. Each supported comparison
has four A/A calibration observations followed by six ABBA blocks per mode.
Ratios are B/A means within each block; the table gives their median and range.
Standalone seconds are medians of the twelve timed A or B observations, which
need not have the same ratio as paired medians. RSS is the maximum measured KiB.

| Pair | Frozen A | Purpose |
|---|---|---|
| common memory/FP/EH/pruning | Stage base `f833cf1f`, SHA `55d08c175a29d771e96d63e00c9f166526db0d743e3ca3cdbb9ef48b610d547c` | Whole-stage cost on equivalent correct executable behavior; A lacks required PA27 object semantics. |
| construction/constant | Before audit148, SHA `a2abb98ced2fc821db02b80d0661bf59797d0d2851847ef4497366e30bdf524e` | Equivalent already-supported storage behavior, including final semantic projection ownership. |
| signature | Audit148, SHA `2dd35668617b07ddd6cb5f24c699d12bbaf02bdb92a06e2c186bbcee26e29fa5` | Handoff149's access/signature work on already-supported input. |
| hosted | B only; A's compile failure retained | Newly supported behavior, not an optimization comparison. |

The fixed common sources match the inherited PA26 hashes. They demand 2400
template bodies and execute checked runtime loops: 3,000,000 memory/FP iterations
or 200,000 throws and destructions. Pruning adds 1200 unreferenced locals.
Construction exercises 600 specializations over 12,000 runtime iterations;
constant construction evaluates 8192 objects and performs 5,000,000 runtime table
reads. Signature demands 1000 redeclared templates; hosted repeats 200,000 stdin
parses. Runtime inputs, observable results and independent expected sums prevent
dead/constant-folded timing. All timed runs check success. Hosted output is also
checked with a host-built control. PA34 self-hosting is outside PA27's scope.

## Four performance dimensions

| Input | Compiler seconds A/B | Compiler peak KiB A/B | Runtime seconds A/B | Text bytes A/B | Paired compiler ratio (range) | Paired runtime ratio (range) |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.3215 / 0.3889 | 29056 / 29376 | 0.0910 / 0.0927 | 151633 / 151633 | 1.065 (0.982–1.408) | 0.977 (0.904–1.205) |
| floating | 0.4490 / 0.4251 | 29096 / 28940 | 0.0945 / 0.0929 | 151474 / 151474 | 0.942 (0.853–1.356) | 1.010 (0.926–1.120) |
| exceptions | 0.4925 / 0.4939 | 28996 / 29192 | 0.4590 / 0.4941 | 151781 / 151781 | 1.002 (0.929–1.156) | 1.014 (0.824–2.023) |
| pruning | 0.4340 / 0.5009 | 35204 / 34792 | 0.0839 / 0.0879 | 182449 / 151633 | 1.079 (0.704–1.522) | 0.978 (0.902–0.994) |
| construction | 0.4864 / 0.5048 | 36160 / 36512 | 0.5104 / 0.5227 | 118696 / 118696 | 0.977 (0.857–1.126) | 0.995 (0.955–1.109) |
| constant | 1.2499 / 1.2255 | 77664 / 77320 | 0.0950 / 0.0943 | 439 / 439 | 0.811 (0.730–1.016) | 0.989 (0.890–1.042) |
| signature | 0.2175 / 0.2990 | 22360 / 22772 | 0.0980 / 0.1021 | 63092 / 63092 | 1.060 (0.924–1.398) | 0.962 (0.855–1.096) |

A/A calibration ranges, seconds:

| Input | Compiler min–max | Runtime min–max |
|---|---:|---:|
| memory | 0.2497–0.2626 | 0.0865–0.1119 |
| floating | 0.3269–0.4807 | 0.0787–0.0869 |
| exceptions | 0.3433–0.5402 | 0.4508–0.5501 |
| pruning | 0.3426–0.5273 | 0.0798–0.0923 |
| construction | 0.4599–0.7394 | 0.4727–0.5035 |
| constant | 1.1335–1.5555 | 0.0881–0.1011 |
| signature | 0.3225–0.4237 | 0.0795–0.0938 |

Hosted compilation costs **2.2120 s** median (1.9070–2.5652), **67,260 KiB**
peak. Runtime costs **0.3267 s** (0.2796–0.6920), **3,548 KiB** peak, with
**31,873 executable text bytes** and a **198,864-byte object**. Twelve compiler
and twelve runtime observations establish costs; the unsupported A cannot
establish profit. These results do not imply broad hosted-library support.

Timing is noisy. The pre-context run's compiler paired medians were 1.020 memory,
1.018 FP, 1.050 EH, 0.994 pruning, 0.999 construction, 0.991 constant and 1.007
signature. Its hosted runtime was 0.1702 s on the **identical executable**.
The final constant ratio of 0.811 therefore does not establish a repeatable
19% speedup, nor does the unpaired signature median establish a precise
slowdown. The EH range includes a 2.023 ratio and is retained. Final pruning
runtime ratios are below one, but the pre-context median was 1.004; only the
required static removal is claimed. No observation is removed as an outlier.

## Equivalent outputs and actual native costs

[Image comparison](evidence150/image-comparison.json), reproduced by
[compare150.py](compare150.py), verifies all final objects/executables are
byte-identical to their pre-context counterparts. The conditional-context repair
does not affect these valid inputs. Comparing A/B:

* Construction and signature objects and executables are byte-identical.
* Constant objects and executable `.text` are byte-identical. Whole executables
  differ in the linker's `STT_FILE` spelling `constantA.o` versus `constantB.o`;
  the complete symbol-table comparison records that difference.
* Memory and FP have identical normalized instruction sequences, counts and text
  sizes. Normalization removes instruction/target addresses and RIP displacements
  but retains symbolic targets, opcodes and other operands.
* EH changes one imported RTTI address from LEA to the required GOT MOV. Instruction
  count and text size are unchanged. It enforces object identity and PIE-safe
  imports; the measured runtime range is disclosed rather than excused by an IR
  count or called an optimization gain.
* Pruning removes exactly 1200 unused functions and **30,816 executable text bytes
  (16.9%)**; every shared function has identical normalized instructions. Addressed
  locals and required base/runtime entries remain in the separate correctness
  controls.

The common memory object grows **466,960 → 933,720 bytes** for required COMDAT
sections, groups, symbols and relocations; final executable text stays 151,633.
Avoidable empty relocation sections were already removed and remain absent.
Required host interoperability explains this object metadata cost; no blanket
zero-growth target can replace the assignment's object contract.

The new source-to-ELF trace exposes actual frames and traffic: 109 selected
instructions, 663 code bytes, a 64-byte demanded-function frame, return-value
stores/reloads across destruction, and a preserved register across the TLS call.
These O0 costs are recorded in [the architecture audit](../../pa27/audit.md).
Instruction comparisons above include loop/call/spill instructions, so unchanged
code claims are not inferred from IR size alone.

## Work, validity and stage-scoped acceptance

Construction has unchanged 6196 tokens, 9334 parsed nodes, 600 body transitions,
600 class completions, 3000 constructor actions and 1200 storage paths. Final
semantic projection facts number 5400, while the 8192-constant input needs only
nine projections and two constructor paths. They replace repeated path discovery
with TU-owned completed-layout facts; activation groups share selected receiver
addresses and freeze aggregates once. Compiler peak changes are +352 KiB and
−344 KiB respectively, with no executable growth.

Signature retains 10,189 tokens, 15,299 parsed nodes, 3010 specializations, 1000
body transitions and 1001 class completions. It adds 2000 required access checks,
reduces uncached type substitutions **8018 → 5018**, increases substitution hits
**1 → 2001**, and lookup work **16,071 → 17,071**. Peak RSS rises **412 KiB**.
These source-access facts enforce semantics; `(frame, recipe)` memoization and
subtree pruning prevent unrelated checks. The final header-probe repair adds one
context check at the shared owner and no collection or additional pipeline pass.

Whole-pipeline bounds remain the ones reconstructed in the audit: demand-specific
facts and queues, O(symbols + operands) reachability, near-linear per-function
allocation, bounded local selection/carry windows, O(S log S) section placement
and linear byte/fixup writing. Canonical caches have complete keys and TU/function/
activation release points; incomplete-class changes invalidate only dependent
facts. Existing constant step/depth, frame/alignment and ELF section limits remain.
No new speculative growth or fixed-point optimization is introduced. Unknown
legality/budget cases keep conservative operations; ABI and unwind facts survive.

Under spec §9, inherited blanket **15% latency/RSS** and **zero-growth** targets
remain diagnostics. All historical measurements and misses are preserved.
Required COMDAT/GOT/access costs do not create new gates, and known avoidable
metadata/projection work has been removed. There is no new optional transform
whose profitability depends on these noisy timings. O0 is the current policy;
accepted O2 controls do not claim PA32/33 completion. PA34 self-hosting remains
later scope. No mandated limit, timeout, correctness obligation or test coverage
is reclassified.

## Preservation and reproduction

The [measurement inventory](evidence150/measurement-inventory.json) hashes
**3736 observations**: **2904 historical**, **416 pre-context**, **416 final**.
Pre-context [common](evidence150/pre-context-common-performance.json),
[storage](evidence150/pre-context-storage-performance.json) and
[signature/hosted](evidence150/pre-context-hosted-performance.json) records remain
complete. The rerun was required by the compiler repair, not by disappointing
numbers. Previous [148](performance148.md) and [149](performance149.md) evidence
is unchanged.

Use the existing scripts with the frozen A appropriate to each comparison and
the final B:

```sh
PERF_CPU=0 python3 student.tests/pa27/performance147_common.py OUT BASE FINAL
PERF_CPU=0 python3 student.tests/pa27/performance148.py OUT STORAGE_A FINAL
PERF_CPU=0 python3 student.tests/pa27/performance149.py OUT SIGNATURE_A FINAL
python3 student.tests/pa27/compare150.py "$RALPH_ARTIFACT_DIR/pa27-150"
python3 student.tests/pa27/validation150.py OUT
```

Frozen binaries, generated fixed inputs, full outputs and views remain under
`$RALPH_ARTIFACT_DIR/pa27-150/`: `common-final`, `storage-final`, `hosted-final`
use `validated-cppgm`; the unsuffixed directories use preserved `final-cppgm`
from before the repair. [Validation](evidence150/validation.json) pins the final
code/binary and passing required gates. The trace additionally proves emitted
object bytes are unchanged with telemetry disabled.
