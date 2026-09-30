# PA27 performance147

Implementation: `79dffd03b26122773f10e9b45154b2ecea711b3a`; entry: `8d595af6b91289109cae36554a8e8e8551192873`.
Final compiler SHA-256: `a2abb98ced2fc821db02b80d0661bf59797d0d2851847ef4497366e30bdf524e`.
Entry compiler SHA-256: `37d90792b7d692fe04d29077a46540bb7751f79b4037d095b6d593cd66512955`.

## Protocol and observations

Final measurements use frozen binaries, `-O0 -c --stats`, fixed generated inputs and host `g++` final linking. Each compiler/executable comparison has one four-run A/A calibration and six ABBA blocks, measured separately with wall time and `/usr/bin/time` peak RSS. Final measurements pin CPU 0. Every execution checks an independently calculated result, with `argc` supplying a runtime loop bound. Inputs, compiler/object/executable hashes, phase counters and every sample are in the linked JSON; sources are reproducible from the committed scripts.

[Common](evidence147/common-performance.json) has 224 final observations; [construction](evidence147/construction-performance.json) and [constant construction](evidence147/constant-performance.json) each have 112: **448 final observations**. The four common inputs retain their inherited fixed source hashes and cover templates, loops, calls, memory, floating point, exceptions and local-function pruning. Self-hosting remains PA34 work; no PA27 performance claim is made for it.

Preserved earlier observations: [initial common](evidence147/initial-common.json), [initial construction](evidence147/initial-construction.json), [scoped common](evidence147/scoped-common.json), [scoped construction](evidence147/scoped-construction.json), and [short constant run](evidence147/short-constant-performance.json). Together all files retain **1232 observations**. The initial run was unpinned; the second run preceded flat constant-buffer pooling; the short constant workload had 1024 objects. The final constant workload uses 8192 objects to increase compilation duration. These historical data are not substituted for final-binary evidence.

## All four dimensions

Values show before/after medians, peak compiler RSS and executable `.text` bytes. Ratios are medians of the six paired block ratios, not ratios of unpaired global medians. Every sample/spread is retained in JSON.

| Workload/comparison | Compiler seconds | Compiler peak KiB | Runtime seconds | Executable text bytes | Paired compile ratio (range) | Paired runtime ratio (range) |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.1841 → 0.2305 | 29428 → 29268 | 0.0716 → 0.0722 | 151633 → 151633 | 1.069 (0.921–1.444) | 1.005 (0.993–1.761) |
| floating | 0.2609 → 0.2609 | 29064 → 29276 | 0.0513 → 0.0561 | 151474 → 151474 | 1.000 (0.376–1.220) | 1.038 (0.992–1.083) |
| exceptions | 0.2265 → 0.2210 | 29024 → 29156 | 0.3125 → 0.3134 | 151781 → 151781 | 1.029 (0.844–1.194) | 1.002 (0.739–1.329) |
| pruning | 0.1924 → 0.1936 | 35020 → 35176 | 0.0737 → 0.0736 | 151633 → 151633 | 1.011 (0.996–1.231) | 0.998 (0.996–1.028) |
| 600 constructor specializations entry → final, named | 0.3187 → 0.3213 | 33428 → 33928 | 0.5469 → 0.5475 | 108712 → 108712 | 1.006 (0.998–1.239) | 0.988 (0.957–1.012) |
| 600 constructor specializations final named → projected | 0.2047 → 0.1673 | 33848 → 30040 | 0.5946 → 0.3991 | 108712 → 90112 | 0.816 (0.800–0.831) | 0.672 (0.543–0.691) |
| 8192 constexpr objects entry → final, named | 0.4265 → 0.5286 | 71840 → 71984 | 0.0566 → 0.0570 | 436 → 436 | 1.104 (0.847–1.380) | 1.006 (0.998–1.014) |
| 8192 constexpr objects final named → projected | 0.4084 → 0.4226 | 71980 → 70556 | 0.0566 → 0.0565 | 436 → 436 | 1.037 (1.021–1.108) | 0.994 (0.986–1.007) |

