# Scalar initialization and cleanup evidence

Final implementation: `67947601`, following `7e88ba59`. PA12 reaches
**257/257**, including all 13 controls; the full through-stage report passes
**1584/1584**. All 67 personal sources and three explicit property scripts pass.
No fixture, reference, comparison rule or coverage changed in this group.

## Protocol and frozen inputs

[Harness](../student.tests/pa12/scalar_benchmark.py),
[initial broad policy observations](../student.tests/pa12/scalar-broad-performance.json)
and [final observations](../student.tests/pa12/scalar-performance.json) preserve
all hashes, inputs, telemetry, warmups, four A/A observations and two ABBA blocks
per compiler and runtime workload. Both campaigns have 15 workloads. Compilation
uses `--emit-lowir -O0`; validation and telemetry run separately from timing.
The supplied PA8 backend uses `-O0`. Twelve-million-iteration runtime loops have
volatile bounds, varying inputs and checked sums, destructor counts and observed
initialized values. State belongs to caller stack objects.

Compiler size is `.text`; native size is the supplied sectionless ELF executable
payload after entry, including its runtime support/data. RSS is `/usr/bin/time`
peak KiB. Tables use medians from paired blocks. Small single-call runs check
outcomes and size; their startup-dominated timings make no speed claim. JSON
retains every observation, A/A range and pair, including outliers.

| Binary | Commit | SHA-256 | Compiler text bytes |
| --- | --- | --- | ---: |
| A | `f3e7ce93` / `51fbedef` code | `7c16e8acd813fdb09104ef01da69187fcbac0067ceefebe7dae33b54902abce7` | 972294 |
| Broad B | `7e88ba59` | `65b2696ec1bea2c4c25430ab12fd37c47dcdf69e33203438b24b75b1ea910148` | 980230 |
| Final B | `67947601` | `7f19ddb9099151715f3a967c6fae5cb9139d9bfbca9d7a6308e9fd08fcc34ed8` | 980166 |

## Final compiler and native measurements

| Compiler corpus | Seconds A / B | RSS A / B, KiB | Compiler B/A pairs | Native payload A / B, bytes |
| --- | ---: | ---: | --- | ---: |
| common-1000 | 0.25040 / 0.25011 | 49886 / 49358 | 0.999 / 0.989 | 168056 / 168056 |
| common-4000 | 1.05052 / 1.04027 | 184966 / 184794 | 0.975 / 0.996 | 672056 / 672056 |
| dynamic-100 | 0.04965 / 0.04954 | 13218 / 13444 | 1.005 / 0.999 | 94992 / 94992 |
| dynamic-400 | 0.18604 / 0.18429 | 39344 / 38676 | 0.990 / 0.993 | 378192 / 378192 |
| constant-100 | 0.04897 / 0.04775 | 13138 / 12634 | 0.969 / 0.983 | 91992 / 74392 |
| constant-400 | 0.18295 / 0.17696 | 38218 / 36018 | 0.964 / 0.972 | 366192 / 295792 |
| mutated-100 | 0.05501 / 0.05542 | 13364 / 13874 | 1.018 / 0.876 | 98392 / 98392 |
| mutated-400 | 0.21135 / 0.20818 | 41372 / 39374 | 0.981 / 0.997 | 391792 / 391792 |
| observed-100 | 0.06150 / 0.06066 | 15462 / 14688 | 0.986 / 0.985 | 106192 / 106192 |
| observed-400 | 0.23182 / 0.23271 | 47346 / 47530 | 0.855 / 0.995 | 422992 / 422992 |

| Runtime corpus | Compiler seconds A / B | Compiler RSS A / B, KiB | Runtime seconds A / B | Runtime B/A pairs | Native payload A / B, bytes |
| --- | ---: | ---: | ---: | --- | ---: |
| common-runtime | 0.00620 / 0.00635 | 5014 / 4992 | 0.30841 / 0.30670 | 0.995 / 0.997 | 323 / 323 |
| dynamic-runtime | 0.00661 / 0.00647 | 5018 / 5036 | 0.32202 / 0.32366 | 1.002 / 1.006 | 1640 / 1640 |
| constant-runtime | 0.00602 / 0.00604 | 4958 / 4990 | 0.30929 / 0.34680 | 1.124 / 1.118 | 1600 / 1424 |
| mutated-runtime | 0.00621 / 0.00625 | 4990 / 5008 | 0.34500 / 0.34502 | 1.000 / 0.994 | 1672 / 1672 |
| observed-runtime | 0.00634 / 0.00633 | 5032 / 4978 | 0.35986 / 0.35700 | 1.038 / 0.984 | 1752 / 1752 |

Final common compiler medians change -.11% / -.98% at 1000 / 4000 namespaces;
these small changes do not establish a substantial compiler speedup. All
nonconstant workloads produce **byte-identical LowIR and native output** under
A and final B. Their mixed runtime pairs are timing noise, not code improvements.
The first broad campaign's common-1000 median rose 14.3%; its A baseline also
rose from its .251–.256 second A/A range to a .360 second paired median. All of
those observations remain; the final campaign measures .25040 / .25011 seconds.

