# PA33 final independent audit evidence 220

The [audit](../../../pa33/audit.md) reconstructs the stage from its actual
producers, consumers and encoded output. The [plan](../../../pa33/plan.md)
contains final policy, acceptance and the closed handoff ledger.

Production correction `d7b7c9cf` replaces per-call scratch/partition allocation
with function-owned capacity and a bounded stable ordering step. No fixtures,
reference outputs, comparison rules or default suite selection changed.

- `performance.md` summarizes all 812 final frozen observations, with separate
  compiler/runtime ABBA and A/A calibration. The five JSON files preserve every
  sample, flags, input/binary hashes, checked status, RSS, text and spread.
- `history.json` records independent reconstruction of 924 historical samples,
  547 committed source bindings and 184 artifact hashes from 219. Rejected
  suffix measurements and the earlier restricted-CPU timeout remain archived.
- `checks.json` binds final required commands and full logs. All 33 stages
  pass. The actual report count is 5454/5454, also present in the supplied
  primary log; the prompt census of 5840 is not substituted for command output.
- `binding.json` freezes current implementation, audit, plan, personal controls,
  binaries, generated source/LowIR/MIR/objects, full logs and inspection data.
  It verifies 14 accepted fixed benchmark outputs remain byte-identical after
  the scratch refactor, plus identical A/B output for the new scratch workload.

Full artifacts live under `$RALPH_ARTIFACT_DIR/pa33-220`. The audit source trace
includes interleaved GPR/XMM/stack call arguments, templates with undemanded
invalid bodies, destruction, builtin identity and meaningful debug locations.
It checks four optimization levels, twelve runtime inputs, direct/replay and
entry/final ELF equality, combined/single MIR/executable identity and CLI
failures. `native-inspection.json` retains loop/call/builtin disassembly.

Reproduce using `audit220.py OUT`, `audit220_history.py OUT_JSON`, and
`audit220_performance.py ARTIFACT_DIR` in `student.tests/pa33`. Performance
expects immutable final binaries in `ARTIFACT_DIR/final` and the frozen 219
entry/final binaries. Run measurements without concurrent builds/tests. The
final verifier is `python3 student.tests/pa33/audit220_records.py verify --clean`.

Storage maintenance compressed 72 old PA10/11 scratch outputs losslessly,
verified their decompressed hashes and reclaimed 503795834 bytes; the manifest
is bound. No measurements or current frozen artifacts were deleted.
