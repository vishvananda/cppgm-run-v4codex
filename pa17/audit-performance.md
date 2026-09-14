# PA17 checkpoint audit performance — loop 48

Source: `58789b00e19ae2aff032c8b04980b7d43265617f`. Frozen checkpoint comparator:
`232f4a93` (same implementation as `3bef03e0`); cumulative comparator: stage base `21748547`.
[Checkpoint observations](../student.tests/pa17/audit-performance.json) and
[cumulative observations](../student.tests/pa17/audit-base-performance.json) retain
binary/source/backend/harness hashes, flags, phase/work telemetry, checked output
hashes, every warmup and observation, A/A noise ranges, paired ratios and spreads.
The [harness](../student.tests/pa17/audit_benchmark.py) measures compilation and
native execution separately: one warmup per binary, four A/A samples and four
ABBA blocks on one pinned CPU. All compiler outputs are validated before timing
and byte-identical within each comparison. Runtime bounds are volatile and every
run checks its result; all corresponding native binaries are byte-identical.

The checkpoint corpus preserves all eighteen loop-47 workloads and adds four
wide alias/partial workloads (300 distinct instantiations at widths 128 and 512).
The cumulative corpus uses the seven inputs supported correctly by the stage base.
Newly implemented features with rejecting historical comparators remain separately
documented in the retained entity/member/definition campaigns; rejection is never
counted as a fast equivalent compilation. The
[pre-head-fix campaign](../student.tests/pa17/audit-performance-before-heads.json)
is preserved as historical evidence, alongside all earlier measurements.

| Checkpoint compiler workload | A / B median ms | A / B peak KiB | Paired B/A median (range) |
|---|---:|---:|---:|
| common-partials-1500 | 110.35 / 110.07 | 22,312 / 22,404 | 1.0008 (0.9844–1.0082) |
| common-loop-float-1500 | 153.45 / 153.51 | 30,004 / 30,256 | 0.9959 (0.9593–1.0385) |
| entity-pack-alias-1500 | 137.52 / 138.41 | 26,096 / 26,960 | 1.0000 (0.8434–1.0202) |
| common-partials-6000 | 449.89 / 453.71 | 72,556 / 72,968 | 1.0065 (0.9878–1.0133) |
| common-loop-float-6000 | 598.95 / 595.55 | 104,076 / 104,080 | 0.9325 (0.8567–1.0010) |
| entity-pack-alias-6000 | 574.65 / 573.00 | 89,912 / 89,264 | 1.0010 (0.9899–1.0087) |
| member-qualified-1500 | 286.35 / 287.36 | 49,724 / 49,396 | 1.0013 (0.9966–1.0154) |
| common-qualified-access-1500 | 69.56 / 69.03 | 15,216 / 15,368 | 0.9937 (0.9862–1.0005) |
| member-qualified-6000 | 1195.98 / 1199.20 | 180,488 / 182,232 | 1.0016 (0.9981–1.0119) |
| common-qualified-access-6000 | 274.01 / 275.79 | 44,608 / 45,308 | 1.0083 (0.9913–1.0735) |
| primary-definition-1500 | 258.24 / 257.04 | 42,372 / 42,264 | 0.9952 (0.9851–1.0199) |
| partial-definition-1500 | 196.23 / 197.12 | 36,000 / 36,140 | 1.0007 (0.9936–1.0133) |
| primary-definition-6000 | 1062.46 / 1072.26 | 153,108 / 154,196 | 1.0062 (0.9897–1.0153) |
| partial-definition-6000 | 821.91 / 829.31 | 127,552 / 127,928 | 1.0112 (1.0012–1.0164) |
| partial-runtime | 5.85 / 5.87 | 5,860 / 5,796 | 0.9961 (0.9896–1.0046) |
| runtime-calls | 5.96 / 5.99 | 5,692 / 5,736 | 1.0026 (0.9936–1.0372) |
| runtime-memory | 6.07 / 6.00 | 5,772 / 5,736 | 0.9939 (0.9642–1.0209) |
| runtime-floating | 5.73 / 5.75 | 5,832 / 5,820 | 1.0002 (0.9785–1.0078) |
| wide-alias-128 | 351.24 / 320.56 | 58,372 / 53,020 | 0.9147 (0.9120–0.9169) |
| wide-partial-128 | 460.54 / 429.40 | 80,972 / 75,828 | 0.9321 (0.9298–0.9384) |
| wide-alias-512 | 1559.71 / 1293.83 | 217,572 / 200,416 | 0.8301 (0.8231–0.8333) |
| wide-partial-512 | 2004.01 / 1734.78 | 306,608 / 283,992 | 0.8656 (0.8599–0.8699) |

