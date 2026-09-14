# PA15 implementation handoff

Stage base commit: `8000f3c8ef4647d57f2c0775192585f14cab33d8`
Last reviewed commit: `538cfcb00441f57c0629f6d27fddbad723539479`

Target: **PA15 full-stage**, O0 typed LowIR. Loop 35 enters at `d705aafc`,
**173/177**, and completes **177/177** at `5452ad74`. All four original failures
are resolved without changing fixtures, references, coverage or comparison rules.
The previous goal turn was progress (committed initialization changes and verified
reports). This completes the implementation handoff; independent whole-stage
review remains pending. Preserve [the accumulated audit](audit.md) and both markers.

## Design and semantic ownership

| Owner | Data flow and completed behavior | Work / validation |
|---|---|---|
| Constant execution | Selected call/conversion -> checked body and lifetime facts -> immutable activation -> integral result. A frame includes body identity, canonical scalar arguments and typed receiver identity; function identity includes its specialization environment. Source-node constant caches never receive parameter-dependent results. | One checked body per function, one evaluation per complete activation/node key. Scalar/default/nested/recursive calls, function templates, stateless literal conversions, direct/functional casts, return/argument conversions and short circuiting tested. |
| Ordinary body validation | Class completion drains its own ordinary/explicit-class body interval. Checking is separate from the existing member-emission demand. Implicit class-template bodies remain lazy. Earlier semantic dump scheduling stays in its owning mode. | One check per required body, no global retries. Unused false assertions, bad returns/jumps and explicit classes reject; complete-class lookup and unused dependent templates succeed. |
| Static member storage | A defined namespace class object requests its own class's static constant storage through indexed definition edges. Later definitions satisfy the existing storage queue. Class completion does not instantiate unrelated nonvirtual bodies. | One class declaration walk per requested concrete class; per-member definition/storage requests deduplicated. Repeated objects, separate specializations and late definitions tested. |

Constant evaluation supports the fixture-required extension: C++11 single-return
integral functions and stateless literal temporaries with implicit construction.
Nonempty object execution, reference/pointer activation values and general PA16
constant evaluation keep their later-stage owner; none of the PA15 fixtures is
excluded. Overload resolution, access, deletion, conversion and construction are
checked by existing semantic owners. Execution consumes those facts, including
AST receiver records directly; it does not reconstruct calls through syntax or
names. Functional-cast prediction recognizes a braced object operand without
parsing a source region twice.

Execution is bounded by **512 active calls and 1,000,000 expression visits/root**.
Successful and expected-failure facts are memoized. Exhausted budgets and missing
constant definitions remain pending so a later shallower call or newly available
definition cannot inherit a false rejection. Completed independent values remain
reusable. Flat indexes and growing vectors are TU-owned and release with the
frontend. No new optimizer, textual phase transport, process-global cache or
per-node owning pointer is introduced. Compiler text grows by 16,000 bytes (1.06%).

Earlier completed groups remain intact: canonical value/pack arguments and
specializations, retained matching and aliases, complete-class initialization,
base/omitted zero plans, retained increment/decrement queries, and constexpr array
validation/storage. The measured **32-byte/object** O0 array-backing budget stays
in force. The prior [reference correction](reference-corrections.md), reducer,
standard proof, bundle revision and original/revised hashes are preserved.

## Remaining work and review boundary

**Required PA15 implementation: no known unfinished behavior group; 177/177.**
Independent audit must review the combined changes after `538cfcb0`, including
constant execution/cache availability, body checking versus emission, storage
edges and whole-stage architecture/performance. Passing implementation checks
neither resolves those review questions nor authorizes advancement. No review
marker is advanced and no requirement is waived.

## Validation and performance

Fresh required checks: `make test-pa15` **177/177**, prior-through-PA14
**1935/1935**, root-through-PA15 **2112/2112**, all exit 0. File audit passes with
three inherited header-ownership warnings. No PA15 native/debug gate is omitted:
its required LowIR validator is active; native controls use the supplied backend.
Root reports run sequentially to preserve their shared count sink.

Explicit personal controls: **24 native + 18 rejection** execution/body/storage;
26 value groups; 10 constant groups; 24 specialization native + 13 rejection;
27 pack native + 8 rejection; 15 matching native + 7 rejection; 23 initialization
native + 13 rejection + 3 LowIR; 11 checkpoint native + 7 rejection.

[Execution performance](execution-performance.md) retains all **224** frozen
A/A/ABBA and B-only observations, compiler latency/peak RSS, native runtime/text,
work scaling and the noisy dormant-body observation. Common LowIR and executable
bytes are identical. At 4,000 ordinary/template cases, paired compiler ratios
are 0.983–0.999 / 0.989–1.014. Checking 4,000 dormant bodies adds about 22.1 ms and
5,004 KiB; this is required validation work. Scalar evaluation has N activations,
5N visits and N memo hits; storage checks zero unrelated member bodies.
PA15/O0 has no mandated numeric latency/RSS/text ceiling and introduces no optional
transform requiring a speedup. Native optimization and self-hosting retain their
later owners. Preserve [initialization](initialization-performance.md),
[matching](matching-performance.md) and [audit](audit-performance.md) measurements;
inherited diagnostic targets do not override stage-scoped acceptance.

| Handoff | Implementation / independent review boundary | Evidence |
|---|---|---|
| Checkpoint 32 | Reviewed through `538cfcb0`; matching, constants/storage and ordinary validation unfinished | 166/177; prior 1935/1935; [audit](audit.md) |
| Loop 33 | Matching/parser and ordering/environment ownership; reviewed marker unchanged | 169/177; [matching handoff](../student.tests/pa15/matching-handoff.json) |
| Loop 34 | Initialization/query/array facts, reference correction and measured backing limit; four implementation failures remain | 173/177; prior 1935/1935; [initialization handoff](../student.tests/pa15/initialization-handoff.json) |
| Loop 35 | `5452ad74` checked constant execution and complete storage/body demand; full-stage independent audit pending | 177/177; prior 1935/1935; through 2112/2112; [verified handoff](../student.tests/pa15/execution-handoff.json) |
