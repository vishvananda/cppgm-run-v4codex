# PA11 constructor checkpoint evidence

Implementation: `1789701403c7b5e52eb29ce1f2f761dea9a36a63`. Stage base and last-reviewed markers remain unchanged in [plan.md](plan.md). PA11 remains incomplete.

## Correctness and ownership

- `make test-pa11`: **156/302**, versus 43/302 at entry; 146 failures remain. The full fixture-failure set comparison finds 113 original failures fixed and no newly failing original fixture. Required coverage and comparison rules are unchanged.
- `make test-report-through-pa10`: **1025/1025**. `perl scripts/cppgm_file_audit.pl --stage pa11 --paths dev/src`: exit 0; the Analyzer header advisory counts declarations as body lines. Implementations reside in their registered .cpp owners.
- `python3 student.tests/pa11/check.py`: two programs pass typed in-memory LowIR validation and native execution: inherited fields, this, cv overloads, reference-returning methods/static calls; ordered base/default-member construction, constructor defaults, reference binding and namespace startup.
- No reference output, fixture, oracle, runner, coverage rule or attribution was changed. The supplied backend only executes student-generated LowIR in personal validation/performance measurements.

`ClassFacts`, canonical constructor entities and `MemberFacts` own layout, overload selection and helper demand. Constructor member initializers are resolved after parameter scopes exist; actions store selected entities/types and source NodeIds. Lowering consumes those records and typed field/base projections. No fake calls, AST copies, host compilation or textual phase transport are introduced. Source views distinguish direct source demands from implicit subobject work. Namespace initialization has one typed internal function and preserves declaration order.

Each queued member body is analyzed once. Synthetic-helper triviality is memoized after semantic demand closes. Layout visits each actual field once and excludes using-declaration views. Candidate work is linear in required candidates/arguments; action lowering is linear in produced instructions, including explicit destination projections. Rare member-object facts occupy a TU-owned arena indexed by one compact expression field. Temporary initializer paths are function-local vectors, released after lowering.

## Frozen common-subset comparison

See [protocol](../student.tests/pa11/performance-protocol.md), [all common observations](../student.tests/pa11/common-performance.json), [template repeat](../student.tests/pa11/template-repeat-performance.json) and [constructor observations](../student.tests/pa11/constructor-performance.json). All include hashes; frozen binaries and generated inputs remain under `$RALPH_ARTIFACT_DIR/pa11-performance`. No builds/tests ran during timing. The PA10 harness supplies AAAA calibration followed by two ABBA blocks, affinity, external wall/RSS and separate phase telemetry. All nine common output pairs are byte-identical.

| Compiler input | A/B median seconds | A/B peak RSS KiB | Paired B/A ratios | A/A spread |
|---|---:|---:|---|---:|
| calls-1 | 0.38540/0.39144 | 71844/73388 | 1.0116, 0.9708 | 1.74% |
| memory-float-1 | 0.33639/0.33810 | 62064/67100 | 0.8379, 0.9992 | 1.51% |
| references-1 | 0.01566/0.01597 | 6456/6452 | 1.0178, 1.0082 | 1.17% |
| template-semantics-1 | 0.06542/0.06688 | 12160/12292 | 1.0199, 1.0250 | 2.61% |
| calls-4 | 1.57112/1.58384 | 276624/279292 | 1.0004, 0.9795 | 0.87% |
| memory-float-4 | 1.35410/1.34790 | 235604/241204 | 1.0032, 0.9507 | 1.24% |
| references-4 | 0.04786/0.04899 | 12636/12820 | 1.0181, 1.0181 | 3.88% |
| template-semantics-4 | 0.25329/0.25818 | 35548/36584 | 1.0074, 1.5747 | 3.00% |
| references-8000 | 0.11185/0.11471 | 23956/24428 | 1.0229, 1.0283 | 5.20% |

The template-4 timing spike is retained. Repeating only that input with the same frozen binaries gives paired ratios 1.0143/1.0090, A/A spread 1.75%; the spike is not repeatable in this follow-up. No outlier was dropped.

Compiler text grows 564230 → 597894 bytes (+33664, 5.97%) for required object-model behavior. Common-subset peak RSS increases by at most 5600 KiB. Most paired changes are within calibration variability; short references/templates are diagnostic only. The second common template block exceeds the inherited 1.10x diagnostic but the follow-up does not reproduce it. There is no additional numeric PA11 gate; mandated correctness, complexity and coverage remain. No optional optimization or runtime benefit is claimed.

| Executable | A/B median seconds | A/B text proxy bytes | Paired B/A ratios |
|---|---:|---:|---|
| calls-long | 0.47858/0.47936 | 206/206 | 0.9948, 1.0044 |
| memory-long | 0.28008/0.28027 | 434/434 | 0.9984, 1.0000 |
| floating-long | 0.33231/0.33035 | 230/230 | 0.9989, 0.9889 |

All three executable pairs are byte-identical and return 0. Volatile trip counts and checked checksums retain observable runtime work. Backend compilation is excluded from execution timing. The native text proxy is the executable payload after entry in the supplied sectionless ELF, on inputs without static data; it is not a .text section measurement.

## Newly implemented constructors

The stage base is not a correct constructor implementation, so these are A/A measurements of the correct candidate, not performance ratios against missing behavior.

| Class-pair count | Median seconds | Peak RSS KiB | A/A spread | Object facts / constructor actions / processed demands | IR instructions |
|---|---:|---:|---:|---|---:|
| constructors-1000 | 0.19706 | 40572 | 1.81% | 4000 / 3000 / 3000 | 34003 |
| constructors-4000 | 0.79374 | 149308 | 0.78% | 16000 / 12000 / 12000 | 136003 |

Fourfold source growth yields 4.03x latency and 3.68x peak RSS. Object facts, initialization actions and processed demands grow exactly fourfold; candidate work grows 3001 → 12001. Both generated programs validate and return 0. The new 96-million-iteration constructor/member-call runtime has median 0.64312 s, 235-byte text proxy, peak RSS 256 KiB and four checked exit-0 observations. These are necessary semantic costs, not optimization gains.

## Handoff boundary

The initial member-address work was extended through constructor selection, default/member/base ordering, aggregate destination paths, namespace initialization, compact object-use ownership and measured demand/scaling. This finishes that scalar/reference and single-base construction group with executable evidence. Further class-array work cannot reuse scalar initialization alone: it must establish partial-construction/destruction state and equal-suffix cleanup ownership across loops, goto, returns and global/TLS teardown. Adding more constructor calls before that shared lifetime model would preserve incorrect observable cleanup. That is the concrete architectural boundary for this incomplete checkpoint; the full-stage objective remains active.

Remaining groups: destructor and shared lexical cleanup (including helper inline policy); array/aggregate brace elision and union/volatile/zeroinit rules; access/friends/ADL/operators; bit-fields and stronger alignment; TLS wrappers, inherited constructors, and remaining metadata/derived conversion shapes. The current-stage suite still fails and no later PA is being advanced.