| Stage-base compiler workload | A / B median ms | A / B peak KiB | Paired B/A median (range) |
|---|---:|---:|---:|
| common-partials-1500 | 107.17 / 108.69 | 22,024 / 22,312 | 1.0139 (0.9771–1.0173) |
| common-loop-float-1500 | 150.59 / 150.74 | 29,924 / 30,132 | 0.9996 (0.8566–1.0064) |
| common-partials-6000 | 443.81 / 452.23 | 72,476 / 72,940 | 1.0231 (1.0023–1.0759) |
| common-loop-float-6000 | 594.76 / 598.26 | 103,904 / 104,112 | 1.0048 (0.9811–1.0194) |
| runtime-calls | 6.02 / 6.03 | 5,672 / 5,732 | 0.9760 (0.9675–1.0372) |
| runtime-memory | 5.96 / 5.86 | 5,696 / 5,744 | 0.9772 (0.9728–1.0036) |
| runtime-floating | 5.86 / 5.80 | 5,852 / 5,816 | 0.9691 (0.9623–0.9810) |

| Comparison / native workload | A / B median ms | Text bytes A = B | Paired B/A median (range) |
|---|---:|---:|---:|
| checkpoint / partial-runtime | 362.17 / 362.80 | 206 | 0.9996 (0.9967–1.0178) |
| checkpoint / runtime-calls | 358.80 / 358.89 | 206 | 0.9991 (0.9984–1.0043) |
| checkpoint / runtime-memory | 210.98 / 211.32 | 434 | 1.0005 (0.9989–1.0029) |
| checkpoint / runtime-floating | 248.40 / 248.77 | 230 | 1.0011 (0.9986–1.0021) |
| stage-base / runtime-calls | 361.52 / 361.41 | 206 | 0.9985 (0.9901–1.0099) |
| stage-base / runtime-memory | 211.61 / 210.92 | 434 | 0.9990 (0.9871–1.0095) |
| stage-base / runtime-floating | 249.33 / 249.56 | 230 | 1.0007 (0.9990–1.0036) |

Compiler `.text`: stage base **1,669,510**, checkpoint **1,718,406**,
reviewed **1,720,006 bytes**: +1,600 bytes over the checkpoint,
+50,496 bytes (3.02%) over the full stage base. Native text is the sectionless
executable payload after ELF entry for these inputs without static data; native
RSS is 256 KiB. The supplied backend is a validation consumer, not part of the
student implementation. No student native allocator, encoder or self-hosting
performance claim is made at PA17.

Wide-head compiler improvements repeat in every ABBA block: about 8.5% / 17.0%
for aliases and 6.8% / 13.4% for partial matching at 128 / 512 parameters.
The 512-wide alias workload uses **300 frames instead of 153,600**; the partial
workload uses **1,507 instead of 155,619**. Substitution work counts stay equal
(153,900 and 156,435 respectively). Peak memory drops by **17,156 / 22,616 KiB**.
This is a bounded semantic-environment improvement, with no executable transform
or change to demanded behavior. Both the historical and final campaigns confirm it.

Common checkpoint compiler paired medians are otherwise near unity. The larger
partial-definition workload costs 1.12% and 376 KiB; member-qualified-6000 costs
0.16% and 1,744 KiB. The complete frame key, alias fact states and correct nested
head normalization account for bounded semantic/index storage. The cumulative
common-partials-6000 cost is 2.31%; the stage-base comparison includes the required
structural selection and retained argument facts from all three handoffs.
The common-loop-float-6000 paired ratio of 0.9325 is influenced by slow A samples;
its ordinary medians are 598.95 / 595.55 ms. No loop speedup is claimed and no
outliers were removed. Native timing differences compare identical executable bytes.

Compiler source workloads take roughly 69–2,004 ms and native loops 211–363 ms.
The four tiny runtime-source compilations take about 6 ms, remain startup-sensitive,
and support no compiler speedup claim. Noise calibration and all timing spreads
remain in the raw records.

PA17/O0 requires correct bounded work but mandates no numerical latency, RSS,
runtime or text ceiling. The inherited +15%, +16 MiB and 5.5× diagnostic targets
are not exit gates under spec.md §9. Their measurements remain preserved. The
audit removed the avoidable wide-head walks and frame collision chains; the
remaining measured costs establish required semantic facts. No optional optimizer
was added, and no runtime-profit or later-stage MIR/native requirement is asserted.
Constant-evaluator limits, existing lowering growth policies, correctness,
coverage and all comparison rules remain intact.
