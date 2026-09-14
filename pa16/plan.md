# PA16 implementation — loop 39

Stage base commit: `438d56b164600f4fa19d25dcb5f09a76e2a79776`
Last reviewed commit: `438d56b164600f4fa19d25dcb5f09a76e2a79776`

Target: **PA16 full-stage**. Entry `c2ccf2d5`: **78/154, 76 failures**, clean.
Previous turn: progress (validated storage handoff). Current handoff:
**93/154, 61 failures**; **15 entry failures fixed, no lost passes**.
The implementation handoff ends here; whole-stage implementation and independent
review remain open. Neither review marker has advanced.

## Design/spec alignment and completed groups

| Owner | Data flow, identity, complexity and lifetime | Validation |
|---|---|---|
| `semantic/exception_specification` | Declaration + retained environment -> demand-specific contextual-bool value -> memoized exception fact. Function EntityId owns source/redeclaration chain; a specialization gets its own fact and substitution context. Source class completion drains its queue interval. No unrelated body demand or global retry. | Direct/deferred specs, recursive class completion, suppressed specialization, redeclaration, defaults, prototype names, inherited constructors. |
| `semantic/exception_expression` | Selected calls/conversions, temporary lifetime facts and typed initializer/list plans -> expression/query exception effect. Flat TU indexes; query key includes whether construction materializes a temporary (allocation does not destroy the allocated object). Each actual edge is visited once per complete key; scratch is local. | Calls, indirect calls, operators, user conversions, omitted aggregate/array members, defaulted/inherited constructors, default arguments, temporaries, new/delete and dependent queries. |
| `semantic/constexpr_validity` + type formation | Canonical type/class edges -> structural literal suitability and signature/constructor coverage checks. EntityId caches, bounded by actual declarations and subobjects. Implicit member const enters the canonical function type; template source constraints are not reapplied to concrete runtime specializations. Initializer *contents* remain evaluator work below. | Literal return/parameter/owner rejection, all base/member types, missing constructor initialization, union selection, declaration-only forward types, specifier/redeclaration rules, static/out-of-class and template cv controls. |
| `constant_execution` receiver extension | Checked nullary constexpr constructor with no subobject or body work -> existing stateless receiver identity. Stateful constructors are not represented by that receiver domain. Existing activation limits and scalar keys remain. | True/false contextual conversion; rejects nonconstexpr/effectful construction. |

Inherited scalar execution, target floating values, array ranges, persistent local
storage/lifetimes and structural constant-data interning remain in the cumulative
pipeline. See [scalar evidence](performance.md) and [storage evidence](storage-performance.md).
All new sources are registered in `dev/frontend_source_sets.mk`. Production uses
the shared typed graph/LowIR; no fixture/reference/comparator changes or delegation.
At O0, implicit default constructors retain conservative structural cleanup even
when their exception fact is nonthrowing; no optional cleanup rewrite is required.

## Remaining implementation / concrete handoff boundary

- **Object/address execution:** persistent and activation-local object roots,
  subobject paths and bounds, lifetimes, class/array values, constructor/base/member
  execution, reference/callable values and conversions. `Constant` still carries a
  scalar payload; general calls need object/alias/lifetime inputs in their keys.
- **Initializer and declaration completion:** evaluating full constructor and
  default-member initializer contents, literal-type initializer obligations, and
  class-valued constexpr variable rejection. Structural validity above does not
  certify these remaining expression obligations. Dependent nonliteral runtime
  results also still expose duplicate destructor emission/aliases.
- **Storage depending on those values:** class/static-pointer constants, member
  projections, address-producing calls and demanded static definitions.
- **Exception specifications needing object evaluation:** the remaining
  `400-constexpr-noexcept-decltype-static-assert` failure requires class-valued
  `complete_or_unbounded` evaluation in a dependent class assertion. It is an
  evaluator implementation gap, not an unanswered exception-effect query.
- **Ordinary automatic constant arrays:** README requires a copy, while 16 PA10–15
  fixtures compare element stores under identical flags. Prior storage trial was
  reverted after required earlier comparisons failed. Reducer:
  `int f(){int a[2]={1,2};return a[0];}`. Both forms have correct C++ behavior;
  the allowed miscompilation-reference exception is unproven. Preserve the
  [recorded conflict/evidence](storage-performance.md); this requirement is unwaived.

The completed exception group consumes declaration, initializer and lifetime facts;
further linked failures require establishing general object-valued evaluation,
not extending effect traversal. Doing that needs a new value domain and complete
call/lifetime identities across constructors, references and storage, rather than
more stateless receiver cases. Structural declaration work is an intentional
related increment, with its unfinished initializer-content obligations listed above.

## Performance and required validation

[Loop 39 performance](validity-performance.md): two frozen campaigns, **476**
observations with A/A and ABBA, all samples retained. Compiler text +23,360 bytes
(1.50%); class-heavy compilation +2–5% paired time with essentially unchanged RSS.
Common generated LowIR/native binaries are identical. Work counters scale 4x for
4x source; 4,000 dependent specs demand zero bodies. The repeated campaign does
not reproduce the initial isolated 1.605 default-query timing ratio. No runtime
speedup is claimed. PA16/O0 has no mandated numeric latency/RSS/text ceiling;
historical self-selected diagnostics do not override stage-scoped acceptance.

`make test-pa16`: **93/154**, exit 2 for unfinished implementation.
Exact prior-through command: **2112/2112**, exit 0.
`make test-report-through-pa16`: **2205/2266**, only PA16 failures, exit 2.
File audit passes (three inherited header warnings). Final root reports ran
sequentially because they share `.test_counts`; exploratory overlapping report
counts are not handoff evidence. Personal controls: **24 native/29 rejection
validity**, **41 native/8 rejection exception**, **40 native/23 rejection
scalar/floating**, **27 native storage**. The [manifest](../student.tests/pa16/validity-handoff.json)
and [verifier](../student.tests/pa16/verify_validity.py) bind source/binary, unchanged
coverage, logs, all current measurements and retained historical evidence.

## Handoff ledger / independent review

| Increment | Disposition |
|---|---|
| `d7594d63`, `482c6b44`, `7484f22b`, `cc842756` | Loop 37 scalar group and performance evidence; review pending. |
| `4aab464e`, `66f123ab`, `e85d39a9`, `c2ccf2d5` | Loop 38 storage/interner/lifetimes and validated evidence; review pending. |
| `65c09c3b` | Loop 39 entry and owner/data-flow plan; original markers preserved. |
| `3ff1c1db` | Structural constexpr validity and deferred exception expressions. |
| `3a836964` | Declaration edge cases, stateless contextual conversion, complete initializer/allocation/temporary exception edges and explicit controls. |
| Loop 39 evidence handoff | Required checks, reduction 76 -> 61, source/binary and performance provenance recorded; independent audit pending. |

Independent review must retrace accumulated scalar activation identity/target
floating semantics, array ranges, static classification/lifetime callbacks,
constant-data interning, and new exception contexts/redeclaration/lifetime keys,
structural validity boundaries and stage-scoped performance evidence. These are
review obligations, separate from the unfinished implementation and contract
conflict above. Neither category is waived by this handoff.
