# PA9 plan and ledger

Target: **PA9 full-stage**; phase: **complete**.
Stage base commit: `affdafd23213da497d2946cae83df02d7a41ba7a`.
Last reviewed commit: `affdafd23213da497d2946cae83df02d7a41ba7a`.
These review markers remain unchanged for Ralph's independent review.
Entry: **2/117** reported; the default harness discovers **111** ABI fixtures.
Final: **111/111 default**, **117/117 explicit**, **904/904 through PA9**.
All six additional root fixtures were checked explicitly; coverage is unchanged.

## Design/spec alignment and completed groups

| Owner | Data flow and complexity | Validation |
| --- | --- | --- |
| Graph / fact reader | Case-local binders -> canonical compact facts; flat pools/maps, IDs and child slices. O(bytes + edges) expected, iterative compact modifiers. No owning record/tree copy. | Builtins, cv, arrays, duplicate IDs, 64-bit indices, 20k modifiers |
| Type/name encoder | Typed IDs -> append-only symbol and sparse name-local substitutions. Work tracks consumed facts/output; canonical tags and qualified/template prefixes. | 100/200 fixtures, equivalent/distinct values/types, tagged template probes |
| Function / special names | Explicit semantic terminals, qualifiers, local contexts, template prefixes and fixed/virtual thunk fields. Production supplies member/nonmember shape. | Operators, conversion ordering, lambdas, external entities, six root probes |
| Arguments / expressions | Canonical DAGs retain ordered operands, literals, traits and dependent owner facts. No mangled-text keys or phase transport. | 300–600 fixtures, expression equivalence/distinction, direct API |
| Serialization / observability | Optional fact view memoizes definitions and iterates modifier chains. CLI streams per case; phase timing is optional, counters observe existing work. | All successful fixture roundtrips, deep roundtrip, batch order, sanitizers |

[Completion audit](audit.md) records ownership, representative traces and the
stage boundary. No remaining PA9 behavior groups. PA10 source-to-LowIR naming
is a later milestone; it will call the same typed encoder directly.

## Performance and required checks

[Performance report](performance.md): frozen correct A versus final B, AAAA
calibration and two ABBA blocks per workload, all 96 final observations retained
alongside the initial 96 (including its failed text-growth gate). Final compiler
latency/RSS/scaling/text satisfy all **41 unchanged budgets**. Modifiers are
40.7–41.1% faster; other workload regressions of 1.9–6.4% are disclosed. Text
+91.1 KiB; fourfold wall 3.93–4.41x. Generated executable runtime/text: N/A in PA9.

- `make test-pa9`: **111/111 pass**.
- `make test-report-through-pa9`: **904/904 pass; nine stages pass**.
- Exact prior-through command: **793/793 through PA8 pass**.
- `perl scripts/cppgm_file_audit.pl --stage pa9 --paths dev/src`:
  **pass, 112 files**.
- Explicit personal suite: **117 fixture status/output checks**, successful
  fixture roundtrips, **11 valid +15 invalid** probes, batch order and direct
  API assertions pass. Final **ASan/UBSan with leak checks pass**.
- Performance verifier confirms frozen/current binaries, input/output hashes,
  raw observation ordering and all budget calculations.

## Handoff ledger

- `7442a3192`: initial plan and review markers before stage edits.
- `df7dbb00a`: complete first typed encoder; all required behavior fixtures pass.
- `f6552f8d1`: sparse substitutions, linear modifiers, canonical tagged templates,
  full-width indices, explicit operator shape, serializer, telemetry and probes.
- `ea87f4ba7`: serializer growth correction and deep roundtrips; final frozen
  implementation. First-run observations retained without altering budgets.
- Final evidence commit: compact plan, audit, performance report, raw final data,
  reproduction instructions and independent evidence verifier. Implementation
  checks rerun against this source state; committed clean status checked last.

Handoff reason: **PA9 is complete**, with no incomplete group or reduced test
coverage. Course fixtures, references, harnesses and earlier implementations
were not changed. Review markers are preserved for the next review.
