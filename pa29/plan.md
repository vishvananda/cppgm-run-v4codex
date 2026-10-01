# PA29 compact plan — implementation161 in progress

Target: **PA29 full-stage**. Phase: **implementation; stage unfinished**.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `1ab3499d7046daf5c298d958a8770b413edb3615`.
Entry161: `7f0a2c4b06f7b81442c41c8f5bbdb6ed423977e4`, clean, **323/403** (80 failures).
Previous goal turn: progress (implementation160 completed and validated invocation ownership).
No surviving build/test process was found at entry.

## Active ownership group

Implement the C11/GNU atomic frontend over existing typed LowIR/native atomic
operations. Owners: syntax/type interning for atomic storage; semantic builtin
registry/signature selection for pointee, order, effects and result facts;
lowering for direct atomic operations and bounded CAS loops. Preserve literal
type/cv distinctions and 16-byte alignment. Probe answers derive from implemented
registries; intrinsic exception facts feed noexcept analysis. Work scales with
source operands and emitted operations, with no whole-program search or optional
optimization. Validate runtime/concurrency, rejection, layout, LowIR roundtrips,
all existing fixtures and earlier stages. Freeze entry binary before edits;
measure A/A+ABBA common workloads and new capability costs, runtime/text/RSS.

Prior handoff evidence below remains authoritative until refreshed.
Code endpoint: `2fd6963e`. Current: **323/403**, **80 failures**, two resolved,
zero new failures. All 403 fixtures, references and comparison rules are retained.
Previous goal turn: progress (handoff159 changed code and validated 321/403).

## Completed ownership and spec alignment

[Implementation160](implementation160.md) traces intrinsic member invocation from
source and template queries through selected receiver/argument facts to typed
LowIR and native calls. Direct/base, raw-pointer and pointer-like receivers share
member-pointer validation and typed unary recipes. Queries retain cv/ref/access,
base adjustments, ADL, conversion and exception facts; evaluated uses materialize
temporaries without mutating query recipes. Constexpr evaluation shares the facts.
The shared query path now preserves each expanded parameter-pack environment.
Fixed template invocations retain source recipes. No reparse, synthetic syntax,
text transport, lowering-time resolution or global retry is introduced.

Work follows required candidates, arguments and base edges. TU-owned compact
indices and canonical query keys bound facts to their semantic owners. Repeating
the same invocation queries 100 → 10,000 times keeps query work at 11 and body
demand at 0; explicit `declval` source-name binding remains linear in occurrences.
Earlier [trait ownership](implementation159.md) and its audit obligations remain.

## Remaining implementation and independent review

[The fixture ledger](../student.tests/pa29/evidence160/remaining.json) preserves
every failure and disposition; owner labels alone are not root-cause proof.

| Owner / failures | Remaining work |
|---|---|
| Atomic/assembly: 21 | Atomic storage/order/effect facts, assembly constraints, layout and noexcept controls. |
| Extended syntax/types/layout: 37 | Extended numeric/complex types, vector width/layout, unused-wrapper validation, designated initialization, folds, lambdas and bindings. |
| Template demand/hosted ABI: 19 | Packs/aliases/context keys, pretty-function, extern/inline emission and ABI; includes the contract question below. |
| Structured intrinsic operands: 2 | Constant-evaluation/address/fence semantics and source-location facts. |
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

- `make test-pa29`: **323/403**, exit 2. [Delta](../student.tests/pa29/evidence160/stage-delta.json): two existing failures removed, none added.
- `make test-report-through-pa28`: **4538/4538**, exit 0.
- File audit: pass, four inherited substantial-header warnings.
- Explicit controls: **46/46**. Query sharing, selected native calls, LowIR
  roundtrip execution and telemetry/object byte equality pass.
- [Validation hashes](../student.tests/pa29/evidence160/validation.json) and
  [performance160](performance160.md) retain frozen A/A+ABBA observations,
  latency/RSS, checked runtime/text, demand scaling and timing spread. Common
  objects/executables are byte-identical. No speedup claim is made.

No new optional optimization is introduced; its work/growth budgets are **zero**.
Historical blanket 15%/zero-growth targets remain diagnostics under spec §9;
measurements and mandated limits remain intact. PA29 excludes runtime vector
lowering; broad hosted headers/runtime, optimization/allocation and self-hosting
retain PA30–34 owners. Required PA29 behavior is not waived.

## Handoff ledger and boundary

| Increment | Result |
|---|---|
| `2fd6963e` | Typed member invocation, query pack environments, fixed recipes, constexpr/lifetime/exception integration; both invocation failures resolved. |
| Final evidence | 46 explicit controls, shared-query/native inspection, frozen performance, complete serial validation and the remaining-failure ledger. |

The invocation group is complete. Source locations, constant-evaluation modes,
fences/atomics and the other remaining groups require distinct facts beyond the
completed receiver/conversion recipe. Further work therefore starts a new
semantic ownership group, making this the implementation handoff boundary.
PA29 remains unfinished. Ralph's independent audit and a full through-PA29 pass
remain required before advancement. Stage base/review markers are preserved.
