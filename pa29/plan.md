# PA29 compact plan — implementation161

Target: **PA29 full-stage**. Phase: **implementation; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `1ab3499d7046daf5c298d958a8770b413edb3615`.
Entry161: `7f0a2c4b06f7b81442c41c8f5bbdb6ed423977e4`, clean, **323/403** (80 failures).
Code endpoint: `802f28fd`. Current: **338/403**, **65 failures**, fifteen resolved,
zero new failures. All 403 fixtures, references and comparison rules are retained.

## Completed ownership and spec alignment

[Implementation161](implementation161.md) traces atomic storage from one parsed
`_Atomic` type through canonical type/layout/substitution and ABI facts, selected
builtin signatures/conversions, typed LowIR operations and native output.
Atomic identity survives cv removal, pointers, overloads and template deduction.
C11/GNU/sync forms share one immutable registry; signatures are TU-owned and
cached by operation/form/pointee identity. Probes reflect that registry.
Nonthrowing calls, class destinations, padding and expected-value updates are
preserved through serialization. Misaligned/large generic representations use
ordinary libatomic runtime calls. Weaker orders conservatively use seq_cst;
weak compare-exchange uses strong semantics. Operands evaluate once.

Compiler work follows arguments and object payloads. Each RMW emits one bounded
IR recipe/CAS loop; runtime retries do not trigger compiler search. No reparse,
name-based lowering recovery, production text transport or global pass is added.
[Invocation160](implementation160.md), [traits159](implementation159.md) and their
independent review obligations remain part of the cumulative implementation.

## Remaining implementation and independent review

[The fixture ledger](../student.tests/pa29/evidence161/remaining.json) preserves
every failure and disposition; owner labels alone are not root-cause proof.

| Owner / failures | Remaining work |
|---|---|
| Assembly: 6 | Parsed constraints, input/output operands, clobbers and effect recipes; includes locked update and memory fences. |
| Extended syntax/types/layout: 37 | Numeric/complex types, vector width/layout, unused-wrapper validation, designated initialization, folds, lambdas and bindings. |
| Template demand/hosted ABI: 19 | Packs/aliases/context keys, pretty-function, extern/inline emission and ABI; includes the contract question below. |
| Structured intrinsic operands: 2 | Constant-evaluation/address semantics and source-location facts. |
| Legacy trait contract: 1 | Forward-declared std-trait oracle question retained from audit158. |

Inherited implementation obligations include code-alignment placement, dependent
offsetof ABI signatures and class-convertible designator indices. Extended float
suffix recognition does not implement extended precision.

Independent contract questions remain: undefined `std::is_nothrow_*`
specializations and the reserved-name nothrow-invocable fixture whose primary is
explicitly false. They do not authorize library-name shortcuts; failures remain.
Audit must also review invocation recipe ownership/environment keys and inherited
binding phase order, class-completion cache validity, and structural versus
usable-member facts. Review questions do not replace unfinished implementation.

## Validation and performance

- `make test-pa29`: **338/403**, exit 2; fifteen existing failures removed,
  none added ([delta](../student.tests/pa29/evidence161/stage-delta.json)).
- `make test-report-through-pa28`: **4538/4538**, exit 0.
- `make test-report-through-pa29`: **4876/4941**, exit 2, only PA29 fails.
- File audit: pass, four inherited substantial-header warnings.
- Explicit controls: **45/45**. LowIR validation/roundtrip/native execution,
  locked instructions, ABI names and telemetry/object equality pass.
- [Validation hashes](../student.tests/pa29/evidence161/validation.json) and
  [performance161](performance161.md) retain frozen A/A+ABBA observations,
  latency/RSS, checked runtime/text and demand scaling. Five equivalent A/B
  object/executable pairs are byte-identical. Common paired compiler ratios
  are 1.0116–1.0175; the measured cost and spread are disclosed. No speedup claim.

No new optional optimization is introduced; its work/growth budgets are **zero**.
Historical blanket 15%/zero-growth targets remain diagnostics under spec §9;
measurements and mandated limits remain intact. PA29 excludes runtime vector
lowering; broad hosted headers/runtime, optimization/allocation and self-hosting
retain PA30–34 owners. Required PA29 behavior is not waived.

## Handoff ledger and boundary

| Increment | Result |
|---|---|
| `802f28fd` | Atomic types, signatures, ordinary scalar access, C11/GNU/sync lowering, class padding, fallback ABI and explicit controls; fifteen failures resolved. |
| Final evidence | 45 explicit controls, serialized/native inspection, frozen A/A+ABBA costs, serial validation and remaining-failure ledger. |

The atomic group is complete, including related padding, type-identity,
serialization and fallback defects exposed by independent controls. The six
assembly cases require a new constraint/operand/clobber model; extending atomic
builtin signatures cannot represent those source facts. That concrete ownership
boundary makes additional related work a new implementation group. Other
syntax/template/ABI failures are also unfinished groups, not waived requirements.
PA29 remains unfinished. Ralph's independent audit and a full through-PA29 pass
remain required before advancement. Stage base/review markers are preserved.