A/A calibration ranges, seconds:

| Workload/comparison | Compiler A/A | Runtime A/A |
|---|---:|---:|
| memory | 0.2629–0.2718 | 0.0701–0.0732 |
| floating | 0.1619–0.2349 | 0.0587–0.0607 |
| exceptions | 0.1563–0.2008 | 0.2553–0.2885 |
| pruning | 0.1938–0.3032 | 0.0530–0.0553 |
| 600 constructor specializations entry → final, named | 0.2035–0.4008 | 0.4955–0.5387 |
| 600 constructor specializations final named → projected | 0.2060–0.2090 | 0.5922–0.5966 |
| 8192 constexpr objects entry → final, named | 0.4086–0.7145 | 0.0561–0.0568 |
| 8192 constexpr objects final named → projected | 0.4016–0.4854 | 0.0563–0.0567 |

## Interpretation and stage-scoped acceptance

All four common objects are byte-identical before/after, as are the named constructor benchmark objects and executables. The constant benchmark emits identical object bytes for entry/named, final/named and final/projected; all have 436 text bytes after host link. Host executable hashes can differ even with identical object/text bytes. Thus the runtime variation in same-source comparisons is measurement noise, not evidence of generated-code regressions or improvements.

Compiler paired medians on the common inputs are 1.000–1.069. The constructor-focused same-source comparison is 1.006; it adds exactly 600 declaration-owned default contexts/scopes, preserving 2400 actions. The large same-source constexpr run measured 1.104 with a wide 0.847–1.380 paired range and 0.409–0.714 s A/A range. Its semantic work counters are unchanged (49152 constant-object work units, 65536 address units, 40960 activations), and peak RSS rises 144 KiB. Empty projection indexes allocate lazily. These data disclose timing variability and necessary state costs; they do not establish a repeatable compiler speedup or a precise slowdown. No unsupported percentage gate is imposed.

Projected source is newly supported, so it cannot be compared against the failing entry compiler. The named/projected pairs use the **same final binary** on semantically equivalent correct programs. Runtime construction needs 600 projected paths and 1800 actions, versus 2400 actions and additional named-storage constructor calls. It emits 90112 versus 108712 executable text bytes and has a measured runtime ratio of 0.672. This is a representation comparison, not a before/after optimization claim. The nested constexpr comparison has two prepared paths shared by 8192 evaluations, 65536 object-work units, 90112 address-work units, 24576 activations and identical final object bytes; its 1.037 compile ratio accounts for additional nested projections.

No optional optimizer, speculative rewrite, new optimization level or growth allowance was added. Required semantic work owns the costs: one context per class with demanded defaults, one path per selected storage in each constructor plan, and one action per initialized leaf/base. Paths occupy 24 bytes and actions 28 bytes on this target. A declaration-order walk uses O(fields + explicit initializer path edges) work; lowering consumes these facts in O(actions + produced IR). Layout/source nodes are not cloned.

Constant projected groups/parts live in flat activation-owned buffers and indexes, without per-group child-container allocations. Prefix paths are shared, each group is frozen once, and each dependency key includes reachable references and storage versions. Preparation is O(leaves + storage edges); dependency traversal is bounded by the reachable semantic graph and uses visited identities. Temporary buffers are destroyed with the activation; canonical values/addresses have TU ownership. Existing constant-evaluation and native cleanup limits remain intact.

This satisfies the current-stage evidence/structural acceptance for a correctness change. There is no optional transform whose profit is being inferred from smaller IR. Compiler timing spread and whole-stage architecture questions remain visible for independent audit; required correctness, coverage and future-stage obligations are not relaxed.

Reproduce with `performance147_common.py OUT ENTRY FINAL` and `performance147.py OUT ENTRY FINAL [constant-construction]`, setting `PERF_CPU=0`. Scripts are in this directory. All prior145/146 measurements remain unchanged.
