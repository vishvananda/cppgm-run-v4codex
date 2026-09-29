# PA20 final independent audit — loop 101

Stage base: `a9b24ab68f1a75288df10161cb171fa239e1409a`.
Audit entry: `8c86c298`; reviewed implementation: `806b38fb`.
Target: **PA20 full-stage**, `--emit-lowir -O0`. Final Spec Alignment: **pass**.
The earlier checkpoint is preserved verbatim in [audit97.md](audit97.md).
This review covers the entire stage, including previously unaudited handoffs
98–100. Its evidence is the owners and current executions below, independently
of checkpoint conclusions. The preceding handoff is **progress**, verified by
its commits, source and primary log; no prior task process was live at entry.

## Finding and repair

**Captured range storage was lost at the implicit range binding.**
`range_initializer` checked an outer identifier and recorded its capture.
`resolve_range` then reused its original EntityId as `RangePlan::range`.
Lowering's `binding(entity)` bypassed the capture projection and allocated
unrelated local storage, or reused a foreign parameter slot. Arrays, aliases,
member/ADL ranges, nested lambdas and specializations were affected. Course
fixtures did not cover this interaction. This was a correctness and semantic
fact-consumption defect.

The `captured_array` reducer in [audit101_controls.py](../student.tests/pa20/audit101_controls.py)
captures `{3,4}`, sums it by range-for and checks 7. Entry returned failure;
an alias case faulted and parameter cases failed validation with foreign slots.
[Entry evidence](../student.tests/pa20/audit101-entry-controls.json) is **7/29
passing, 22 failing**. Current controls are **29/29**, including mutation,
const/volatile/multidimensional arrays, nested forwarding, both callable entries,
fixed/dependent class ranges, early-exit cleanup and missing-capture rejection.

