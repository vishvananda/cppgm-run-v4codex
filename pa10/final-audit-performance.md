# PA10 independent final performance assessment

The retained final compiler is the implementation at `60f7088f`, SHA256
`735577050dd7757fad4b71577ac153656bf87d7d986a0f9b30a326c40ea098f7`.
The current `dev/cppgm++` was rebuilt and checked byte-identical to this frozen
binary. It is B in [independent-performance.json](../student.tests/pa10/independent-performance.json)
and A in [consolidated-performance.json](../student.tests/pa10/consolidated-performance.json).
The latter filename records the final decision campaign, not acceptance of B.
Candidate B (`4fc61de6`, SHA256 `4e6c7fb9680b23d818a887b7dbc5977dce15ba8a9b9ca584e5a86a7ca24e19f7`)
is rejected and its source change reverted. Its frozen artifacts are retained.

Both campaigns follow the committed [protocol](../student.tests/pa10/performance-protocol.md):
fixed binaries/inputs/flags, pinned CPU, four A/A observations followed by two
ABBA blocks, checked output equivalence, separate wall/RSS measurement and
separate work telemetry. Each retains 108 compiler, eight startup and 36 native
observations. No builds/tests ran during timing. All observations and outliers
remain. Earlier campaigns and their assessments remain in [performance.md](performance.md).

## First-correct implementation versus independent audit

A is `17f3deb7`; B is the retained final implementation. Seconds are medians;
RSS is maximum observed KiB. Ratios are the two paired B/A blocks, followed by
the A/A relative range. These workloads are correct on both implementations;
newly corrected behaviors are tested separately, never timed against wrong code.

| Workload | A seconds | B seconds | A/B RSS KiB | Paired B/A | A/A spread |
|---|---:|---:|---:|---|---:|
| calls-1 | 0.79907 | 1.00945 | 71432/71396 | 0.9617, 1.1081 | 37.14% |
| memory-float-1 | 0.74791 | 0.79260 | 65428/65644 | 0.7483, 0.9806 | 47.84% |
| references-1 | 0.07751 | 0.03798 | 6312/6456 | 0.5089, 0.4827 | 10.88% |
| template-semantics-1 | 0.17622 | 0.18269 | 11968/12140 | 0.9709, 1.0558 | 10.12% |
| calls-4 | 1.56452 | 1.56810 | 274128/275056 | 1.0031, 1.0036 | 1.06% |
| memory-float-4 | 1.35987 | 1.42291 | 233768/249576 | 1.0455, 1.0456 | 2.66% |
| references-4 | 0.30886 | 0.04782 | 12388/12628 | 0.1559, 0.1526 | 33.05% |
| template-semantics-4 | 0.25125 | 0.25221 | 35548/35672 | 1.0046, 0.9054 | 2.02% |
| references-8000 | 2.80716 | 0.11266 | 22080/23828 | 0.0401, 0.0398 | 2.79% |

The required ownership/control/static-conversion work adds a repeatable ~4.6%
latency on the large floating/memory sample and 15808 KiB peak RSS in this run;
other groups and all observations are disclosed above. Compiler text grows
532422 → 564230 bytes (+31808, 5.97%). The long reference chain still measures
about 2.81 s versus 0.113 s, with paired ratios ~0.04. Its B duration is below
the protocol's 20x-startup diagnostic threshold in this campaign (startup
medians ~10.96/10.54 ms), so it is not used for a new speedup claim. The historical
long-case speedup evidence and current linear work evidence remain intact.
Short reference/template samples are diagnostic, not throughput claims.

## Final profitability decision

A below is the retained compiler; B only changes the redundant C object-name
check to avoid display construction for ordinary C++ declarations. All nine
output pairs are **byte-identical**, stronger than relaxed LowIR equivalence.
Unequal files still use the unchanged course comparator in the personal harness.

