# PA12 destructor boundary and suffix evidence

`951799ed` separates destructor body effects from retained ABI entries and
parameter, private-temporary and array lifetime boundaries. Empty ordinary local
and subobject destruction can be omitted while required boundary calls remain.
Nontrivial empty synthesized destructors retain their ABI roots. Constructor
unwind handlers retain required subobject boundaries, and a nonthrowing
constructor no longer installs handlers that its boundary cannot use.

Subobject destruction now guards the remaining subobjects. Small O0 epilogues
retain the course form; inline suffix duplication is capped at eight actions
(at most 28 duplicated tail actions). Above eight, suffixes share one block per
remaining subobject. The normal path destroys each action once; the shared
handler chain and outer destructor handler are each linear. Local array
expansion retains its separate eight-total-element limit. These are explicit
work/output budgets, not fixture-size recognition or an unbounded optimization.

## Frozen experiment

[Harness](../student.tests/pa12/destruction_benchmark.py) and
[all observations](../student.tests/pa12/destruction-performance.json) retain
hashes, inputs, flags, telemetry, warmups, four A/A observations and two ABBA
blocks. CPU 0 is pinned; compiler wall time and peak RSS are measured separately
from native execution. Compilation uses `--emit-lowir -O0`; validation and stats
are outside timing. The supplied PA8 native backend uses `-O0`. Every A/B
executable validates and exits 0, checking dynamic field values, live counts and
all destructor calls. Namespace counts measure compiler scaling. Runtime loops
use volatile bounds and twelve million total leaf lifetimes (rounded down when
necessary), so they do not time dead or constant-folded work.

A is `8bf45f86`, `/tmp/pa12-cleanup-final-cppgm`, SHA-256
`dfd4607a76a31d09da89ae3f5daa4f2bd899b7148bcfc1b16cfff64e54408e55`.
B is `951799ed`, `/tmp/pa12-destruction-final-cppgm`, SHA-256
`dda7927f664ea60214320d6a2135d1edbab5c9000f4403ef811aa9bdd3b35f4e`.
Compiler text grows 1536 bytes, 948934 to 950470 (.16%). Scratch is
`/tmp/pa12-destruction-evidence`. Native text uses the same sectionless payload
metric as earlier campaigns. Both binaries correctly execute these nonthrowing
runs; only B supplies the additional subobject unwind suffixes.

| Fields / namespaces | Compiler median A/B seconds | Peak RSS A/B KiB | Paired B/A | Native text A/B |
| --- | ---: | ---: | --- | ---: |
| 1 / 100 | .03739/.03657 | 10588/10252 | .964/.962 | 64992/47992 |
| 1 / 400 | .13231/.13210 | 26524/26248 | .989/.992 | 258192/190192 |
| 8 / 100 | .06185/.06540 | 15912/16892 | 1.092/.927 | 221992/238992 |
| 8 / 400 | .22384/.24493 | 45028/49224 | 1.097/.769 | 886192/954192 |
| 9 / 100 | .06457/.06307 | 16200/16120 | 1.010/.974 | 244392/209392 |
| 9 / 400 | .23906/.24103 | 49856/50052 | .994/1.006 | 975792/835792 |
| 32 / 100 | .13322/.13453 | 30508/30544 | 1.006/1.018 | 760992/661592 |
| 32 / 400 | .51612/.52752 | 99516/100024 | 1.013/.697 | 3042192/2644592 |

Eight-field compiler median cost rises 9.4%, peak RSS 9.3%, and native namespace
text 7.7%. That small bounded suffix form satisfies the PA12 O0 epilogue
contract. At nine fields sharing reduces instructions despite the additional
unwind semantics; 32-field compiler median cost is 2.2%. Large A timing outliers
remain, including paired ratios .769 and .697; these do not establish a compiler
speed gain. The 32/400 A/A range is .50999-.52501 seconds.

| Runtime fields | Median A/B seconds | Paired B/A | Native text A/B |
| --- | ---: | --- | ---: |
| 1 | .30530/.24491 | .802/.822 | 1336/1160 |
| 8 | .18770/.13562 | .717/.720 | 2904/3080 |
| 9 | .18556/.13999 | .729/.765 | 3128/2784 |
| 32 | .18175/.12730 | .693/.487 | 8296/7304 |

Runtime peak RSS is 256 KiB. Removing redundant constructor handlers has a
repeatable runtime benefit in these workloads: approximately 20%, 28%, 25% and
30% by the medians. The last paired outlier is retained; no 51% benefit is
claimed from it. Eight-field text grows 176 bytes (6.1%) for the required suffix
form while the runtime improves 28%. Other runtime text sizes shrink.
The proof consults the already recorded nonthrowing constructor fact; it adds
constant work per action and no new body analysis. The bounded code growth and
measured runtime improvement justify retaining this refinement.

For 100/400 namespaces, B instruction counts are 6804/27204 (one field),
26404/105604 (eight), 21404/85604 (nine), and 62804/251204 (32). Beyond the
inline threshold, increasing nine to 32 fields adds exactly 18 instructions per
field per namespace; the suffix representation scales linearly. No input,
fixture, reference, comparison rule or mandated limit was weakened.
[Final common and cleanup evidence](cleanup-performance.md) discloses the
remaining required branch/condition representation costs across the full group.
