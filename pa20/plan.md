# PA20 implementation plan — handoff 96

Stage base commit: `a9b24ab68f1a75288df10161cb171fa239e1409a`.
Last reviewed commit: `a9b24ab68f1a75288df10161cb171fa239e1409a`.
Target: **PA20 full-stage**. Phase: **implementation handoff; stage incomplete**.
Turn entry: `e59e46fa8ccef7dda44cbf57f5d9f4075801861a`, **110/144**,
34 failures. Previous turn made verified progress (19 original failures fixed);
its [ledger](../student.tests/pa20/validation95.json) remains preserved.

## Design/spec alignment and remaining groups

| Owner | Data flow / complexity | Validation / remaining implementation |
|---|---|---|
| Deduction, conversions and ranges (handoffs 94–95) | Canonical initializer/return/range facts, fixed recipes and lexical lifetimes → direct LowIR; demand scoped by source occurrence/context | Inherited controls and required prior-stage suites rerun; [range evidence](performance95.md) preserved |
| Scalar array member helpers (implemented) | Semantic initializer actions/field identities → complete typed parameter shape → ordered argument initialization and representation copies; O(fields) per site, one helper per complete type/arity key | Nested/omitted arrays, scalar holes, strings/enums, volatile fallback, large-tail budget, template returns and sequencing; original array-member failure fixed |
| Captureless callable entries and defaults (implemented) | One checked closure body/context → selected call declaration plus ABI entry identity → separately owned blocks/slots; at most two demanded body emissions, no repeated body analysis | Defaults and definition-time lookup, pointer conversion, direct invocation, shared statics, class/reference ABI, access, cleanup, labels/switches/ranges; ten original failures fixed |
| Nontrivial aggregate member helper ABI (unfinished: 2 failures) | Selected member construction and omitted class initialization must agree with helper transfer/storage/lifetime contracts | Braced aggregate returns with class copies; omitted class tail. Interleaved transfer sequencing is fixed, but these required shapes remain unresolved |
| Capture environments (unfinished: 17 failures) | Closure occurrence + enclosing specialization → capture fields/access, local/this rebinding, special members and pack contexts | Local/this/nested captures, traits/assignment, friend/member lookup and template/pack composition |
| Constructor conversion integration (unfinished: 2 failures) | Overload/deduction and permitted user-conversion sequence → selected constructor argument representation | Lambda constructor-template preference and wrapper conversion; current closure-vs-pointer reference discrepancies need a reducer/proof before any correction, or implementation correction as warranted |
| Retained declarations/lifecycle (unfinished: 2 failures) | Complete enclosing environment → retained local declaration/probe and ABI/lifecycle facts | Dependent-owner lifecycle and repeated local declaration probe |

No new source file requires registration. No parser rewrite, synthetic syntax,
textual phase transport or lowering-time lookup was added. The complete
[performance/architecture record](performance96.md) describes ownership,
complexity, budgets and source-to-native validation. No references, source tests,
statuses or comparison rules changed; prior
[reference corrections](reference-corrections95.md) remain preserved.

## Performance and handoff ledger

Frozen entry/final binaries, inputs and flags: **572 observations / 58 warmups**,
A/A calibration and four ABBA blocks per common compiler/runtime workload.
Compiler text grows **2,624 bytes (0.127%)**. General native controls are
byte-identical. Pointer-closure runtime pairs improve to **0.796–0.859×** with
50 fewer payload bytes. Required array-helper representation costs **3.015–3.034×**
runtime and +58 payload bytes in the affected loop; this is disclosed required
PA20/O0 contract work, not an optimization benefit. The 3,200-array compiler
peak RSS is 96,424/95,780 KiB; the prior run's +8.35% and timing noise are retained
without attributing RSS variation to the cache. Array classification work stays
one at both 800/3,200 specializations (3,202/12,802 hits). Spec §9 stage scoping
applies; no unsupported percentage gate is introduced or inherited. Fixed
work/growth limits and correctness/coverage remain mandatory.

- `beb83901`: scalar array member helpers, complete parameter shapes for empty
  scalar holes, and destination construction for interleaved class transfers;
  16 executable/rejection controls. PA1–19 passed before continuing.
- `6fbfd99f`: destination closure construction; lexical defaults; selected
  receiver-free callable entries; demand only emitted entries; shared checked
  body/parameter facts with function-owned labels/range slots; 36 controls.
- `5d2e6a65`: canonical array-type eligibility cache, including shared nested
  tails, with work/hit telemetry; no repeated type walk per helper argument.
- Evidence commit: [validation ledger](../student.tests/pa20/validation96.json),
  [raw measurements](../student.tests/pa20/performance96.json),
  [source-to-native trace](../student.tests/pa20/trace96.json), and this boundary.

Required checks: `make test-pa20` **121/144** (exit 2); PA1–19 **3452/3452**;
through-PA20 **3573/3596** (exit 2); file audit passes with three inherited header
warnings. **11 original failures removed, no new failures**, without reduced
coverage. Personal controls: **188/188** (136 inherited + 16 aggregate + 36
closure), plus the source-to-native trace. Local stage progress passes; the
full-stage and through-PA20 gates remain incomplete.

Handoff boundary: scalar/array representation and captureless callable/default
ownership now have coherent implementations and executable validation. Further
related fixes exposed by their work (parameter holes, transfer sequencing,
shared-entry labels and range storage, fixed default lookup) are included.
The remaining features need capture-field/environment construction, a separate
constructor/member-transfer ABI decision, or retained declaration/lifecycle
handling. They cannot be supplied by more scalar-array transport or callable
entry/default changes. These are concrete unfinished implementation groups,
not deferred reviews and not waived requirements.

Independent review remains pending for these three implementation commits,
handoff 95's `2cbd6b8d`, `14fff139`, `58c89851`, `fe0c1722`, and handoff 94's
`dbd1a8f6`, `df239d8e`, `0dd795a1`. Review should assess helper key completeness,
initializer sequencing, closure entry demand/body reuse, function-local storage,
default binding, performance cost classification, and the prior range/deduction
and reference-proof questions. Review markers remain unchanged. Ralph's
independent audit must resolve whole-stage findings before advancement.