| Workload | Retained A seconds | Candidate B seconds | A/B RSS KiB | Paired B/A | A/A spread |
|---|---:|---:|---:|---|---:|
| calls-1 | 0.38681 | 0.38707 | 71592/71596 | 0.9899, 0.8776 | 1.21% |
| memory-float-1 | 0.33772 | 0.34093 | 65644/65616 | 1.0105, 1.1979 | 1.06% |
| references-1 | 0.01582 | 0.01568 | 6456/6452 | 1.0064, 1.0005 | 6.44% |
| template-semantics-1 | 0.06594 | 0.06587 | 12196/12200 | 0.9932, 0.9998 | 1.77% |
| calls-4 | 1.56280 | 1.56883 | 276648/276648 | 1.0037, 1.0073 | 7.17% |
| memory-float-4 | 1.36442 | 1.42560 | 249600/249564 | 1.0209, 1.0500 | 1.74% |
| references-4 | 0.04842 | 0.04729 | 12628/12636 | 0.9674, 0.9850 | 2.55% |
| template-semantics-4 | 0.25153 | 0.25087 | 35676/35652 | 1.0215, 0.9971 | 77.43% |
| references-8000 | 0.11080 | 0.11078 | 23940/23948 | 0.9951, 0.9980 | 1.08% |

The candidate saves 640 compiler-text bytes (564230 → 563590), but its large
floating/memory slowdown is 2.09% and 5.00%, exceeding the 1.74% A/A spread.
No repeatable compiler benefit compensates for that regression; generated code
is identical. The optional change is therefore removed. Smaller samples include
outliers (calls and floating/memory second blocks, template-4 A/A); none is
excluded or used to assert a speedup. Cross-campaign medians also vary, so no
cross-campaign latency ratio is used as paired evidence.

Retained A's final calibration median is 5.50 ms. Its 8000-reference workload
is ~0.111 s, just above the diagnostic 20x startup threshold. Its fourfold-input
latency factors are calls 4.04x, memory-float 4.04x, template-semantics 3.81x, references 3.06x.
No pending numeric performance gate remains. The self-selected 1.10x latency,
1.20x RSS +16 MiB, +128 KiB compiler text and fourfold growth targets remain
diagnostic; no mandated PA10 limit or correctness coverage was weakened.
Required semantic work and later-stage native constraints do not create an
extra exit gate. The tested unprofitable optional change was actually reverted.

## Generated executable runtime and size

The final decision campaign's A/B executable hashes are identical for all three
workloads, and also match the first-correct and earlier final binaries' generated
executables. All checked results return 0. Runtime trip counts are volatile
96M calls, 64M indexed updates and 32M floating calls; the observable checksums
prevent dead work. Compilation and native execution are measured separately.

| Workload | Retained A seconds | Candidate B seconds | Payload bytes A/B | Paired B/A | A/A spread |
|---|---:|---:|---:|---|---:|
| calls-long | 0.47745 | 0.47884 | 206/206 | 1.0026, 0.9984 | 0.90% |
| memory-long | 0.27919 | 0.27906 | 434/434 | 1.0028, 0.9947 | 0.83% |
| floating-long | 0.33037 | 0.33139 | 230/230 | 1.0030, 0.9992 | 0.50% |

Native peak RSS is 256 KiB in these observations. No runtime speedup is claimed.
The backend's sectionless ELF metric counts bytes from entry to executable
PT_LOAD end; these fixed workloads have no static data. It is not mislabeled as
an ELF .text section. Native register/frame details are recorded in the
[architecture trace](audit.md); the supplied backend owns allocation and encoding.

## Work and storage evidence

Calls at 3500/14000 pairs have 3500/14000 semantic candidates, 35000/140000
control-entry visits and 84000/336000 instructions. Floating/memory has
28000/112000 control visits and 84000/336000 instructions. Cursor lookahead
peaks at four/eight tokens respectively, and node pool growths are 19/21.
These are observations of existing work, not analyses triggered by telemetry.
Static reference requests/hits remain 32000/15999 at 8000 bindings; per-key
computation stays linear. The separate 4/16-TU linkage evidence keeps 1026 ABI
nodes and 131200 IR capacity bytes while requests grow 2049 → 8193 and hits
1536 → 7680. Retained summaries track unique entities, not duplicated TUs.

O0 has no optional IR passes, fixed-point scans, inlining or unrolling. New
entry flags and discarded-access memoization each use at most one byte per
parsed node; scoped builders and TU arenas retain the documented release points.
Template execution, self-hosting, MIR/backend optimization and native floating/
variadic parity remain outside this PA's acceptance scope.
