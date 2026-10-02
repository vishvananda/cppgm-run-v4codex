# PA33 implementation handoff 219

Stage base commit: 676b6e328c7a05334b15f1c9ee1ec30c783e9aff
Last reviewed commit: 676b6e328c7a05334b15f1c9ee1ec30c783e9aff

Target: **PA33 full-stage**. Phase: **implementation complete; independent audit
pending**. Entry: 47/73 reported passing, ten MIR failures, controls stopped at
strlen and debug not completed. Final: all 57 MIR/behavior fixtures, five native
controls, 18 driver modes and 11 debug fixtures pass. No coverage, reference or
comparison changes. Review markers above are deliberately unchanged.

## Design/spec alignment

Both adapters pass the requested native level through typed LowIR → function
Selector → consumed MIR → direct ELF. Source/object paths retain PA32 optimization
before selection. O1–O3 share the policy below; O0 retains placement and gains the
required unused-result memcpy control. There is no production text roundtrip.
[Owner proofs and reproduction](../student.tests/pa33/evidence219/README.md) and
[bound records](../student.tests/pa33/evidence219/binding.json) carry details.

| Owner | Data flow, legality and fallback | Work/growth limit |
| --- | --- | --- |
| `native/global_placement` | Typed uses/definitions + fixed effects → unique whole-function registers; covers backedges and parallel phi transfers; EH/multiple definitions remain conservative | O(I+V log V), O(V) temporary storage; <=7 retained values, <=5 saved registers; no clones/retries; ordinary bounded selection handles spills |
| Control/frame/bulk | Single MIR compaction preserves labels/debug/effects; direct register bulk addresses feed encoding; optimized integer Boolean comparisons need no FP scratch | O(MIR), zero cleanup growth; unused scratch removed |
| `native/calls`, `call_policy`, `prefix_call` | Complete canonical builtin + ABI/effect gates → MIR call fact or dynamic copy; page-safe prefix with original call fallback; parallel argument setup retained | O(1)/site; <=8 prefixes/function, <=128/unit; +53 encoded bytes/site, <=64 reserved (512/function, 8192/unit); REP adds no call expansion loop |
| LowIR metadata adapter | Explicit `builtin=` preserves hosted identity independently of object spelling; rejects conflicting/invalid identities; source lowering records typed provenance | Constant fields/entity, linear read/write; no production reconstruction |

Each function's transient placement/MIR dies after encoding. New telemetry counts
candidates, retained values, removed controls and selected builtin operations
without extra reporting analyses. Remaining whole-stage ownership/ABI/debug
questions belong to independent review; no known implementation defect is waived.

## Validation and performance

Required checks pass: `make test-pa33` **57/57** plus controls/driver modes;
`make -C pa33 test-debuginfo` **11/11**; `make test-report-through-pa32`
**5397/5397**; `make test-report-through-pa33` **5454/5454**; PA33 file audit
passes with four inherited header warnings. Explicit personal checks cover
cyclic phis, six-parameter pressure, copy ABI ordering, protected-page strings,
all native levels and prefix work/growth caps. Exact commands/log hashes are in
[checks](../student.tests/pa33/evidence219/checks.json).

[Frozen measurements](../student.tests/pa33/evidence219/performance.md) retain
A/A calibration, six ABBA pairs, every compiler wall/RSS and checked runtime
sample, text sizes and all rejected observations. Final binaries equal candidate2.
Affected runtime B/A medians: loop **0.348**, calls **0.786**, short strings
**0.558**, dynamic copy **0.032**; compile ratios **0.996–1.033**, peak RSS grows
by at most **5.6%** on these inputs. Loop/call text shrinks; bounded prefix growth
is **6784 bytes/unit**, below 8192. A 127-byte hosted string pays **1.122x** runtime
for the required prefix probe, with compiler 1.016x and the same bounded text
cost. An attempted suffix continuation is rejected: controlled candidate2/3
ABBA gives **1.389x** runtime and more bytes; its source diff/data remain archived.
This required probe's long-string overhead is disclosed, not a claimed benefit.

Common O0 outputs are byte-identical; O2 common text shrinks 10–25 bytes, and
floating runtime is **0.888x**. The compiler-component object is byte-identical
(34466 text bytes); peak compiler RSS is 77624/77888 KiB. No compiler speedup is claimed from noisy near-one
ratios. Fixed template/memory/FP/EH and compiler-component inputs supplement the
affected cases. Mandated predicates and implemented work/growth budgets remain
gates. Inherited ad hoc timing ratios are diagnostics under spec.md's stage rule;
no extra profiler, allocator statistic or unsupported zero-regression gate is added.

## Handoff ledger

- `441d5ec9`: clean entry, baseline and owner groups recorded before stage edits.
- `f9df1b5b`: level propagation, bounded placement, frame/control/bulk facts and
  builtin selection; all PA33 course controls pass. Earlier object replay found
  two missing hosted builtin identities; these were unfinished implementation.
- `cbf6f115`: explicit builtin fact serialization and incompatible declaration
  fallback close both replay failures; final full checks pass. One restricted-CPU
  PA3 timeout is preserved; normal-setting required reports subsequently pass.
- Boundary 219: all identified related groups complete, no unfinished
  implementation. Independent whole-stage architecture/performance/ABI/debug
  audit remains required before advancement; this handoff does not certify it.
  Evidence/plan records preserve the base/review markers and rejected experiments.

Verify the committed handoff with `python3 student.tests/pa33/records.py verify --clean`.
