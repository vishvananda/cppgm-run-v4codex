# PA12 member and constructor evidence

The measurements cover implementation through `55d15ba8`. The stage-base and
review markers stay in [plan.md](plan.md). This is an O0 source-to-LowIR stage;
the PA8 supplied backend executes the resulting programs. No PA24 optimization
or native-emission gate is imposed.

## Reproduction and frozen observations

- [Harness](../student.tests/pa12/member_benchmark.py): four A observations for
  A/A calibration followed by two ABBA blocks, pinned to one available CPU.
  `/usr/bin/time` records peak RSS; a monotonic wall clock includes process
  creation. All rows, hashes, paths and flags are retained in the JSON files.
- A: `/tmp/pa12-stage-base-cppgm`, SHA256 `fe662c96e7ed834276169929d4b99e4076331e048867f0c3ceeefb9ea0367280`.
- Initial B: `/tmp/pa12-member-cppgm`, SHA256 `a713d6e52938ef95c21eb34c88bcda8093209108cdd644ecc40fc9cd8ea5dd05`.
  [Initial observations](../student.tests/pa12/member-performance.json) and
  `/tmp/pa12-member-evidence/` retain the campaign and frozen harness. Subsequent
  failure-set comparison found two using-declaration ranking regressions outside
  these benchmark inputs; both were corrected. These observations remain historical.
- Corrected B: `/tmp/pa12-member-corrected-cppgm`, SHA256 `9959f526bcc714b21f5491dbedbe90465460d149a70ad9f42123881bf688654c`.
  [Corrected observations](../student.tests/pa12/member-corrected-performance.json)
  use `/tmp/pa12-member-corrected-evidence/`.
- Compiler build: default `g++ -std=gnu++11 -Wall -O3`, course runner enabled.
  Source flags: `--emit-lowir -O0`; native backend: `-O0`. Validation and telemetry
  are separate untimed invocations. Backend and input hashes are in the records.
- Common member inputs produce byte-identical LowIR and checked native exit 0
  with both binaries. The empty-destructor input performs 12 million iterations
  behind a volatile bound and checks a modular checksum; both executables exit 0.
  The loop's destructor is observably empty, so omitting its calls preserves the
  computation. Correctness is not inferred from timing or IR size.

## Corrected results

Compiler timings below are medians of the four ABBA observations per binary;
RSS is the largest timed observation. Runtime/text are reported alongside them.

| Workload | Compiler A / B seconds | RSS A / B KiB | Native A / B seconds | Native text A / B bytes |
| --- | --- | --- | --- | --- |
| 1000 member classes | 0.12402 / 0.12541 | 27204 / 27420 | 0.00306 / 0.00304 | 70053 / 70053 |
| 4000 member classes | 0.50230 / 0.50336 | 95244 / 96372 | 0.00306 / 0.00300 | 280053 / 280053 |
| Empty-destructor loop | 0.00553 / 0.00555 | 4788 / 4796 | 0.17787 / 0.08463 | 928 / 241 |

Compiler text grows from 785798 to 795014 bytes (+9216, 1.17%). Startup medians
are 0.00568/0.00562 seconds. Common compiler workloads dominate startup; their
native executions and the tiny loop's compilation do not, so those small timings
support no speed claim.

Paired compiler B/A ratios are 1.0330/1.0149 (1000 classes) and 1.0066/0.9393
(4000). A/A ranges are 0.12499–0.12616 and 0.50428–0.50993 seconds. The second
4000-class block includes a slow A observation (0.57182 seconds); it is retained
and does not establish a compiler speedup. Corrected compiler medians increase
1.12% and 0.21%, with RSS increases of 216 and 1128 KiB. This small cost accompanies
required identity/selection checks; no avoidable large regression is evidenced.

The loop's paired runtime ratios are 0.4738/0.4758. A/A is 0.17453–0.17788 seconds;
ABBA A spans 0.17627–0.18075 and B spans 0.08378–0.08538. The initial campaign
independently measured ratios 0.4771/0.4882. The approximately 52% runtime decrease
and 74% text decrease are repeatable on this affected workload. They are not a
claim about unrelated programs. Common outputs and native text are unchanged.

## New semantic costs and work budgets

The [feature harness](../student.tests/pa12/feature_benchmark.py) measures new
ref-qualified/delegating classes independently because the stage base rejects
them; a rejecting compiler is not an equivalent-correct performance baseline.
[All observations](../student.tests/pa12/feature-performance.json), inputs and
native artifacts are retained under `/tmp/pa12-feature-evidence/`.

1000/4000 classes compile in median 0.17767/0.71845 seconds with timed peak RSS
36000/130416 KiB (separate telemetry: 36092/130464). Native text is 148048/592048
bytes; all executions return 0. Their startup-sized runtimes are recorded without
a performance claim. At 4x input, time grows 4.04x and peak RSS 3.62x. Constructor
actions grow 2000 to 8000, candidates 10001 to 40001, and member demands/processed
facts 5000/5000 to 20000/20000. The maximum pending token count stays 48.

Explicit O0 work bounds: canonical ref qualifiers add no graph copy; candidate
checks visit only the object/base/using path. Delegation colors each demanded
vertex and edge once. Empty-destructor decisions inspect an empty body and cache
variant/subobject summaries by class identity for the TU. They demand no unused
variant body. There is no fixed-point search, body cloning or speculative inlining;
removing empty calls has a zero generated-code-growth budget. The shared eight
array-element expansion limit remains intact. All other lowering stays conservative.

These are stage-scoped observations and structural budgets, not newly invented
numeric exit gates. Existing PA10/PA11 benchmark inputs and measurements remain
available for template, loop, call, memory and floating-point coverage. Correctness,
coverage, normalized comparison rules and mandated limits are unchanged.

The transfer continuation, including a removed unprofitable optional fold and
all final measurements, is recorded in [transfer-performance.md](transfer-performance.md).