## Measured policy correction and retained contract cost

The initial broad policy moved dynamic/modified-condition cleanup into the
selected branch. Native payloads shrank, but runtime regressed:

| Initial runtime | Seconds A / broad B | B/A pairs | Payload A / broad B |
| --- | ---: | --- | ---: |
| dynamic | .32401 / .35084 | 1.081 / 1.087 | 1640 / 1600 |
| modified through reference | .34124 / .41040 | 1.199 / 1.198 | 1672 / 1632 |
| observed destination | .35566 / .37044 | 1.039 / 1.049 | 1752 / 1696 |

That optional extension is removed. Unknown, modified and volatile conditions
retain the existing shared cleanup path. Final native byte identity verifies
that the extra runtime cost was removed without a source, fixture or backend
workaround. The compiler still records normal writes and exposure once, because
they invalidate the private constant proof used by the required path.

The constant-condition path retains a measured **12.1% runtime cost**:
.30929 -> .34680 seconds, paired ratios **1.124 / 1.118**. Its payload shrinks
1600 -> 1424 bytes; the 400-namespace compiler median falls 3.27% and peak-RSS
median falls 38218 -> 36018 KiB. These compile/size reductions do not establish
runtime profitability. No runtime improvement is claimed for this path.

The owning `500-direct-class-call-temporary-destination` contract requires the
known branch's scalar result store and normal destructor before its region
ends, and direct class-call materialization before the initial resource guard.
It retains the source condition/calls while omitting cleanup on the proved
inactive arm. B implements these O0 ordering and materialization facts; A uses
a shared selector and later destructor. Both benchmark executables have the
same checked C++ outcomes, but A lacks the required LowIR shape. Removing the
new ordering would restore the failing course fixture. The remaining measured
cost is therefore a required PA12 output cost with the supplied backend, not
an optional optimization or an added positive-runtime exit gate. Its native
microarchitectural cause is not isolated here; no backend bug, padding remedy
or speculative speed benefit is asserted.

## Ownership, bounds and validation

Ordinary semantic modification, address-taking and direct reference conversion
record observation by canonical object ID. Unevaluated operations do not mark
an object. After function ABI completion, scalar initializers query a literal or
already-proved integral constant, or an unchanged unexposed local integral
initializer, converted to its declared type. Volatile, floating, modified,
aliased and unknown conditions retain conservative lowering.

A consumer records the existing final conversion, declared destination type,
private-storage fact and condition truth. Lowering performs the final scalar
conversion before cleanup; an observed destination also receives its store
before any destructor can read it. Private storage may receive the merged scalar
after branch cleanup. Enclosing temporaries retain their existing lifetime
state. Only proved inactive branch actions are omitted; source calls and the
condition remain in O0 output.

The translation unit owns at most one 16-byte consumer per selected initializer,
one sparse observation entry per modified/exposed scalar, and sparse ID indexes.
An existing entity traversal collects automatic scalar initializers; each
candidate's source wrappers and initializer are inspected at most once, with
constant work per use. There is no graph clone, source replay, global retry,
call-graph search or new per-node cache. Work remains linear. The 400-namespace
constant corpus records 400 consumers, 400 observations and 2800 inspection
steps, versus 100 / 100 / 700 at the small scale. The 4000-namespace common
corpus records zero consumers, observations and inspection steps. Dynamic,
modified and observed 400-namespace corpora record zero consumers.

The premeasurement diagnostic review budgets were 8 KiB compiler text growth
and 5% common compiler median cost, plus the structural bounds above. Final text
growth is **7872 bytes (.81%)** and the final common medians remain below the
review threshold. These targets are not added course gates. The mandated array
expansion cap remains eight. All prior measurements and necessary ABI/backend
cost classifications in the other PA12 reports remain in force.

Validation uses the full course report, file audit, all personal sources and
`check_parameter_representation.py`, `check_terminal_returns.py` and
`check_zero_initialization.py`. The scalar reducer exercises both dynamic arms,
both known arms, writes/reference aliases/calls/conditional lvalues, volatile
conditions, integral narrowing, unevaluated modification and pointer/reference
exposure of the destination. Frozen source/compiler/native hashes and complete
observation order were checked for both campaigns; current `dev/cppgm++` matches
final B. All processes completed.

```sh
python3 student.tests/pa12/scalar_benchmark.py /tmp/pa12-scalar-base-cppgm /tmp/pa12-scalar-policy-cppgm /tmp/pa12-scalar-repeat /tmp/scalar-repeat.json
python3 student.tests/pa12/check.py
make test-pa12
make test-report-through-pa12
perl scripts/cppgm_file_audit.pl --stage pa12 --paths dev/src
```
