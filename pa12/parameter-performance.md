# Parameter representation evidence

Final implementation: `f3e7ce93`. The checked PA12 suite advances from
**255/257 to 256/257**, with all 13 controls, all 66 personal sources, all three
explicit property scripts, earlier 1327/1327 and file audit passing. No fixture
or comparison rule changed.

## Protocol and frozen inputs

[Harness](../student.tests/pa12/parameter_benchmark.py) and
[all observations](../student.tests/pa12/parameter-performance.json) preserve
binary/source/backend hashes, flags, affinity, warmups, four A/A observations
and two ABBA blocks for every workload. Compilation uses `--emit-lowir -O0`;
validation and telemetry are checked separately from timings. The supplied PA8
backend runs at `-O0`. Dynamic 12-million-iteration loops have volatile bounds
and checked results/copy effects. Namespace counts provide scaling evidence.

Compiler size means `.text`; the supplied sectionless ELF size is its executable
payload after entry. RSS is `/usr/bin/time` peak KiB. Tables report medians from
the paired blocks. Short single-call native runs establish outcomes/size, not
speed; the full observations and paired ratios remain in JSON.

| Binary | Commit | SHA-256 | Compiler text bytes |
| --- | --- | --- | ---: |
| A | `eb0a4251` | `0fba5a741627fc8d1978a5991e56d3d38c95fafdc76bec24819f0f6beaecbd2f` | 969542 |
| B | `f3e7ce93` | `7c16e8acd813fdb09104ef01da69187fcbac0067ceefebe7dae33b54902abce7` | 972294 |

## Compiler and native measurements

| Compiler corpus | Seconds A / B | RSS A / B, KiB | Compiler B/A pairs | Native payload A / B, bytes |
| --- | ---: | ---: | --- | ---: |
| common-1000 | 0.25239 / 0.25284 | 49702 / 49680 | 0.999, 1.002 | 168056 / 168056 |
| common-4000 | 1.02728 / 1.02506 | 184572 / 184656 | 0.995, 1.047 | 672056 / 672056 |
| parameter-100 | 0.03254 / 0.03310 | 9878 / 9918 | 1.007, 1.045 | 14456 / 16856 |
| parameter-400 | 0.11600 / 0.11739 | 25050 / 25406 | 1.005, 1.019 | 57656 / 67256 |
| declaration-100 | 0.07358 / 0.07363 | 16692 / 16510 | 1.001, 1.003 | 3656 / 3656 |
| declaration-400 | 0.28573 / 0.28699 | 51598 / 52316 | 1.005, 1.005 | 14456 / 14456 |
| effect-400 | 0.15792 / 0.15731 | 31950 / 32010 | 0.994, 1.001 | 100068 / 100068 |
| pointer-800 | 0.07068 / 0.07144 | 15504 / 15500 | 1.022, 0.924 | 56068 / 56068 |

| Runtime corpus | Compiler seconds A / B | Compiler RSS A / B, KiB | Runtime seconds A / B | Runtime B/A pairs | Native payload A / B, bytes |
| --- | ---: | ---: | ---: | --- | ---: |
| common-runtime | 0.00611 / 0.00619 | 4956 / 5034 | 0.31026 / 0.30872 | 0.992, 0.999 | 323 / 323 |
| parameter-runtime | 0.00588 / 0.00590 | 4958 / 5016 | 0.24988 / 0.39759 | 1.586, 1.595 | 299 / 323 |
| declaration-runtime | 0.00633 / 0.00635 | 4944 / 5002 | 0.06402 / 0.06480 | 1.022, 1.002 | 191 / 191 |
| effect-runtime | 0.00617 / 0.00617 | 5014 / 5016 | 0.28721 / 0.28687 | 0.997, 1.000 | 417 / 417 |
| pointer-runtime | 0.00565 / 0.00562 | 4932 / 5030 | 0.06317 / 0.06327 | 0.996, 1.009 | 237 / 237 |

The large common compiler median is essentially unchanged (-.22%), with paired
ratios .995/1.047; this does not establish a repeatable compiler speedup. All
non-parameter native outputs are byte-identical, including declaration-only
queries, nontrivial-copy controls and typed pointer constants. Startup noise
and all outliers remain in the data.

The direct object-parameter workload changes **.24988 -> .39760 seconds**,
paired ratios **1.586/1.595**, and **299 -> 323** payload bytes. The 400-class
module grows by exactly 24 bytes per function. This is a disclosed **59% runtime
cost**, not an optimization benefit. The owning fixture requires the incoming
`obj<16x8>` payload to be copied into the callee parameter slot before its use.
A used indirect transport, while B emits that required object boundary and
`copyobj` materialization. The ordinary source-selected copy constructor still
executes once. Both closed programs produce identical checked source outcomes.

The PA12 course contract requires this ABI shape; representation costs do not
create an additional positive-runtime gate. Removing the parameter slot copy
would violate the required O0 boundary. Improving its native implementation
belongs to the later native backend/optimization stages; no source elision,
calling-convention workaround or backend delegation was introduced.

## Ownership, bounds and validation

A class owns one monotonic query state and selected-copy ID. Member facts retain
whether the body was defined in the class. An incremental entity cursor schedules
body queries alongside the existing deduplicated demand queue. No global retry
or source replay is added, and query-only bodies do not cause copy-helper emission
or an unrelated scalar-transfer proof. Lowering consumes the completed ABI fact.

The proof checks the empty in-class body, one full-storage trivial base, the
selected trivial base transfer, and a parenthesized source-parameter identity.
Unknown operations, constructor effects, source substitutions, volatile sources,
other move constructors, fields and comma-expression escapes remain conservative.
Language triviality and result ABI are unchanged. The explicit multi-TU property
checks declaration-only, caller and definition signatures, unused-helper omission,
callee slot materialization and linked execution. The personal reducer checks
observable copies and destination identity, including an initializer escape.

Work is linear in newly encountered entities, class declarations and inspected
source wrappers; each completed class is queried once. The declaration corpus
has 32 by-value declarations per class to exercise caching. The existing array
expansion cap remains eight. Diagnostic review budgets are one class state/ID,
one member flag, bounded local inspection, at most 6 KiB compiler text growth
and a 5% common compiler median cost; these are not new course gates. Actual
compiler text growth is **2752 bytes (.28%)**, with modest measured RSS costs.

```sh
python3 student.tests/pa12/parameter_benchmark.py /tmp/pa12-parameter-base-cppgm /tmp/pa12-parameter-final-cppgm /tmp/pa12-parameter-benchmark /tmp/parameter-repeat.json
python3 student.tests/pa12/check_parameter_representation.py
```

All campaign processes completed. Generated sources, LowIR and executables
remain scratch artifacts; the harness and all observations are committed.
