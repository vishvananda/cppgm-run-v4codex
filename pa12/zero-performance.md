# PA12 zero-initialization evidence

`b5645333` records zero-initialization by completed canonical type. Constructor
and value-list preludes demand the plan; lowering consumes its typed members,
padding, skipped references, null member-pointer representations and array
operations. A constructor defaulted after its first declaration retains its
user-provided initialization rule. Whole-storage zeroing is retained only when
the representation permits it; data-member pointers use the ABI all-ones null.

Plans and edges are translation-unit-owned, keyed by canonical type. Child array
plans provide flattened extents without rescanning nested dimensions. Local
array expansion is capped at eight total elements, including nested expansions;
larger arrays use one fixed-size loop. Padding expands at most eight stores,
then uses one bulk operation. No optional optimization pass was added.

## Frozen experiment

[Harness](../student.tests/pa12/zero_benchmark.py) and
[all observations](../student.tests/pa12/zero-performance.json) retain inputs,
hashes, flags, telemetry, warmups, four A/A observations and two ABBA blocks for
correct equivalent common/boundary programs. New null-member-pointer paths use a
warmup and six absolute observations because A initializes them incorrectly.
CPU 0 is pinned. Compiler `--emit-lowir -O0` latency and peak RSS are measured
separately from execution through the supplied PA8 `-O0` backend. Validation and
stats run outside compiler timing. Every measured executable validates and
returns its checked zero result. Runtime inputs use volatile bounds and checked
values. Common/boundary runs execute twelve million iterations; member-array
runs inspect 9.6, 10.8 and 16.384 million representation bytes for extents 8, 9
and 1024. Compiler scaling programs are not used for runtime speed claims.

A is `ce2d8363`, `/tmp/pa12-zero-base-cppgm`, SHA-256
`4717657d78309ec69aa89413c513d07d462d5f53eb6bee81273f3d0648e5af41`.
B is `b5645333`, `/tmp/pa12-zero-final-cppgm`, SHA-256
`606259d2a8d4d0a914b3b9df7b99930c94f45f13aa74929909f56a9848f84a6d`.
Compiler .text grows 6464 bytes, 959430 to 965894 (.67%). Scratch is
`/tmp/pa12-zero-evidence`; native text uses the earlier sectionless ELF payload
metric. All observations and outliers remain in the JSON.

| Workload / namespaces | Compiler median A/B seconds | Peak RSS A/B KiB | Paired B/A | Native text A/B |
| --- | ---: | ---: | --- | ---: |
| Common / 1000 | .25419/.25327 | 49784/49684 | 1.010/.990 | 168056/168056 |
| Common / 4000 | 1.02606/1.04635 | 184996/184916 | .974/1.023 | 672056/672056 |
| Boundary / 1000 | .20964/.21527 | 42156/43356 | 1.039/1.033 | 134068/144068 |
| Boundary / 4000 | .83426/.86241 | 153144/159024 | 1.033/1.077 | 536068/576068 |

Common LowIR and native bytes are identical. Large common compiler median cost
rises 1.98%, with paired noise disclosed. Typed volatile/base/union stores add
2.69%/3.37% compiler median cost and at most 3.84% peak RSS. Boundary A/A ranges
.83188–.83636 seconds at 4000 namespaces; its paired 1.077 outlier is retained.
The plans/parts scale from 2002/3000 to 8002/12000; emitted instructions are
37004/148004, linear in namespaces.

Common runtime medians are .30738/.30662 seconds, paired 1.004/.998, with 323
identical native bytes. Boundary runtime is .08396/.08725 seconds, paired
1.035/1.039, with 301/311 bytes. Runtime RSS is 256 KiB. The measured 3.92% boundary
runtime cost and ten-byte growth follow the required PA12 typed-store form;
there is no optional transform claiming a speed benefit. These required semantic
costs do not create a new positive-runtime exit gate under the stage-scoped spec.

| New null-array extent / namespaces | Compiler median seconds | Peak RSS KiB | Native text |
| --- | ---: | ---: | ---: |
| 8 / 100 | .03073 | 9320 | 21268 |
| 8 / 400 | .10712 | 24028 | 84868 |
| 9 / 100 | .03037 | 9236 | 22068 |
| 9 / 400 | .10410 | 22740 | 88068 |
| 1024 / 100 | .03050 | 9324 | 25468 |
| 1024 / 400 | .10417 | 22180 | 101668 |

Runtime medians for extents 8/9/1024 are .04824/.06425/.08661 seconds; native
text is 382/390/424 bytes and runtime RSS 256 KiB. These are absolute costs,
not comparable speed ratios: iteration counts and byte counts differ.
At 400 namespaces every extent has 1200 plans and 400 edges. Instructions are
20404 at eight elements and **18804 at both nine and 1024**. Larger constants
change native encoding size, while loop IR and compiler work remain bounded.
All mandated bounds, correctness and coverage remain unchanged.
