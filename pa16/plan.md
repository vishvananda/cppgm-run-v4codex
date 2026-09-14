# PA16 final plan — audit 44

Stage base commit: `438d56b164600f4fa19d25dcb5f09a76e2a79776`
Last reviewed commit: `95cdc3d8ec4117d69c53a32bceddcf7de6230abb`

Target: **PA16 full-stage**. The independent audit reviewed the complete current
architecture, stage history and all handoffs since `7c39a6ed`. Entry was
`241c6870`, clean. [audit.md](audit.md) records the reconstruction, reducers,
repairs, rules, ownership and scope decisions. All PA16 exit criteria pass.

## Final design/spec alignment

Immutable buffers feed streaming interned tokens and one source graph. Integrated
semantic construction publishes canonical types, selected conversions, layouts,
lifetime actions and demand facts. Template occurrences share source regions and
fixed facts through immutable substitution frames. Typed constants, object parts,
subobject addresses and invocation storage support ordinary expressions and
canonical template queries through the same execution engine.

Constant validity, execution and static-image classification have distinct owners.
A required constexpr initializer includes its conversions and lifetime obligations;
an ordinary failed constant probe retains dynamic initialization. Evaluated
bit-fields carry target-width values into later constructor actions and static
data packing. Selected reference temporaries retain their proper identity before
construction, validation and relocation emission. Only the affected storage
version changes after a late definition; activation keys include reachable
storage dependencies and liveness. Shared value DAGs are traversed once per query.

Lowering consumes typed facts directly. Automatic constant scalar arrays retain
separate stack objects and use one copy each from structurally interned readonly
data. Local statics own guards and destruction records. Program-owned startup and
shutdown coordinators combine TU actions once. Result ABI and cleanup consume
class triviality and final temporary identities. Output text is the requested
LowIR adapter; the supplied backend is used only for authorized validation/native
execution. No frontend or lowering behavior is delegated.

## Audit repairs and validation

The audit repaired bit-field publication/packing, persistent aggregate/class
reference temporaries (including self/subobject addresses), late-definition
cache validity, template query calls/operators/addresses/casts, cast legality in
required constexpr arrays and unselected query branches, and exponential
repeated traversal of shared values.
Two new implementation sources separate constant-query adapters and shared
explicit conversions; both are registered in the frontend source sets. Named
cast lookahead now preserves nested template argument boundaries.

The explicit final controls add **43 native / 10 rejection** cases. Together with
all ten inherited suites, **289 native / 93 rejection** controls pass. Required
file audit passes (three inherited header warnings); `make test-pa16` passes
**154/154** and `make test-report-through-pa16` passes **2266/2266**, all 16 stages.
Sources, fixtures, status sidecars, comparison rules and reference tools are
unchanged from audit entry. The 25 inherited oracle revisions were independently
reviewed against their reduced standard/contract proofs; no new revision was made.

## Performance acceptance

[Final measurements](final-audit-performance.md) preserve all frozen campaigns,
the startup-amplified DAG check, all A/A and ABBA observations, flags, inputs,
output hashes, latency/RSS, native runtime and code/data sizes. The first campaign
identified avoidable eager constexpr-array construction; the final code removes
it while retaining the legality check. Shared-object traversal improves compiler
work with identical output. Common executable bytes remain identical to entry;
no new runtime improvement is claimed.

PA16/O0 has no mandated numerical latency, memory or text ceiling. Earlier
+15% latency, +16 MiB RSS and 5.5x scaling targets are diagnostics under
spec.md's stage-scoped acceptance, not inherited exit gates. Historical
observations and misses remain preserved. Existing 512-call / 1,000,000-step evaluator limits and the small-array
expansion cap remain; correctness and course comparison are unchanged. Native
optimization, MIR/allocation/debug and self-hosting are later-stage surfaces.

## Completion ledger

All object, initializer/lifecycle and result/member handoffs since the previous
checkpoint are reviewed in the audit ledger. No PA16 handoff remains unaudited.
Cross-owner member-pointer conversion and virtual member-pointer ABI are inherited
source-model extensions outside PA12's required member-pointer boundary; the
represented same-owner value family is covered. PA17/18 template extensions are
explicitly deferred by the PA16 handout, not unfinished constexpr work.

[Final evidence](../student.tests/pa16/final-checkpoint.json) and its
[verifier](../student.tests/pa16/verify_final.py) bind the reviewed source commit,
protected trees, control results, required checks and preserved measurements.