[C++11 range-for equivalence](https://timsong-cpp.github.io/cppwp/n3337/stmt.ranged#1)
binds the range expression through a reference before obtaining endpoints.
[Reference and nested capture rules](https://timsong-cpp.github.io/cppwp/n3337/expr.prim.lambda#15)
preserve the captured object's identity. Fresh storage is not a valid replacement.
These rules establish the expected behavior independently of compiler agreement.

The repair in [semantic/range_statement.cpp](../dev/src/semantic/range_statement.cpp)
records storage observation and allows the direct entity shortcut only when
the checked identifier has no capture. Captured ranges use existing hidden
reference initialization, consuming their source expression and capture ID
once. Array addresses, member receivers, ADL arguments, conversions and lifetime
paths consequently see the correct object. No lowering lookup, new cache,
duplicate tree, fixture change or reference correction was introduced.

## Independent architecture reconstruction and Spec Alignment

| Spec | Actual owners and data flow | Conclusion |
|---|---|---|
| §1 | Immutable SourceBuffer bytes/file IDs → Preprocessor → PostTokenCursor → syntax::Cursor ring → Parser::translation_unit → Analyzer::consume. Tokens carry interned IDs and source locations. | Streaming phases; consumed tokens are released. No successive owning token streams or text phase transport. |
| §§1–2 | NodePool stores source nodes once and `(source,context)` occurrence identities. `angle_interpretation.cpp` uses the real lexical scope, retained operands and shared precedence table. `source_region` publishes topology; `resolve_source_node` forbids later mutation or interpretation during instantiation. | One parse per region. No syntax clone/replay; source locations and wrappers are retained. Category alternatives, lexical hiding, aliases, operand forms and failed angle-probe caching were reviewed and rerun. |
| §§2–3 | Canonical Types/EntityId/ScopeId/argument-pack IDs, FactStore slabs, ExpressionStore shared properties and occurrence uses, flat IdIndex. Lookup indexes `(scope,name,kind)` and follows lexical/import/base/ADL edges. Selection filters candidate shapes and records conversions. | No rendered semantic keys, unrelated registry scans, global generation invalidation or lowering overload reconstruction on these paths. Expected candidate rejection is a compact result; demanded hard errors retain diagnostics. |
| §§4–5 | `specialize` keys canonical pattern/argument tuple with a separate explicit pack-prefix owner. Substitution-frame keys include specialization, parameter slice/count, parent and arguments. Instantiation projects retained source and uses separate monotonic fact states. `finish` advances deduplicated demand queues; query dependencies target affected consumers. | No visible-environment snapshots, global retries, repeated completed bodies or blanket cache clears. Incomplete failures retain query-specific prerequisites/revisions. Class completion does not demand unrelated bodies. |
| §§2,4,6 | `placeholder.cpp`, `placeholder_return.cpp`, query/list initialization establish cv/reference types, extents, selected conversions and initialization actions. Return deduction uses function-body state and dormant use edges, including unevaluated demand. | Plain-value deduction avoids needless scratch maps without changing rules. Queries and evaluated lowering share typed facts. Current deduction/array/conversion controls and traces 94–95 pass. |
| §§4–6 | `lambda.cpp` owns source/specialization closure identity, one checked body, defaults and exception facts. `closure_capture.cpp` indexes `(closure,original declaration)` and forwarding edges. ObjectUse records capture IDs; source this differs from the ABI receiver. Ordinals are assigned separately. | Handoff 98 reviewed through construction, copying and calls. Captureless object/pointer entries share semantics/statics but own slots/labels; capturing closures have no pointer conversion. The range bypass found here is repaired at its owner. |
| §§2,6 | `range_shape` records array shape or begin/end/test/increment/dereference selections. Fixed recipes are retained; concrete uses own storage/conversions. Jump validation records loop/body lifetimes; range/typed-operation lowering consumes them. | One source binding, at most five operations, two iterators or one index. Range and element temporary lifetimes survive early exits. No fabricated AST or repeated resolution. |
| §§6–7 | `initializer_effects.cpp` summarizes checked expressions/completed constructors. Missing bodies are conservative uncached answers. List initialization records helper safety/copy/transfer facts. Helpers use target/complete shape or transfer-plan identity. Class-value facts distinguish parameter and result ABI. | Aliases, unknown effects, self/destination observation and volatile accesses retain ordered paths. Representation transport requires trivial copy/destruction and independent construction. Shared helpers own slots; nontrivial transfers retain their selected operations. Signatures/calls consume one recorded ABI convention. |
| §§6,8 | TU-owned source/identifier/node/fact vectors, slabs and flat indexes; function builder/lifetime/call scratch reset after each body. The driver releases frontend/lowering owners after each TU and retains typed Program/ABI linkage until output. | No hot per-node shared ownership, duplicate semantic tree, recursive syntax destruction, global mutable cache or retained textual IR. The typed Program is PA20's required output, not a dead phase retained by a native compiler. |
| §§6,10 | Procedural directly constructs typed instructions/operands/signatures. Linkage keys use typed ABI graph identities before final name rendering. Full validation is an explicit audit option. | No production LowIR reparse or repeated full validation. Current syscalls show one compiler process and no reference/cached-answer reads; only the independent harness invokes the supplied backend. |

## End-to-end and optimization traces

[audit101_trace.cpp](../student.tests/pa20/audit101_trace.cpp) follows a nontrivial
Range declaration and demanded `sum<int>`/`sum<long>` templates. Array member
initializers, auto endpoint returns, layout and destructor actions become
canonical facts. Three calls reuse two specialization bodies. Nested closures
forward the original arrays; hidden range references consume their capture
projections. The ordinary class range calls selected member endpoints, mutates
the original array and destroys its object exactly once.

[The current record](../student.tests/pa20/audit101-trace.json) retains complete
LowIR, compiler syscalls and native payload disassembly: **2 template body
transitions, 5 closures, 3 range plans, 12 checked bodies**, **1640-byte** backend
payload, checked exit zero. Ordinary and instrumented/validated LowIR are
byte-identical. Capture projection becomes pointer loads; O0 loops retain
index/iterator loads, compare, branch and increment. The supplied backend's
stack/frame traffic is visible; no allocator improvement is claimed.

Six inherited traces were independently rerun: deduction/array packs (94),
range conversion/lifetime (95), aggregate/callable entries/shared statics (96),
reference/this captures (98), shared helpers/result ABI (99), and retained angle
grammar (100). Their current assertions/results are in
[validation](../student.tests/pa20/audit101-validation.json).

The helper optimization audit follows storage independence from a checked
expression/constructor through `helper_safe`/`helper_copy` to helper/copy IR or
ordered destination actions, then supplied-backend encoding. Completed summaries
are immutable and keyed by occurrence/constructor; unavailable bodies do not
poison negative caches. Self-observing and independent controls exercise both
paths. Required helper representation has no optional profitability decision;
the separate pointer-wrapper removal has historical executable evidence.
Fewer IR nodes alone are never the runtime-profit argument.

Pipeline budgets: one parse and completed fact per complete key; demanded
dependency edges only; one plan per range occurrence; five implicit operations;
one capture per `(closure,object)`; at most two demanded bodies per captureless
closure plus its conversion entry; one helper per complete shape/transfer key
with O(fields) work. The nested initialization expansion product remains **8**,
with loop/zero fallback. No fixed-point optimizer, unbounded inlining/unrolling,
or new search/growth policy is introduced. Student MIR/allocation/ELF/debug,
optimization levels and self-hosting are later PA24–34 surfaces, not available
PA20 implementations to audit. They are not waived PA20 requirements.

## References and validation

All **14** stage reference revisions were reconstructed and verified: eleven
[array images](reference-corrections95.md), two [closure conversions](reference-corrections98.md),
one [aggregate copy](reference-corrections99.md). Their original bytes, reduced
sources, pinned bundle revision/hash and cited proofs remain preserved. PA16's
explicit readonly-array/copy rule establishes the array output requirement;
C++11 initialization/object identity and LowIR copyobj establish preservation.
The other proofs preserve closure type during deduction, forbid chained implicit
user conversions, and require ordered lvalue member copies without extra moves.
The cited draft rules were read again; reconstruction never reads student IR.

Old/revised/student array executions and closure/aggregate reducers pass their
specified results. The manifest verifies the exact changed-reference set and
**639 unchanged fixture/contract/harness files** since entry. All 144 PA20 source
identities/statuses and comparison rules are preserved. No new oracle was needed.

- `make test-pa20`: **144/144**, exit 0.
- `perl scripts/cppgm_file_audit.pl --stage pa20 --paths dev/src`: **pass**, exit 0.
  Three inherited advisory header-body warnings remain (`procedural.h`,
  `analyzer.h`, `model.h`), with no new diagnostic or changed limit.
- `make test-report-through-pa20`: **3596/3596**, all 20 stages, exit 0;
  separately printed PA10/PA11/PA12 focused controls also pass. The raw entry
  primary log has the same denominator; external 3620 status is not substituted
  for the harness's actual count.
- Personal controls: **373/373**, four ABI checks, six inherited traces, the new
  source-to-native/syscall trace, and all reference proofs/executions.

[Final performance evidence](final-audit-performance.md) reports frozen A/B,
A/A calibration, ABBA pairs/spreads, compiler latency/RSS and checked native
runtime/payload together. Historical observations and mandated limits remain.
Inherited self-selected percentages are diagnostics under spec §9, not extra
exit gates. Final stage-scoped acceptance follows that evidence.

## Ledger and closure

| Boundary | Audit 101 disposition |
|---|---|
| 94–96 and audit 97, through `882cf523` | Shared architecture reconstructed; controls/traces rerun; historical review and measurements preserved. |
| 98, `e75e0d6c..a1faea7a` | Captures, nested forwarding, this, packs, conversions and two reference proofs reviewed. Range composition defect repaired. |
| 99, `16ac49da..37f8300b` | Completed safety/cache ownership, helper slots/sharing, ordered transfers and result ABI reviewed; reference proof reproduced. |
| 100, `2e31ab57..7925464d` | Category alternatives, lexical publication, precedence, source projection and negative probe bounds reviewed; all 41 controls and trace pass. |
| 101, `8c86c298..806b38fb` | Range repair, 29 new controls (22 entry failures), full validation, current native/syscall trace. Final measurement/records follow without production changes. |

No unaudited PA20 handoff or known remaining stage correctness, self-containment,
timeout, architecture or file-audit defect remains. The later-stage boundaries
above preserve the assignment scope and defer no PA20 requirement.
