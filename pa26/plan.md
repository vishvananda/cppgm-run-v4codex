# PA26 implementation handoff

Stage base commit: 369c57f19fa13c0394d4fb0345cf2bb79708fc67
Last reviewed commit: 369c57f19fa13c0394d4fb0345cf2bb79708fc67

Target: **PA26 full-stage**. Phase: **implementation143 handoff**.
Turn entry: `9063d5260c65bf6b1984ebae6028fef44251910a`, **29/30**.
Implementation boundary: `a624085e1f3823c590da657c2d17f596a301215e`.
Final: **30/30**; PA1–25 **4253/4253**; through PA26 **4283/4283**;
file audit passes (four inherited header-division warnings).
No contract fixture, reference, harness, comparison rule or coverage changed.

## Design/spec alignment and completed groups

| Owner | Data flow / complexity | Validation |
|---|---|---|
| Streaming preprocessor / host configuration | Build-host metadata supplies target roots/predefines; user overrides retain precedence. Include-next carries the found-directory index; physical-root deduplication is O(d log d), once per TU. No host compilation of user input or library replacement. | Ten header controls, real installed `<string>`, include-next siblings/duplicate roots, labels and block linkage. |
| Parser / canonical semantic graph | Traits, typeof, intrinsic queries and dependent enum/auto facts use typed queries/substitution. Scalar varargs, stack allocation and address operations reuse existing ABI facts. Integer packs are linear output, capped at 1048576. | Twenty-two trait/intrinsic controls, invalid cases, cross-host va_list, all string conditional paths and heap-backed copies. |
| Semantic demand / lifetime lowering | Selected boundaries enqueue exception facts once, after evaluated bodies and before lowering table allocation. Constructor cleanup and ordinary destruction consume the same completed effect fact. No late reconstruction of callees. | Prototype/temporary noexcept, auto decltype, partial-construction unwind, source temporary lifetimes; prior suites pass. |
| Native EH / ELF | Typed regions -> per-function frame/selector facts -> sparse LSDA, CFI, one physical resume terminal and local terminate action. GOT function-address facts use one symbol-identity pass; private output allocates no host GOT table. Work/storage is linear in emitted CFG, instructions, clauses and relocations. | All PA26 object facts and runtime cases, nine host ABI controls, DSO function-pointer identity/PIE control, production template LowIR/MIR/ELF trace. |

The unchanged final fixture is compiled through the real headers and this
compiler's own frontend, lowering, backend and ELF writer; the host performs
only the handout's final link. General hosted compatibility remains a later
stage surface, as PA26 explicitly states. Expression-form constexpr-if is a
header-required GNU extension; its condition-declaration form and aggregate
va_arg remain unsupported extensions/surfaces, not claims of full library support.

## Remaining implementation and independent audit

- **Known unfinished command-line requirement, not waived:** default `-c -o x.obj`
  retains PA25 private output; other names produce host ELF, with explicit
  `--object-format=elf|private` overrides. PA26's arbitrary-output-name wording
  requires a uniform policy. This is a distinct driver/importer/runtime boundary:
  PA25's compile/direct/mixed-link path consumes private EH/TLS/runtime objects.
  Resolving the policy must preserve those earlier contracts; it is not another
  local fix to the completed header/EH/relocation group. The 30/30 fixture result
  does not establish that untested naming requirement.
- **Scope correction to inherited plan:** integration of the host object path
  into a private linker/runtime is explicitly out of scope in PA26's README.
  It is not an additional PA26 performance or completion gate. This does not
  remove the separate uniform-output requirement above.
- **Independent audit questions remain pending:** whole-stage source-to-ELF
  identity/lifetime ownership, demand precision, CFI/LSDA legality, exception
  spill ownership, extension semantics, output policy and performance evidence.
  These are review questions, distinct from the known implementation gap.
  Review markers above remain unchanged; this handoff does not certify PA26.

## Performance and handoff ledger

[Performance143](../student.tests/pa26/performance143.md) records frozen entry/final
A/A + ABBA comparisons on common correct inputs and a final/final string baseline.
Common objects/executables are byte-identical; compiler medians rise about 1%,
RSS under 2%. Short-TU batches expose a 22.3% increase (about 1.04 ms/TU) for
required host setup; direct compiler RSS is 6752/6932 KiB. The string baseline
compiles in about 666 ms at 45 MiB RSS, runs in about 535 ms, and emits 35963
executable text bytes. All samples/outliers and prior142 measurements survive.
No optional transform or speedup claim. Inherited 15%/zero-growth targets are
diagnostics under spec section 9, not new gates; mandatory limits remain intact.

[Validation143](../student.tests/pa26/evidence143/validation.json) records required
commands, zero failures, inventories and unchanged contract hashes. Personal
controls and the typed inspection ran explicitly.

| Handoff | Disposition |
|---|---|
| implementation142 | Host EH group; original stage 0/30 -> 29/30. Historical measurements retained. |
| implementation143 | Related header, semantic-demand, cleanup and PIE groups committed; turn 29/30 -> 30/30. Earlier suites and file audit pass. Implementation handoff complete; return to Ralph for full audit. |
| Independent audit | Pending; uniform-output implementation gap is recorded separately above, not disguised as an audit question or waived by passing tests. |
