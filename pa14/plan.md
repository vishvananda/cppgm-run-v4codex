# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement**; stage remains incomplete.
Stage entry **84/314**; continuation entry **297/314**; current **306/314**.
**222 original failures resolved; nine this continuation; no lost passes or reduced coverage.**

Active continuation at `2c80bc70`: previous turn is **verified progress**.
Entry rerun confirms 297/314. The next coherent owner is object transfer and
lifetime facts: selected constructors and completed layouts feed explicit
subobject actions, return-destination selection and ordinary LowIR lowering.
Keep C++ triviality distinct from direct storage lowering and preserve empty
object identity without copying nonexistent payload. Preparation stays linear
in class members/return sites; array lowering retains its bounded expansion.
Validate affected course cases, native reference/lifetime reducers, all earlier
PAs and file audit. Freeze entry/current binaries and retain A/A+ABBA latency,
RSS, executable runtime/text observations before claiming performance benefits.

## Design/spec alignment and remaining groups

Canonical declarations/types/arguments feed ordinary semantics and typed LowIR.
Dependent type queries retain parameter ordinals, operation/type/declaration
edges and selected conversions; fixed queries and name bindings are shared.
Lexical definition scopes, fixed-base edges, initialization-prefix jump checks
and retained out-of-class overlays validate unused bodies. Nested specialization
projects original source identities and retains enclosing arguments separately.
[Ownership, data flow, complexity and validation](implementation.md) records scope.

| Remaining owner | Next coherent group |
| --- | --- |
| Inherited transfer/lifetime/layout facts | Reference-member and empty transfers, implicit move returns, constructor entries, virtual destruction, reentrant collection layout/override. |
| Local ABI and O0 expression/control output | Local enum linkage/root provenance, discarded-value loads and constant-condition presentation. |
| Parser/template contexts | Remaining variable-template-defaulted fixture; retain all 314 tests. |
| General typed body facts | Remaining query/bound forms; fixed body type/conversion sharing; dependent-only checking, finer occurrences, typed demand/reverse edges and narrow structured failures. |

The query/binding group was extended through user conversions, operators,
complete-class/condition scopes, out-of-class definitions, alias constructors,
nested source projection and an enclosing-environment cache reducer. Remaining
output failures consume inherited object/ABI/lifetime facts; lookup fixes cannot
repair those choices. Whole-body semantic sharing also needs a broader typed
body graph. These are current PA14 requirements, not PA15 deferrals. This is the
concrete incomplete handoff boundary; no fixture/reference was changed.

## Performance evidence

The [performance review](performance.md) preserves **2,660 verified timed
observations**, including all historical and new frozen campaigns. The context-cache reducer found a correctness
error after the first campaign; that campaign remains preliminary. A second
campaign exposed avoidable expression-record growth. Packing flags restores
**36-byte expressions**, with **112-byte declarations** unchanged. Final A/A+ABBA and direct packing comparisons preserve every outlier. Packing
saves 3,178/4,228 KiB peak RSS on two large workloads for 704 compiler text bytes.
Final compiler text grows 65,280 bytes this continuation. Nineteen common outputs
and five common executables are byte-identical. Fourfold new query/binding input
yields 4.08×/4.06× wall and 3.53×/3.50× RSS; fixed binding work stays constant.
No compiler/runtime speedup or native-size gain is claimed.

O0 adds no optional optimizer or native backend. There is no mandated numeric
compiler threshold. Unsupported inherited diagnostic gates remain diagnostics;
correctness, coverage, ownership and mandated limits remain requirements.

## Active validation

Transfer/layout/ABI increment: PA14 **306/314**, prior **1621/1621**, file audit
passes with three inherited advisories; twelve personal native programs pass.
Entry binary is frozen in `$RALPH_ARTIFACT_DIR/pa14-transfer/`; current performance
measurements remain pending. This is an implementation checkpoint, not a handoff.

## Handoff ledger

Continuation at `e27474c2`: previous 59-case increment is **verified progress**.

| Current increment | Commit / result |
| --- | --- |
| Canonical dependent queries and parameter scopes | `7c6c3528`: 286/314 |
| Shared operator and user-conversion rules | `d8d6f07b`: query/ABI checks and native cases pass |
| Lexical binding and initialization barriers | `0fa48c8e`: 295/314 |
| Retained owner overlays and nested source projection | `af95f4d7`: 297/314 |
| Enclosing-environment cache correction | `47f5f975`: reduced native failure fixed, no lost course passes |
| Packed expression flags | `2dc67391`: 36-byte record restored; course/native/sanitizer checks pass |

Sequential `make test-pa14`: **297/314**; prior through report **1621/1621**;
through PA14 **1918/1935**, only PA14 fails. Eleven personal native programs,
12 unused-body rejections and nine query/ABI checks pass. **325** frozen
release/ASan/UBSan status/output checks and 19 additional rejection checks pass;
parity does not mean 325 course-correct inputs. File audit passes with the same
three inherited header advisories. Current logs, exact exit statuses and frozen
artifacts: `$RALPH_ARTIFACT_DIR/pa14-symbolic/`; previous evidence remains under
`pa14-dependent/` and `pa14-measurements/`. The final evidence verifier passes. This is a verified incomplete checkpoint;
all implementation/evidence changes are committed and the tree is clean.
