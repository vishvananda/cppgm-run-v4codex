# PA34 inception

PA34 adds no feature or mode. Preserve the shared typed production pipeline,
all PA1–PA33 contracts, canonical generations and byte comparisons (spec §§1–10).
No additional performance gates: use stage-scoped acceptance, the handout's
900/3600-second compile limits and 8 GiB command RSS cap. Investigate divergent
self latency/RSS with frozen A/B, A/A and ABBA evidence before optimization.

## Remaining divergences and validation

- Fixed PA5 syntax/class_parser: elaborated class uses now reuse visible class
  bindings without hiding a callable name (`test_runner.cpp` sigaction call).
  `student.tests/pa5/check_elaborated.py`: six parser cases and runtime pass;
  C++11 [basic.lookup.elab] ¶1–3, [dcl.type.elab] ¶2. Explicit local forward
  declarations retain their scope. Original-source probe now compiles.
- Fixed PA29 builtin registry/semantic construction: `__builtin_trap` missing
  in retained queries for the hosted array header. It shares the typed abort
  operation permitted by the GNU contract; `student.tests/pa29/trap.md` records
  the proof. Probe/noexcept/arity and ordinary/template termination at O0/O3
  pass; the original PA2 number source probe compiles. The one stale probe
  reference token is corrected with contract proof and bundle revision there.
- Fixed PA7 promotion: narrow scoped enums were promoted to int in switches.
  Preserve their enum type ([conv.prom], [stmt.switch]); signed/unsigned runtime
  and invalid bool/integer-case/unary-plus controls pass at O0/O3.
- Fixed PA11 semantic scheduling: FactStore's local static requested synthetic
  constructor actions before completion. Queue each declaration's constant
  relocation demand until selected member actions finish; new demands rejoin
  the existing worklist. O0/O3 default-member, once-only effect and zero-init
  reducers pass; the compiler entry source probe now compiles.
- PA34 source correction: split the ill-formed multi-auto declaration in
  semantic/template_call.cpp (TypeId versus Type). The PA14 rejection reducer
  and [dcl.spec.auto]/7 proof document GCC's acceptance gap; no language rule
  or valid compiler construct was weakened.
- PA34 source correction: explicit TypeId/ArgumentId recovery from the wide
  PackExpansion bound slot in deduction_parameters/class_pattern_selection.
  Their aggregate-list conversions were ill-formed under [dcl.init.list]/3,7;
  GCC -Werror=narrowing confirms. PA11 id-narrowing.reject.cpp preserves rejection.
- Fixed PA29 extended-float native legalization: appended helper declarations
  now extend an existing function emission order. The seed's native crash was
  an out-of-bounds schedule read, exposed by constant.cpp/resolved_output.cpp.
  The typed order/validator control and direct/replay O0/O3 runtime checks pass.
- Fixed PA5 expression/type-id prediction: `word(*p)` is functional construction,
  not an abstract pointer declarator. A declarator-id disambiguates the operand
  under [expr.type.conv], [expr.cast] and [dcl.name]; functional-pointer.cpp
  passes AST and O0/O3 execution including pointer/reference cast controls.
  The original lowering/cleanup.cpp source probe now compiles.
- Fixed PA12 conditional semantics and lowering: preserve cv-qualified class
  prvalues in directional matches ([expr.cond]/3,6, [conv.lval]); use recorded
  construction/elision facts when predicting nested-arm result cleanup
  ([class.temporary]/3). The two native driver/dump source probes pass; PA12
  check_conditional.py covers cv overload selection and balanced nested
  lifetimes at O0/O3 with four runtime branches.
- Fixed build metadata: seed/self/inception now share the generated host-header
  filename and owning source from frontend_source_sets.mk. The PA34 rules used
  a stale preprocessor owner/name, so toolchain/host_config.cpp lacked its input.
  The generated contents match the seed configuration; host probes remain the
  PA25/PA29 build-time boundary, never compilation delegation (spec §10).
- Fixed PA29 complex libm registry/signatures: the selected `<complex>` header
  exposed missing library builtins. All 66 probes/signatures, wrong arities and
  O0/O3 three-precision runtime identities pass. The second PA29 gap, GNU complex
  brace initialization, now uses typed component list plans and packed constants;
  scalar/member lowering preserves the list conversion and checks narrowing per
  component. check_complex_braces.py covers constants, templates, references,
  arguments, arrays, effects and invalid lists. The original source probe passes.
- Fixed PA24 native/selection: an indexed i128 load overwrote r10 after its
  low word, then used that value as the high-word address. Preserve both memory
  operands' address carriers when choosing the chunk scratch. PA25 self crash
  traces to semantic/constant_integer.o; replacing only that object yields the
  seed's identical LowIR. PA24 indexed-wide reducer fails before and passes
  after at O0/O3 (three strides, loads/stores, incoming/spilled addresses).
- Continue each newly exposed self-build/test/object divergence at its earliest
  owner. Trace seed/self differences to object and source; probes are diagnostic.
- Required order: file audit; `make test-report-through-pa33`; canonical
  `make -C pa34 test-through-pa5 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++`;
  pptoken inception comparison; full `make inception CXX=g++ CPPGM_HOST_CXX=g++`.
  Continue the handout's self ladder through PA8/PA33 and audit source-to-ELF
  ownership, performance evidence and reproducibility before completion.
- Commit cohesive fixes and evidence, keep this ledger current, finish clean.

Current evidence (`$RALPH_ARTIFACT_DIR/pa34-221`): host through PA33 5454/5454
after the complex fixes; canonical self through PA5/PA8 passes. Self through
PA33 passed PA1–PA24, then exposed the PA25 i128 crash above. Fresh file audit,
host regression/debug checks and canonical rebuild follow that fix. Inception
remains unverified. Frozen first-complete seed/self binaries, original stack,
object disassembly, failing reducer MIR and corrected single-object diagnostic
are retained. Historical generated IR was gzip-compressed to reclaim 840 MiB;
its contents and measurement records are preserved.
