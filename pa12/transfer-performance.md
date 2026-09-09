# PA12 transfer evidence

This continuation adds the special-member declaration/action model in `73664568`
and then removes the unprofitable scalar-only prefix fold. Earlier measurements
remain in [performance.md](performance.md). This is O0 source-to-LowIR work;
executables use the handout's supplied PA8 native backend.

## Frozen protocol and observations

[Harness](../student.tests/pa12/transfer_benchmark.py): pinned CPU, four A/A
observations followed by two ABBA blocks. Compiler wall time and `/usr/bin/time`
peak RSS are measured separately from execution. Validation and semantic work
counters use untimed runs. Each native program checks its result; the runtime
loops have four million iterations controlled by a volatile bound. Source,
binary, backend and harness hashes, flags, all observations, paired ratios and
A/A ranges are retained in the linked JSON files. Native text uses the existing
sectionless ELF payload metric; compiler text is `.text`.

- Entry compiler: `/tmp/pa12-transfer-base-cppgm`, SHA256
  `9959f526bcc714b21f5491dbedbe90465460d149a70ad9f42123881bf688654c`.
- Equivalent field-wise variant A: `/tmp/pa12-transfer-evidence/fieldwise-cppgm`,
  SHA256 `72f934ab646aa643990a234e51a5149242d12a270b383c306b06b765f56d36db`.
  Starting from `73664568`, disable only the non-union prefix-fold block with
  `else if (false)`, compile that translation unit with the ordinary
  `g++ -std=gnu++11 -Wall -O3 -Idev/src` flags, and relink the same objects.
  The modified source is retained as `fieldwise_transfer_actions.cpp` there.
  Union representation copies, allocation-unit transfers and all selected
  constructors/assignment functions remain unchanged.
- Initial unrestricted prefix B: `/tmp/pa12-transfer-evidence/prefix-cppgm`,
  SHA256 `b7f7443cda562c1f18476b69a7861373ddfe3cc2c0730ef94a894b59e3ac7ca2`.
- Final bounded B: `/tmp/pa12-transfer-evidence/bounded-prefix-cppgm`, SHA256
  `ca9a6344d384b58ae2263c6ef24f16c92041cfdc4db987ade65c858d3ed58f02`.
  Its compiler text is 815302 bytes, versus entry 795014 and field-wise A 814150.

[Initial campaign](../student.tests/pa12/transfer-performance.json),
[8/16/32/64-byte probes](../student.tests/pa12/prefix-probes-performance.json),
[storage-prefix probe](../student.tests/pa12/storage-prefix-performance.json),
[final transfer campaign](../student.tests/pa12/bounded-transfer-performance.json)
and [common-input comparison](../student.tests/pa12/transfer-common-performance.json)
retain every observation. Frozen harness versions and inputs are under
`/tmp/pa12-transfer-evidence/`, `/tmp/pa12-prefix-probes/`,
`/tmp/pa12-storage-prefix-evidence/`, `/tmp/pa12-bounded-transfer-evidence/`
and `/tmp/pa12-transfer-common-evidence/`.

## Acceptance and final results

The unrestricted 32-byte scalar prefix reduced compiler work and native text
but slowed the runtime loop from median 0.05811 to 0.07355 seconds; paired B/A
ratios were 1.262/1.283. Independent size probes confirmed losses, including
8-byte ratios 2.120/2.058 and 64-byte ratios 1.175/1.162. This optional fold is
removed. Final scalar-prefix LowIR is byte-identical to field-wise A, as are
its executable hashes and text sizes. Its paired runtime ratios are now
1.0021/1.0028, within the observed spread. No runtime speedup is claimed.

| Final workload | Compiler A / B seconds | Timed peak RSS A / B KiB | Native A / B seconds | Native text A / B bytes |
| --- | --- | --- | --- | --- |
| 1000 transfer classes | 0.56627 / 0.56434 | 117052 / 117004 | 0.00325 / 0.00329 | 730048 / 730048 |
| 4000 transfer classes | 2.30609 / 2.32709 | 409784 / 409592 | 0.00324 / 0.00317 | 2920048 / 2920048 |
| Scalar-prefix runtime | 0.00625 / 0.00620 | 4880 / 4888 | 0.05820 / 0.05826 | 955 / 955 |

The 1000-class A/A interval is 0.56452–0.60864 seconds; final ABBA A includes
a 0.78283-second observation. The 4000-class A/A interval is 2.29224–2.32763;
ABBA A includes 2.79914 and B 2.47747 seconds. All are preserved. The second
paired compiler blocks are 1.0023 and 1.0079; the outliers do not establish a
compiler speedup. Final 4x scaling uses 4.124x time and 3.501x timed RSS.
Transfer actions grow 36000 -> 144000, candidates 19001 -> 76001, and processed
member demands 8000 -> 32000. Startup-sized native runs support correctness,
not a runtime claim.

Common member workloads compare the entry compiler to final B, with identical
LowIR/native outputs: 1000 classes take 0.12518/0.12630 seconds and
27276/27144 KiB; 4000 take 0.50521/0.50540 and 95564/95596 KiB. Native text stays
70053/280053 bytes. The inherited empty-destructor loop stays byte-identical,
with text 241 bytes and runtime 0.08411/0.08325 seconds. These measurements
show no material common-workload regression from the required new semantics.

The remaining storage-prefix probe (scalar followed by union storage) runs
0.03869/0.07371 seconds, paired ratios 1.898/1.933, while text shrinks 489 -> 441
bytes. No profit claim is made for it. Required PA12 fixtures
`300-leading-trivial-prefix-storage-copy{,-enum}` prescribe this prefix form;
whole-object transfer fixtures likewise prescribe `copyobj`. Their normalized
comparison does not accept the field-wise alternative. Those O0 representation
requirements remain; the supplied backend's small-copy implementation is a
later-stage constraint, not a new positive-runtime exit gate. This classification
preserves the mandated output and records the loss rather than treating smaller
IR as runtime evidence. The optional scalar-only case was independently removed
without losing any passing course test.

## Work and growth budgets

Special-member declarations and completed transfer/deletion/triviality facts are
cached by class/member identity for the translation unit. Candidate ranking uses
the existing conversion rules, then lowering consumes typed action slices.
Preparation visits required subobject edges; prefix discovery is one bounded
linear scan with no fixed-point work. Adjacent nonvolatile bit-fields transfer
one allocation unit; unsafe layouts retain value semantics. Array expansion is
limited to eight total elements across dimensions, otherwise one counted loop.
No body cloning, speculative inlining or backend optimizer is added. The
scalar-only fallback emits exactly the existing field-wise action sequence, with
zero growth relative to that conservative baseline. Required semantic costs and
mandatory representation limits create no additional performance exit gate.
