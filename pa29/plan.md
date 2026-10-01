# PA29 compact plan — implementation167 handoff

Target: **PA29 full-stage**. Phase: **implementation handoff; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `f07f78236eb475648834ed78afbca6c864f64408`.
Previous review: `cce8634c3c835cf6d5e8f4fa5fea0db213959718`.
Entry HEAD: `7719caea56d56b5c359796e25a08b3f2f659a4c5`.
Implementation code tip: `db6b91a0` (three coherent increments).
The preceding turn was committed audit progress; entry process inspection found
no inherited live compiler/test process. Independent review markers remain intact.

## Completed owner and design alignment

Hosted vector shape and inline-body validation now share the existing pipeline:
parsed attribute expressions → canonical lane/width types and dependent queries
→ substitution/layout/element validation → evaluated function-use edges → typed
ABI/LowIR → native object. GNU vectors retain byte widths; extended vectors retain
lane counts, padded storage and packed boolean layout. Shape, cv, alignment,
traits, partial deduction, packs and SFINAE consume canonical identities.

Type attributes own ordinary parsed expression nodes. Existing occurrence
projection/substitution retains those nodes; no token replay, fake semantic nodes,
name reconstruction, IR text transport or global retry was added. The TU owns
source nodes, canonical types, query facts and sparse body markers. Specialization
keys/caches retain their existing complete context and lifetime. PA9's Vector
node carries either a concrete count or a typed dependent expression edge.

Hosted namespace inline functions are checked independently of emission. Calls
between dormant inline functions use the existing deduplicated function-use
queue; evaluated calls/addresses activate it. Member construction facts retain
their required semantic demand. A discarded direct unknown reserved builtin can
leave an explicit unavailable-body marker, while its operands and surrounding
statements are checked. Using that body fails. No guessed builtin signature or
result type is introduced. Value-producing unknown builtins remain unsupported.
The standalone non-host LowIR tool retains its existing export roots.

Work is linear in parsed attributes, explicit initializer elements, demanded
facts and use edges; canonical lookups are average O(1). Vector layout rounding
has at most 64 steps. Omitted initializer lanes use one repeated tail action,
not storage proportional to the numeric width. There is no new optimizer pass.

## Validation and performance

- `make test-pa29`: **354/403**, exit 2; **53 → 49 failures**, four fixes, no new failures.
- `make test-report-through-pa28`: **4538/4538**, exit 0.
- `make test-report-through-pa29`: **4892/4941**, exit 2; only PA29 fails.
- File audit: pass, the same four substantial-header warnings.
- Explicit controls167: **45/45**; inherited controls166: **21/21**.
- LowIR validation/roundtrip, standalone execution, MIR, object/ABI and telemetry:
  **88 passing inspection checks**, including dependent PA9 fact roundtrips.
- [Validation](../student.tests/pa29/evidence167/validation.json),
  [unchanged coverage](../student.tests/pa29/evidence167/coverage.json),
  [exact failure delta](../student.tests/pa29/evidence167/stage-delta.json), and
  [performance167](performance167.md) retain authoritative evidence.

Performance acceptance is PA29/O0. Frozen A/A+ABBA comparisons measure compiler
latency/RSS and checked runtime/text together; affected layout/wrapper demand
scales at 600/1,200/2,400. All observations, including preliminary measurements
before the dependent ABI correction, are preserved. No speedup is claimed.
Optional work/growth budgets remain **zero**. Historical blanket 15%/zero-growth
self-selected targets remain diagnostics under spec §9. Mandated evaluator/native
limits, timeouts, correctness and coverage are unchanged; broad hosted runtime,
optimizer/allocation and self-hosting constraints retain PA30–34 ownership.

## Remaining implementation and review boundary

The [remaining ledger](../student.tests/pa29/evidence167/remaining.json) accounts
for every required failure. Labels classify ownership, not proven root causes.

| Owner / failures | Unfinished implementation |
|---|---|
| Extended syntax/types/layout: 32 | Numeric/complex value representations, block pointers, designators, folds, templated lambdas, bindings and other remaining vendor forms. |
| Template demand/hosted ABI: 15 | Pack/alias contexts, extern/inline-variable emission and nested ABI ownership. |
| Source-invocation context: 1 | Invocation/default/member-initializer source coordinates. |
| Legacy trait contract: 1 | Forward-declared trait oracle/required behavior remains unresolved. |

Boundary: vector compile-time layout, unused literal validation and associated
inline emission/ABI ownership are complete. The combined extended-vector fixture
still stops at templated-lambda parsing; it also contains vector runtime operations,
which the README excludes as a general requirement. That fixture remains required
and unchanged. Further progress requires distinct parser/closure or numeric/value
lowering work, not another local attribute/layout correction. Runtime vector values
are explicitly rejected rather than scalarized; their diagnostic control is a
capability guard, not a C++ invalidity claim.

Preserve the positive source-invocation and aggregate-mutation reducers, code
alignment placement, dependent offsetof ABI signatures and class-convertible
index designators in their owners. The forward-declared trait and explicitly-false
nothrow-invocable primary oracle questions remain **independent review questions**;
no reference correction or exemption was made. [Audit166](audit.md) remains the
last independent review. Handoff completes this behavior group, not PA29 or its audit.

## Handoff ledger

| Turn | Owner / boundary | Required progress | Review status |
|---|---|---|---|
| 166 | Accumulated assembly/function/evaluation audit, through `f07f7823` | 350/403; PA1–28 and file audit pass | Independently reviewed; full historical ledger remains in audit.md. |
| 167 | GNU/extended vector layout, inline validation/demand, dependent vector ABI; `7db8a253`, `039541f3`, `db6b91a0` | 354/403; no new failures or coverage changes; earlier stages/audit pass | Implementation ready for Ralph; independent review pending, markers preserved. |
