# PA17 member ownership performance evidence

Source: `5ea7f7f673797126239091f27d2d1ac12d53069d`; entry: `4bbb712a`.
[Raw evidence](../student.tests/pa17/member-performance.json) records frozen
binary hashes, flags, inputs/source hashes, telemetry, output hashes, every
observation and timing ranges. The
[harness](../student.tests/pa17/member_benchmark.py) checks each command's result.
Historical loop 45 campaigns remain in [entity-performance.md](entity-performance.md).

Common workloads have a warmup per binary, four A/A observations and four ABBA
blocks on a pinned CPU. Newly supported member workloads have a final-binary
warmup and six observations; the entry binary rejects them. Compiler and native
execution are timed separately. Source workloads take 67–1,192 ms; runtime
loops take 210–361 ms. Tiny runtime-source compilation (about 6 ms) is
startup-sensitive. Volatile bounds and result checks prevent dead runtime work.
The supplied native backend is only a validation consumer of student LowIR.

The [earlier loop 46 campaign](../student.tests/pa17/member-performance-before-alias-access.json)
also remains preserved; current results include the alias-definition access fix.

| Compiler workload | A / B median ms | A / B peak KiB | Median paired B/A |
|---|---:|---:|---:|
| common-partials-1500 | 107.79 / 110.31 | 22,200 / 22,108 | 1.0202 |
| common-loop-float-1500 | 151.64 / 150.61 | 29,960 / 29,944 | 0.9978 |
| entity-pack-alias-1500 | 135.43 / 137.01 | 26,516 / 26,624 | 1.0158 |
| common-partials-6000 | 440.94 / 448.72 | 72,352 / 72,520 | 1.0178 |
| common-loop-float-6000 | 590.72 / 593.40 | 104,032 / 104,080 | 1.0052 |
| entity-pack-alias-6000 | 569.13 / 577.10 | 89,440 / 89,652 | 1.0155 |
| member-qualified-1500 | rejected / 283.87 | — / 49,724 | not comparable |
| common-qualified-access-1500 | 67.03 / 68.40 | 15,092 / 14,956 | 1.0194 |
| member-qualified-6000 | rejected / 1192.15 | — / 181,372 | not comparable |
| common-qualified-access-6000 | 262.98 / 272.44 | 42,836 / 44,564 | 1.0346 |
| runtime-calls | 5.78 / 5.78 | 5,644 / 5,696 | 1.0026 |
| runtime-memory | 5.92 / 5.87 | 5,604 / 5,680 | 0.9927 |
| runtime-floating | 5.92 / 5.84 | 5,852 / 5,860 | 0.9927 |

| Native workload | A / B median ms | Text bytes A = B | Median paired B/A |
|---|---:|---:|---:|
| runtime-calls | 359.64 / 360.29 | 206 | 1.0046 |
| runtime-memory | 210.59 / 210.76 | 434 | 1.0009 |
| runtime-floating | 248.76 / 248.66 | 230 | 1.0000 |

Compiler `.text`: **1,701,830 → 1,717,190 bytes**, +15,360 (0.90%). All
common LowIR outputs and all three native executables are byte-identical.
Native text is the sectionless executable payload after ELF entry; these
runtime inputs have no static data. Native peak RSS is 256 KiB throughout.

The larger qualified-access workload shows a **3.46% paired compiler cost**
and **1,728 KiB** additional peak RSS. It now checks retained source access
obligations, including invalid private-type reducers accepted by the entry
binary. The work is required semantics. Source subtree summaries are reused;
completed member selection is shared with type substitution, avoiding a second
member lookup. Access results use complete source-recipe/frame identities;
naming exemptions never suppress these definition-owned checks. The smaller
qualified-access workload shows 1.94% paired overhead with comparable peak RSS.

Other common compiler workloads show paired medians near 1.00–1.02. All
observations and timing ranges remain in the JSON, including outliers. No
speedup is claimed from these data. Native timing differences compare identical
binaries. The earlier campaign's 4.02% access cost and 820 KiB RSS difference
remain historical observations, not replacement figures for current acceptance.

New member workloads compile in **283.87 / 1,192.15 ms** for 1,500 / 6,000
owners (**4.20× time for 4× input**), with **49,724 / 181,372 KiB** peak RSS.
These compile-only inputs have no entry point. Their cost is not an A/B
optimization result because the entry compiler rejects the same source.

PA17/O0 has no mandated numerical latency, RSS or text ceiling. No optional
optimization is added and no executable-speed benefit is claimed. The measured
bounded costs implement required declaration, access and demand semantics.
Inherited +15%, +16 MiB and 5.5× targets remain diagnostics under spec.md's
stage-scoped acceptance; no mandated limit, evaluator bound, correctness check,
fixture or comparison rule was changed. Native optimization/MIR and self-hosting
remain later-stage owners.
