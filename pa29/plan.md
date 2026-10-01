# PA29 compact plan — implementation185 handoff

Target: **PA29 full-stage**. Phase: **implementation handoff; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed commit: `667edd800e4e5eb1b1ef92a3108da3bd96c51708`.
Audit entry: `9211517d1f554f61efa7d6e2e30020dc13171f3f`.
Last reviewed commit: `52070178897f5894edaf2f35d03a734b781979d4`.
Implementation185 entry HEAD: `c6d7a46abacfb0862c3955f2367dd902a6a714e6`.
Implementation commits: `8ee20450`, `27012cf5`.

## Design and spec alignment

[Handoff185](handoff185.md) records owner, data flow, complexity and boundary.
`_BitInt` width expressions are parsed once and retained as canonical type/query
facts. Shared substitution, deduction, constants, conversions and layout retain
exact width/signedness. Ordinary typed lowering consumes these facts, with at
most two shifts per necessary normalization and a transient proof to avoid
repeating it. The scalar engine supports signed 2–128 and unsigned 1–128 bits;
larger concrete widths are explicitly unsupported, never silently narrowed.

The initial dormant-template repairs were extended through demanded bodies,
constexpr/runtime arithmetic, traits, packs/partial specializations, bit-fields,
padding, reference rejection, host ABI and exceptions. Wide ABI stack alignment
is serialized as `i128a8`; ABI fact and LowIR/native adapters preserve it. New
scalar RTTI uses the shared emitter. Scalar work is O(1), dependency work follows
typed edges, and existing interning/frame memoization owns facts and failures.
No new cache family, parser replay, global retry or production text transport.

## Validation and performance

PA29 **392/403**, improving **13 → 11** entry failures; PA1–28 **4538/4538**;
through PA29 **4930/4941**. File audit passes with four inherited warnings.
All **31** explicit controls and **169** inspection commands pass. All **403**
inputs and **1,707** contract/harness paths are unchanged. No reference was
corrected. See [validation](../student.tests/pa29/evidence185/validation.json),
[delta](../student.tests/pa29/evidence185/stage-delta.json) and
[manifest](../student.tests/pa29/evidence185/manifest.json).

[Performance185](performance185.md) retains **272** observations plus eight
launchers. Frozen A/A and six ABBA blocks on four equivalent inherited inputs
produce byte-identical objects; compiler paired medians are **0.9397–1.0254**,
with every paired range crossing unity. All four dimensions, spread, noise and
RSS increases are disclosed. New width-dependent constexpr demand has 600/1200/
2400 scaling: one body transition and one cache hit per specialization, bounded
signature work, and identical **1,698-byte** text while a live 93-bit loop runs.
There is no optional transform or speedup claim. Existing mandated limits remain;
inherited blanket 15%/zero-growth targets remain diagnostic under spec §9.

## Handoff ledger and remaining groups

Completed: bit-precise scalar type/query identity, substitution/deduction,
constants/conversions, arithmetic/storage and typed host ABI boundaries through
128 bits. Wider native scalar arithmetic remains an explicit implementation limit.
The [11-case ledger](../student.tests/pa29/evidence185/remaining.json) distinguishes
**8 unfinished implementation cases** from **3 independent contract questions**.
Neither disposition waives a failure.

Remaining implementation needs floating/complex formats and ABI, vector
expressions/deduction, contextual coroutine syntax and hosted template behavior.
Those diagnostics do not enter the repaired bit-integer width path. Quad/half
floating operations and complex return classification cannot be implemented by
extending integer precision normalization. This is the concrete handoff boundary.

Independent contract questions remain nothrow shorthand, invocable-cache
expectation and nested-template ABI-tag policy. Reserved-name/forward-declaration
observations are not treated as sufficient proof to change an oracle. Independent
review must inspect cumulative changes, including the scalar limit and `i128a8`.
[Audit182](audit.md), [handoff184](handoff184.md), all prior evidence and the review
markers above remain preserved. Full PA29/root-through success and whole-stage
audit are still required before PA30.
