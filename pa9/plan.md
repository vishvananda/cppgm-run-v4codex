# PA9 implementation plan

Target: **PA9 full-stage**; phase: **implement**.
Stage base commit: `affdafd23213da497d2946cae83df02d7a41ba7a`.
Last reviewed commit: `affdafd23213da497d2946cae83df02d7a41ba7a`.
Entry: reported 2/117; primary log contains 2/111 ABI fixtures. Preserve all
111 ABI and six additional root fixtures; establish fresh root counts.
Previous goal turn: no implementation progress visible; next action is the
missing encoder, not another status check.

## Design and remaining groups

| Owner | Data flow / work bound | Validation |
| --- | --- | --- |
| Typed graph, fact adapter | File-local binders -> canonical compact IDs in flat pools; no rendered semantic keys. Parse each fact once; O(bytes + edges) expected work. Adapter is explicit input only. | Builtins, cv, arrays, paths, duplicate IDs, negative indices, direct API |
| Name/type encoder | Typed IDs -> one append-only output and name-local indexed substitutions. Prefixes and qualifiers use canonical identity; O(consumed facts + output) expected. | 100/200 groups, standard and structural substitutions |
| Functions and special names | Typed owner/terminal/qualifiers/context/thunk facts -> shared encoder; local contexts share the enclosing substitution sequence. | Operators, tags, local/lambda contexts, special names, six root probes |
| Arguments and expressions | Canonical DAGs -> ordered grammar traversal; external entity encodings isolate substitutions where required. | 300–600 groups, equivalent/distinct expressions and template owners |

Production callers construct the same compact facts directly. No C++ parser,
LowIR, native emission or optimizer is introduced in PA9. Existing optional
scaffold vocabulary can evolve without duplicating a production naming path.

## Performance evidence

Measure latency and peak RSS on fixed repeated and growing typed-fact workloads;
record counters, flags, hashes, observations and explicit bounds. No speedup
claim against the nonfunctional stub. PA9 emits names, so generated executable
runtime/text is not applicable. Any later performance comparison requires frozen
binaries/inputs, A/A calibration, ABBA and equivalent output per spec.

## Ledger / handoff

- `7442a3192`: immutable review markers recorded before implementation.
- `df7dbb00a`: complete first encoder; 111/111 default and 117/117 explicit
  contract checks, cumulative 904/904. Baseline binary independently rebuilt
  from this commit with identical SHA-256 (`78784463b5cbd4e...`).
- `f6552f8d1`: sparse name-local substitutions, linear modifier parsing
  and emission, canonical tagged templates, 64-bit ABI indices, explicit
  member/nonmember production shape, fact serialization and optional telemetry.
  117 fixture roundtrips, 11 valid/15 invalid personal cases and file audit pass.
  Sanitizers pass. First frozen benchmark retained as `initial-performance.json`:
  latency/RSS/scaling pass, compiler text growth exceeds the original 100 KiB
  budget. Serializer joining is now shared and its deep modifier traversal is
  iterative; explicit deep roundtrips pass. Compiler text is 265664 bytes, below
  the unchanged growth budget. Final timings use fourfold longer workloads.
  No executable generation is part of this stage.
- First implementation: all **111/111** root-suite fixtures and **117/117**
  independent exact-output/status checks pass. No fixture changed. Remaining
  work is API/serializer validation, scaling, telemetry and cumulative gates.
- Required checks pending: `make test-pa9`, through-PA9 report, file audit,
  explicit personal checks and committed clean status.
- Remaining: all implementation groups above. No handoff boundary reached.
