# PA33 final plan and audit 220

Stage base commit: 676b6e328c7a05334b15f1c9ee1ec30c783e9aff
Last reviewed commit: d7b7c9cf

Target: **PA33 full-stage**. Phase: **final audit complete**. The independent
[audit](audit.md) reconstructs the architecture, traces source/template/builtin
facts through ELF, closes every handoff and records legality, profitability,
invalidation, work/growth, ABI/debug and lifetime evidence. No open PA33 finding
or unaudited handoff remains. Course fixtures/references/comparisons are unchanged.

## Final Spec Alignment

Both tools consume shared typed LowIR → per-function Selector/MIR → direct ELF.
Source/object paths apply PA32 first. Frontend interned identities, retained
parsed template regions, precise fact demands and immutable substitution frames
feed typed lowering; the frontend dies before native emission. MIR is the actual
encoder input. O0 retains placement; O1–O3 share the bounded native policy.

| Owner | Final policy and conservative fallback | Work/growth bound |
| --- | --- | --- |
| Global placement | Unique scalar definitions reserve whole-function GPRs, including backedges and staged phi edges; effects/parameters/frame base excluded; EH/multiple definitions keep ordinary storage | O(I+V log V), O(V) scratch; ≤7 retained GPRs/5 preserved registers; no retries/clones |
| Control/frame/carry | Single branch compaction; exact frame/save facts; private complete-use reload windows; bulk addresses stay typed | O(MIR) with fixed ≤3×64 carry windows/home; zero cleanup growth |
| Builtin calls | Exact canonical identity, ABI/effect/noalias gates; page-safe strlen prefix with original-call fallback; unused-result memcpy REP after parallel argument capture | ≤8 prefixes/function, ≤128/unit; +53 bytes/site, ≤64 reservation; ≤512/8192 byte growth; linear argument work |
| Call scratch correction | Function-owned vectors reset logical state per call; bounded stable rotation preserves GPR/XMM order without per-call allocation | ≤14 register assignments; O(max stack arguments) retained scratch; dies with Selector; identical output |
| Explicit LowIR metadata | Hosted builtin identity persists independently of ELF spelling; malformed/conflicting metadata rejects | Constant fields/entity, linear adapters; no production text reconstruction |

PA32 fixed pass composition and finite inline/object/loop budgets remain in
force; native selection does not restart those passes. Analyses die at their
natural owner. No host/reference compiler, assembly transport, global mutable
cache or fixture recognition implements required output.

## Performance and acceptance

[Final frozen evidence](../student.tests/pa33/evidence220/performance.md) contains
**812 observations**: A/A plus six ABBA pairs per compiler/runtime lane, latency,
peak RSS, checked runtime and text size. All 14 fixed final benchmark objects
equal their accepted 219 outputs; the new scratch workload also has identical
A/B objects. [Historical verification](../student.tests/pa33/evidence220/history.json)
recomputes all **924** handoff observations, including rejected experiments.

Affected runtime B/A medians: loop **0.335**, calls **0.635**, short string
**0.595**, dynamic copy **0.029**; compile **1.007–1.015**, maximum peak RSS
increase **480 KiB**. Loop/call text shrinks; prefix growth is **6784 bytes/unit**,
below 8192. The 127-byte string retains a disclosed **1.140x** prefix-miss cost;
the rejected suffix alternative and its worse measurements remain archived.
Disassembly confirms removal of four loop loads/two stores per iteration.

Common O0 objects are unchanged; O2 text shrinks 10–25 bytes. Current common
timings are noisy and establish no additional runtime/compiler speed claim.
The fixed compiler component has unchanged 34466-byte text and peak RSS
77592/77820 KiB. The many-call scratch compiler ratio is 0.831 [0.719–1.065],
peak RSS 39848/39668 KiB, runtime 0.985 [0.847–1.082] with identical generated
code; no repeatable timing gain is claimed for the required lifetime correction.

Mandated MIR bounds, correctness, debug and finite work/growth policies remain
gates. Inherited self-imposed timing/RSS/zero-growth targets are diagnostics
under spec.md's stage-scoped rule; none adds a PA33 exit gate or excuses an
avoidable regression. Full self-hosting remains PA34; its applicable fixed
compiler-component benchmark is included here.

## Validation and ledger

- `make test-pa33`: **57/57**, plus **5** native controls and **18** driver modes.
- `make -C pa33 test-debuginfo`: **11/11**.
- `perl scripts/cppgm_file_audit.pl --stage pa33 --paths dev/src`: **pass**, four
  inherited header warnings.
- `make test-report-through-pa33`: **5454/5454**, **33/33 stages**. The actual
  primary log and independent reports agree; the prompt's 5840 census differs.
- Explicit source/template/ABI/builtin/replay, protected-page, cyclic-phi,
  budget and negative CLI controls pass. No new timeout occurred.
- `441d5ec9`, `f9df1b5b`, `cbf6f115`, `c1328ca0`: all stage handoffs reviewed;
  the 219 pending independent review is closed by this audit.
- `d7b7c9cf`: call scratch ownership and bounded stable ordering corrected;
  source traces, frozen benchmarks and final checks pass.

The final binding records source/artifact hashes, unchanged fixture trees,
reviewed code and all results. Verify the clean committed state with:
`python3 student.tests/pa33/audit220_records.py verify --clean`.
