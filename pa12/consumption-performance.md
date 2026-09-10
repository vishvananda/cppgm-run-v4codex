# PA12 terminal consumption and return cleanup evidence

`260e0b35` carries the existing typed return destination into conditional
consumption. Branch-local temporary suffixes finish before merging once their
value reaches that destination; a conditional still feeding an enclosing call
or transfer retains its selector and private lifetime. Return cleanup is keyed
by the function and live prefix, independently of intervening loops.
`14dac876` records scalar-only user transfer bodies after member demand completes.
A terminal branch copying a known automatic glvalue can use that no-unwind proof;
unknown calls and class subobjects remain conservative. Branch construction
materializes storage before opening its constructor guard.

No language exception specification or public ABI is strengthened. The proof
visits each examined typed node once, using a lazy byte per AST node and one
member flag. It rejects unrecognized operations and follows no call graph.
Terminal consumption follows existing conversion edges, and cleanup suffixes
remain indexed by prefix/terminal. Nested selectors and all array/suffix growth
limits are unchanged. Personal properties check both proof acceptance and the
unknown-call fallback, plus nested lifetimes; all source programs execute.

## Frozen experiment

[Harness](../student.tests/pa12/consumption_benchmark.py) and
[all primary observations](../student.tests/pa12/consumption-performance.json)
retain hashes, inputs, flags, work counts, warmups, four A/A observations and two
ABBA blocks for every workload. CPU 0 is pinned. Compiler wall time and peak RSS
are separate from execution through the supplied PA8 `-O0` backend; validation
and stats are outside compiler timing. Every A/B program validates and returns
its checked zero result. Runtime loops use volatile bounds, twelve million
iterations, dynamic values and checked live/destructor counts.

A is `b1e936e8`, `/tmp/pa12-consumption-base-cppgm`, SHA-256
`606259d2a8d4d0a914b3b9df7b99930c94f45f13aa74929909f56a9848f84a6d`.
B is `14dac876`, `/tmp/pa12-consumption-final-cppgm`, SHA-256
`b3946580a85465de1e7ba4e012f6fff369a988c468f924c64c7c6323d5f373d3`.
Compiler .text grows 1728 bytes, 965894 to 967622 (.18%). Native sizes below use
the supplied sectionless ELF's executable payload after entry, including its
small static data, consistently with the earlier campaign metric.

| Workload / namespaces | Compiler median A/B s | Peak RSS A/B KiB | Paired B/A | Native payload A/B bytes |
| --- | ---: | ---: | --- | ---: |
| Common / 1000 | .25377/.25319 | 49704/49688 | .995/.996 | 168056/168056 |
| Common / 4000 | 1.03873/1.05627 | 185008/184908 | 1.047/1.013 | 672056/672056 |
| Terminal / 100 | .05557/.05518 | 14004/13968 | .992/.999 | 127992/112592 |
| Terminal / 400 | .20715/.20454 | 41792/39764 | .986/.989 | 510192/448592 |
| Unknown call / 100 | .05868/.05873 | 13972/14036 | .981/.991 | 133992/130392 |
| Unknown call / 400 | .22233/.21796 | 43708/41660 | .984/.767 | 534192/519792 |
| Loop return / 100 | .02968/.02997 | 9264/9180 | 1.010/1.012 | 29468/28668 |
| Loop return / 400 | .10204/.10242 | 22100/22108 | .999/1.018 | 117668/114468 |

Common LowIR/native bytes are identical; the large compiler median rises 1.69%
within the disclosed VM timing spread (A/A 1.02737–1.06763 s). Terminal and
unknown-call large compiler medians fall 1.26%/1.97%, with peak RSS down 4.85%/
4.69%. The .767 paired outlier is retained and is not a claimed speedup.
Loop compiler median cost rises .37%; its shared slot/cleanup form is part of
this PA12 O0 contract, not an additional optimizing pass.

| Runtime | Median A/B s | Paired B/A | Native payload A/B bytes |
| --- | ---: | --- | ---: |
| Common | .31078/.31019 | .998/.971 | 323/323 |
| Terminal | .42726/.42299 | .989/.989 | 1960/1808 |
| Unknown call | .44456/.43819 | 1.049/.987 | 2024/1984 |
| Loop return, original native layout | .18852/5.18367 | 25.912/26.421 | 464/456 |

Runtime RSS is 256 KiB. Terminal consumption removes 152 native bytes and shows
a small approximately 1% improvement in both paired blocks. That effect is
modest relative to VM noise (A/A .42511–.44685 s); no broader speed claim is made.
The unknown-call runtime has no repeatable winning direction. The very large
loop regression remains in the primary evidence and was investigated below.

At 100/400 namespaces, terminal proof work is 2300/9200 and instruction count
12804/51204; the unknown-call case is 1800/7200 proof visits and 13804/55204
instructions. Cache bytes equal source nodes: 33321/133221 and 35821/143221.
Common and loop workloads allocate no proof cache. Work and output scale
linearly, with no new fixed-point search or unbounded expansion.

## Native code/data placement diagnosis and stage acceptance

The supplied native writer places this reducer's one writable counter directly
after code. Its eight-byte code reduction moves the counter from `0x400244` to
`0x40023c`. The latter shares the 64-byte instruction-cache line beginning at
`0x400200` with the hot loop tail, including the branch at `0x400210`.
`getconf LEVEL1_ICACHE_LINESIZE` reports 64 on this Intel Xeon/KVM host.

The [diagnostic harness](../student.tests/pa12/return_layout_benchmark.py) moves
only that counter 64 bytes forward in frozen native images. It asserts a
single four-byte global and exactly four references, changes their addresses
and ELF segment sizes, and verifies that every instruction position and all
other instruction bytes remain identical. It adds data padding after all code.
This is a diagnostic artifact, not a compiler or reference change. All modified
images execute the same checked program. No production padding workaround was
introduced. [All observations and fixups](../student.tests/pa12/return-layout-performance.json)
and [exploratory source variants](../student.tests/pa12/return-layout-exploration.json)
are retained. The requested `machine_clears.smc` PMU event was unavailable;
hardware counters are not an exit requirement.

Original/relocated medians are .18873/.18830 s for A and **4.85777/.19147 s for B**.
Both relocation ABBA ratios for B are about .040; A ratios are .998/.999.
This establishes that the large slowdown depends on native code/data placement,
consistent with repeated writes to a hot executable cache line. The original
and relocated B images have 452 unchanged code bytes apart from data addresses;
its payload grows from 456 to 520 bytes solely because of the diagnostic padding.

A separate A/A+ABBA comparison of the two relocated images has medians
.35339/.36582 s and paired B/A 1.113/.953 on a noisier interval. It does not show
a stable large algorithmic regression or a loop speed benefit. Both the original
cost and the noisy follow-up remain disclosed; the follow-up does not replace
primary results. `3789b6a3` preserves the original diagnostic harness/results,
and the JSON records the continuation harness hash.

Code/data separation belongs to native layout in PA24; PA12 is required to emit
its own LowIR and use the supplied backend for these behavioral controls. Its
scalar-global contract carries scalar data alignment, not a native cache-line
placement request. Inflating O0 LowIR to shift one reference backend image would
be a fragile workaround, not a general fix. The stage-scoped spec therefore
accepts this demonstrated later-backend constraint without a new PA12 exit gate.
The required shared return representation and conservative unknown-call
boundaries remain, all measurements are preserved, and no correctness, coverage,
comparison rule or mandated growth limit has been weakened.
