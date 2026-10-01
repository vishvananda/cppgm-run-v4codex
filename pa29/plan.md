# PA29 compact plan — implementation164 in progress

Target: **PA29 full-stage**. Phase: **implement; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `cce8634c3c835cf6d5e8f4fa5fea0db213959718`.
Previous review: `1ab3499d7046daf5c298d958a8770b413edb3615`.
Implementation163 entry: `3bc61ecba6d68eb2821022d7d58ac53ae5305f84`, **338/403**.
Code tip: `a6f3d6a2`. Current: **345/403**, **58 failures**; seven removed,
none added. All 403 fixtures, references, sidecars and comparison rules remain.

## Active implementation164

Entry: `e5690337`, **345/403**, 58 failures. Previous turn: progress (committed
assembly behavior and verified checks); no inherited live process remains.
Review markers above remain unchanged.

Initial owner group: function-context strings and source-location intrinsics.
Trace declaration/specialization identity and source coordinates into semantic
query facts, constant storage and direct LowIR/object emission. Fix pretty-name
rendering, enclosing-template argument retention and dependent array queries
together; extend to the source-context builtin family where the same facts apply.
Use one string per declaration/kind, immutable query keys including context,
linear rendering in emitted bytes and no grammar replay or semantic text keys.
Record remaining unrelated owners separately from independent review questions.

Validation: focused required fixtures plus explicit personal controls, full
PA29/prior-through reports, file audit, LowIR roundtrip/object behavior, frozen
common A/A+ABBA and affected demand scaling (latency/RSS/runtime/text). PA29/O0
acceptance; no optional transform/work/growth budget is added. Entry binary and
required-suite log frozen in `/tmp/pa29-164` before implementation.

## Design and completed group

GNU assembly statements now flow from one parsed source recipe and ordinary
operand expression nodes through semantic binding, substitution, typed LowIR,
MIR and direct ELF emission. [Assembly163](assembly163.md) records ownership,
validation, supported recipes, limits and the handoff boundary. Source recipes
are shared across demanded instances; operands evaluate once. Statement scratch
is bounded by 30 operands; compiler work is linear in bytes, operands and emitted
instructions. Additive/exchange atomics reuse existing IR; other locked updates
use fixed-size CAS loops. No compiler fixed point, semantic reconstruction,
external assembler, optional optimization or whole-program search was added.

Work extended beyond the initial six failures to matching inputs, operand
aliasing, same-register xadd, exchange operand order, widths, volatile/memory
behavior, template lookup, temporary/exception cleanup, native opcode dispatch,
and debug/LowIR validation. The attribute/template fixture also now passes.

The prior [audit162](audit.md) made progress through four atomic ownership
corrections and evidence, while the course failure count stayed at 65. Entry
reconciliation found no inherited live process. Its review markers and
[performance162](performance162.md) remain unchanged; this implementation is
not independently reviewed.

## Validation and performance

- `make test-pa29`: **345/403**, exit 2; seven fewer existing failures.
- `make test-report-through-pa28`: **4538/4538**, exit 0.
- `make test-report-through-pa29`: **4883/4941**, exit 2; only PA29 fails.
- File audit: pass; four inherited substantial-header warnings.
- New controls: **53/53**; inherited atomic controls: **45/45, 49/49**.
  Inspection validates native opcodes, serialized LowIR/object execution,
  concurrency, noexcept boundaries, MIR debug locations and telemetry equality.
- [Validation](../student.tests/pa29/evidence163/validation.json),
  [coverage](../student.tests/pa29/evidence163/coverage.json),
  [delta](../student.tests/pa29/evidence163/stage-delta.json) and
  [performance163](performance163.md) retain the concrete evidence.

Performance acceptance is PA29/O0. Frozen common A/A+ABBA compares equivalent
correct implementations; affected measurements record necessary semantic costs
and demand scaling. No speedup is claimed. New optional work/growth budgets are
**zero**. Historical blanket 15%/zero-growth gates remain diagnostics under
spec §9; measurements and mandated limits are preserved. Broad hosted runtime,
optimizer/allocation and self-hosting remain PA30–34 responsibilities. An initial
host-DWARF inspection assumption was corrected against PA8's explicit contract;
required LowIR/MIR debug transport and all course checks remain.

## Unfinished implementation

The [remaining ledger](../student.tests/pa29/evidence163/remaining.json) retains
all 58 failures; owner labels are not root-cause proof.

| Owner / failures | Work |
|---|---|
| Extended syntax/types/layout: 36 | Numeric/complex types, vector width, unused-wrapper validation, designators, folds, lambdas and bindings. |
| Template demand/hosted ABI: 19 | Packs/aliases/context keys, pretty-function, extern/inline emission and naming. |
| Structured intrinsic operands: 2 | Evaluation-mode/address semantics and source-location facts. |
| Legacy trait contract: 1 | Forward-declared std-trait oracle question; required behavior remains unresolved. |

Also retain code-alignment placement, dependent offsetof ABI signatures,
class-convertible designator indices and extended floating precision in their
owners. Runtime vector lowering is explicitly outside PA29, not a waived
compile-time layout requirement. Next implementation should finish another
coherent owner group rather than seek individual fixture symptoms.

## Independent review and handoff ledger

| Boundary | Implementation | Independent review |
|---|---|---|
| audit162 / `cce8634c` | Atomic identity/storage corrections; 338/403. | Reviewed through the preserved marker; whole stage unfinished. |
| implementation163 / `a6f3d6a2` | Required assembly family complete; 345/403. | Pending recipe/effect/ownership/performance review; not waived. |

Existing forward-declared trait and explicitly-false nothrow-invocable primary
oracle questions remain unresolved. No library-name shortcut or reference
correction was made. The remaining fixture failures remain implementation work
even where their contract interpretation also needs independent proof.

Handoff boundary: no known defect remains in the implemented assembly subset.
All related required fixtures and cross-owner controls pass. The 58 remaining
failures require distinct type/layout, template/ABI or intrinsic-context facts;
extending arbitrary assembler syntax would not resolve them. Continuing within
this group's understanding therefore has no remaining required failure to own.
This is an incomplete full-stage handoff, not advancement or an independent
audit. A full through-PA29 pass and whole-stage audit are still required.
