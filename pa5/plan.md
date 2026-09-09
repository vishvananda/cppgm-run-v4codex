# PA5 final plan and audit ledger

Target: **PA5 full-stage**. Phase: **complete; independent final audit passed**.
Stage base: `f8ec565f979dbd43556b17b4784ffed610304622`.
Last independently reviewed implementation: `f0a0f614a`.
Entry: **0/188**. Current course result: **188/188**, through report **393/393**.

## Final Spec Alignment

| Owner | Final design and invariant | Evidence |
| --- | --- | --- |
| Sources/tokens | Shared PA1–4 immutable source and streaming PP/post/syntax cursors; compact deferred tokens, interned identifiers and valid physical/presumed anchors | Direct API lifetime, macro/#line, cross-file literal, TU-reset and through checks |
| Syntax graph | One 32-byte node array; stable IDs, structured names/type-ids/templates and decoded literal values; child-list mutation preserves both ends | 181 successful course TUs plus 19 audit TUs checked directly; no dump reparsing |
| Parser | Shared prefixes and one grammar construction per region; declarator facts carry name/operator/function scope; explicit statement, parameter and enum ownership | All 188 unchanged contracts, 10 core and 15 extended cases, 19 new regressions |
| Categories/prediction | Flat scope/name facts; separate qualified terminal and scope-category lookup; import/base edges; anonymous namespace identity; bounded delimiter/angle caches and immutable spelling hints | Scope/import-cycle API, binding updates, nested-angle/hint work counters and frozen scaling pairs |
| Driver/view | Per-operand TU ownership and deterministic AST view; ordinary failures; future driver scaffolding remains outside PA5 | Multi-file course cases, exact-output benchmark checks and independent sanitizer compiler |

[Independent audit](audit.md) reconstructs all owners and traces templates,
qualified/nested declarations, literal payloads and release boundaries. PA6+
canonical types, overloads, specialization demand, LowIR/MIR, ELF and executable
optimization/self-hosting have no PA5 surface. They remain later-stage obligations.
No PA5 behavior group or unaudited handoff remains.

## Findings and changes

- `9cfce7949`: correct parameter/enum/control category lifetimes; consistent
  qualified/import lookup and namespace identities; target scopes for every
  typedef declarator; structured for declaration conditions; function/object
  declarator classification; cross-file string source anchors; unnamed-pack
  child-list integrity; working optional name/scope counters.
- `f0a0f614a`: remove optional indentation reuse after an isolated frozen
  comparison failed to beat A/A noise in both blocks or improve peak RSS.
  Candidate and control datasets remain committed evidence; their timings are
  not assigned to final source.

## Performance policy and evidence

The budget recorded before the final campaign remains: paired wall regression
<=10% plus A/A noise; RSS <=15% +1 MiB; host compiler text <=15%; fourfold
input/depth <6x wall and <5x RSS +1 MiB. Delimiters get one visit/token, nested
angle work <2x tokens, and relevant name/scope/AST work must scale within the
fixed envelope. No optimization-driven AST or output growth is permitted.

Frozen A: `7e8d10bf2` (same implementation binary as `8365a1124`). Final B:
`f0a0f614a`. Fixed inputs cover declarations, templates, nested depth, classes,
loops, calls, arrays and floating expressions. Use four primary-file repetitions
per process, eight for nested, to dominate startup. Measure ordinary compilation
with CPU affinity, two A/A pairs, B/B and two ABBA blocks; retain eight startup
probes, independent work/phase runs and calibrated ordinary/stats comparisons.
No builds/tests overlapped timing. Every output was checked before accepting a
run. The exact final campaign passes every hash, protocol, startup and budget
gate: 168 ordinary observations +8 startup +24 work/phase +28 telemetry. Host
text +2.36%; expression latency +1.07–1.64%, largest expression RSS +2.53%;
fourfold nested wall 3.898x with <2x token angle work. No broad speedup is claimed.

[Performance record](performance.md) preserves the earlier indexed campaigns,
the rejected reuse candidate/control, and exact-final-binary observations.
Generated-program runtime/text and actual self-hosting are **N/A at PA5**.

## Validation and handoff ledger

| Commit/check | Result / disposition |
| --- | --- |
| `56944a2bc` | Stage baseline and initial plan reviewed |
| `d5e52d04d` | Cursor, graph, parser and driver foundation reviewed |
| `6d67335c1` | Remaining scoped syntax groups and frozen pre-index A reviewed |
| `8365a1124`, `7e8d10bf2` | Prediction/fact handoffs and completion ledger independently reconstructed; old performance hashes/protocol reverified |
| `9cfce7949`, `f0a0f614a` | Full ownership fixes and profitability decision reviewed |
| Course/through | Fresh PA5 188/188 and root 393/393, five stages pass |
| File audit | 62 files pass |
| Personal | PA5 core 10, extended 15, audit 19; PA2 316 (including 7,062 integer cases), PA4 168 pass |
| Graph/sanitizers | ASan/UBSan/leak checks pass API, graph identities/lifetimes/locations, all 188 contracts and audit probes |

Final performance verifiers and required gates pass. The isolated control
rebuild matches its frozen hash. The final evidence commit closes this ledger;
repository status is verified clean after that commit. Remaining work: **none**.
No fixture/reference/harness/discovery/comparator/timeout changes. All new
implementation sources remain registered. Binaries, objects, AST outputs and
execution logs stay outside committed changes.
