# PA9 compact plan and audit ledger

Target: **PA9 full-stage**; phase: **complete**.
Stage base: `2419a3dd943352a4e959d26bdf519d7441afe19d`.
Independently reviewed through: `0e2fcf0ff`.
Original implementation, checkpoint repairs, serializer, telemetry and evidence
handoffs have all been reviewed from their source and commit changes.

## Final Spec Alignment

| Ownership group | Final design / evidence |
| --- | --- |
| Fact adapter (§1) | One pass per explicit record; interned case binders construct canonical graph facts immediately. Streaming CLI and optional whole-file/serializer APIs remain separate. |
| Canonical graph (§§2,5,6,8) | Flat pools, immutable IDs, immediate typed-link checks and no forward edges. Shared canonical values, cv/function types, tags and function-template shapes. No textual equality key or duplicate semantic graph. |
| Encoder (§§3–7) | Append-only output, sparse symbol-local substitutions, demand-proportional traversal. Local contexts share substitution state; external names isolate substitutions but share output and nesting budget. |
| Lifetime / work (§§8–9) | Per-case graph release, bounded parser buffer reuse, geometrically grown tables, iterative modifier chains, all recursive routes guarded at 1024. No whole-graph retry/invalidation or optimizer fixed point. |
| Self-containment / later boundary (§10) | Required names are constructed here. No host/reference implementation is called. PA10 will pass typed facts directly; LowIR/native/ELF and executable runtime/text are not PA9 surfaces. |

The [independent audit](audit.md) records concrete declaration, template/local
context, dependent-expression, qualifier and thunk traces, validity ownership,
complexity, growth limits, and all eight findings. It applies the available PA9
surfaces of the spec; it does not claim to have audited unimplemented source
instantiation, machine optimization or object emission.

## Changes and validation

- Moved literal/cv canonicalization to graph publication; qualified function
  identity and qualifier position now follow the ABI grammar.
- Added immediate graph-link validation, universal nesting guards, correct
  large discriminators, and rejection of completed template prefixes.
- Shared function shape/tag normalization and interned spelling views; isolated
  external encoders append directly to the destination without nested copies.
- Completed function/context/template-parameter/thunk inspection roundtrips.
- Added direct API invariants, eight final exact/roundtrip cases and eleven
  controlled rejections. Original 117 explicit status/output cases, successful
  roundtrips, 11 valid/15 invalid probes, batch order, 100k unrelated graph and
  20k iterative modifiers remain. The full suite passes ASan/UBSan with leaks
  enabled; no signals/timeouts or sanitizer reports remain in these checks.

Final source `0e2fcf0ff` passed:

- `make test-pa9`: **111/111**.
- `make test-report-through-pa9`: **904/904, 9/9 stages**.
- `perl scripts/cppgm_file_audit.pl --stage pa9 --paths dev/src`: **114 files,
  zero warnings, pass**.
- Both personal drivers and the complete ASan/UBSan runs: **pass**.
- Both final performance verifications: **42/42 gates each**; both retained
  preliminary reports independently verify in historical mode.

The incoming 928/928 status is corrected to the authoritative root harness
count: **904/904 through PA9**, including **111/111 default PA9**; six root
fixtures additionally pass in the explicit **117-case** personal run.

## Performance protocol and budgets

[Frozen protocol](../student.tests/pa9/final-audit-protocol.md): final B compared
separately with checkpoint A and first-correct A, identical flags/inputs,
AAAA calibration plus two ABBA blocks per workload. Eight workloads cover
unique template values, dependent expressions, modifier chains and local/
template batches at 1x/4x sizes. Compiler wall/RSS/text are measured;
generated runtime/text are explicitly N/A. All raw observations remain,
including superseded successful runs and the historical text-budget failure.

Unchanged budgets: compiler wall ≤1.25x A; peak RSS ≤1.20x A +16 MiB;
fourfold wall ≤5.5x, measured work ≤4.5x, RSS ≤5x; compiler text growth ≤100 KiB
from the independently rebuilt first-correct binary. See the
[performance report](performance.md) for paired results, spreads and final
checks. Final whole-stage modifier latency improves **37.8–38.1%**; disclosed
regressions elsewhere reach **9.2%** (budget 25%). Fourfold wall is
**3.91–4.63x**, single-case RSS **3.73–3.79x**, and batch RSS stays nearly flat.
Audit-delta positive changes are **0.8–5.9%**; a noisy negative expression
result is explicitly not a speedup claim. Compiler text is **274663 bytes**:
**+8999 bytes** from checkpoint and **+102245 bytes** from first-correct,
inside the frozen **102400-byte** total-growth budget. All **192 final**
observations and **384 earlier** observations remain separate. Correctness
repairs do not claim executable-runtime improvement.

## Commit and handoff ledger

| Commit | Reviewed result |
| --- | --- |
| `a65da9d25` | Stage-base plan and original review markers. |
| `2b19ba07a` | First correct typed encoder and all required behavior groups. |
| `06c5d9bf4` | Sparse substitutions, linear modifiers, canonical tagged templates, indices, shape, serializer and telemetry. |
| `b43bc1ff7` | Serializer code-growth repair and deep roundtrips. |
| `8195487d1` | Checkpoint audit/plan, all-stage validation and controlled performance evidence; independently reviewed here. |
| `3dcf74a98` | Frozen independent audit protocol and unchanged budgets. |
| `f2353211e` | Whole-owner canonicalization, validity, recursion, emission and serializer repairs with explicit tests. |
| `0e2fcf0ff` | Final malformed-prefix guard and retained preliminary A/B evidence. |
| Final evidence commit | Consolidated Spec Alignment, all findings, frozen final measurements, reproduction instructions and fresh exit verification. |

No earlier implementation, contract fixture, reference, harness or attribution
was changed. No PA9 behavior group or previously unaudited handoff is deferred.
PA10's future task is semantic-to-typed-ABI lowering, using the shared encoder
without a fact-text roundtrip. Advancing is outside this audit.
