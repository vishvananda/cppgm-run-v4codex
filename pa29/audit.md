# PA29 final audit194

Target: **PA29 full-stage**. Phase: **final audit complete**.
Stage base: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Previous reviewed code: `5aaf16d15f8e50925c0b75a5485893a958501b85`.
Entry: `bb864d43`, clean. Final reviewed implementation: **`60c3c356`**.
Previous turn classification: **progress**: implementation193 completed the last
course failure. No prior worker was live at entry. This audit independently
reconstructed the current pipeline; the checkpoint conclusions were context,
not substitutes for source review. [Audit190](audit190.md) preserves its full
record and earlier checkpoint ledger.

## Scope and Spec Alignment

Read `spec.md`, PA29's README, testing/reference policy, project layout, current
plan, stage history and implementation191–193 with their performance records.
The [unreviewed-range manifest](../student.tests/pa29/evidence194/range.json)
covers **14 commits and 85 implementation/build paths** since audit190 through
entry, including documentation commits and all three handoffs. The cumulative
stage changes 280 implementation/build paths over 137 pre-audit commits; the
whole-stage reconstruction below also inspects their shared inherited owners.
No PA30 work is included.

| Spec surface | Actual owner and independently checked flow | Disposition |
|---|---|---|
| §1 source/tokens/one parse | `preprocess/preprocessor.h`, `posttoken/cursor.*`, `syntax/cursor.*`, `syntax/parser.cpp`, `syntax/ast.h`, `lowering/driver.cpp`: TU source buffers outlive tokens; one PP lookahead, decoded literal scratch and a ring of unresolved parser lookahead; identifiers are interned. Parser calls semantic construction while building the one source graph. | No successive owning token streams, syntax-to-semantic graph copy or grammar replay in the traced production path. Source-region publication and occurrence/context IDs retain deferred grammar. |
| §§2/3 identity and lookup | `semantic/model.h`, `support/id_index.*`, `semantic/lookup.cpp`, `overload.cpp`, `call_selection.cpp`, `candidate_substitution.cpp`: canonical types/argument packs, numeric scope/name indexes, lexical/base/using/associated edges and compact candidate sequences. | Equality uses IDs, not rendering. Selected declarations and conversions are published. Expected query/substitution rejection returns compact facts; hard definition errors retain their diagnostic boundary. |
| §§4/5 template demand/caches | `template_call.cpp`, `template_instantiation.cpp`, `template_binding.cpp`, `template_type_facts.cpp`, `template_definition_environment.cpp`, `query_dependencies.cpp`, `declaration.cpp`, `member.cpp`: specialization key is canonical pattern plus interned arguments; nested patterns own enclosing frame identity. Frame keys include specialization, parameter range, parent and selected tuple. | Parsed non-dependent facts are shared; dependent occurrences overlay source nodes. Declaration/body/layout/default/exception/emission states are separate. Queues have monotonic cursors and per-owner states. Class completion invalidates only reverse query dependents and their local revisions; no global generation flush/retry. |
| §6 semantic-to-LowIR | `lowering/driver.cpp`, `values.cpp`, `initializer_plan.cpp`, `vector_values.cpp`, `floating_constants.cpp`, `symbols.cpp`, `local_abi.cpp`, `query_abi.cpp`: typed facts become builder operations, object identities, conversions and ABI nodes. | Production passes the same in-memory Program to native emission. Missing facts are errors; text adapters are explicit. Both discovered representation defects in those adapters are repaired below. |
| §7 preparation/native/ELF | `lowir/force_inline.cpp`, `native/extended_float.cpp`, `native/placement.cpp`, `native/driver.cpp`, `native/abi.h`, `native/float_encoding.cpp`, `toolchain/host_elf.cpp`: prepare once, select/allocate/encode per function, move native buffers into ELF sections, emit relocations/CFI directly. | Legality, bounds, fallback and actual homes/calls reviewed below. No assembler or serialized production handoff. |
| §8 storage/release | TU vectors/slabs, flat IdIndex tables, canonical type/query/constant pools; source nodes are shared by compact occurrence records. Arguments/candidate/projection scratch are local; old preparation pools die after replacement; MIR/selector state dies per function. | No new per-node owning allocation, process-global mutable cache, duplicated template tree or retained textual IR. Source/semantic state dies after that TU's lowering, before native selection. |
| §9 evidence/acceptance | Fixed source and executable benchmarks; final/historical measurements, A/A, ABBA, exact input/image hashes, phase/work counters, checked outputs. | All four performance dimensions retained. Apply stage-scoped acceptance and unchanged mandated budgets; see [performance194](performance194.md). |
| §10 self-containment | `toolchain/host_config.cpp`, `toolchain/driver.cpp`, build source sets and the above production owners; build-time captured headers/macros, shared builtin registries, in-process object writer. | No host/reference compiler supplies required output. A fresh hosted compile with PATH and CPPGM_HOST_CXX set to nonexistent locations produces identical object bytes. Host linking/runtime libraries remain the PA27/28 boundary. |

The four substantial-header file-audit warnings remain warnings; their inline
helpers do not imply individual allocation or extra production representations.
No new implementation source is added by this audit. The handoffs registered
`semantic/value_builtins`, `lowering/vector_values`, `support/extended_float`
and `native/extended_float` in their applicable tool source lists.

## Combined handoff review

- **191, vectors and representation builtins** (`66179d47`, `37d03290`,
  `f542b376`): followed registry/probe → typed operand grammar → canonical query
  and normal/fixed-template expression facts → list/constructor/global plans →
  lane operations or bounded counted loops → GNU/ext-vector ABI carriers. Checked
  boolean packing, scalar bit representations, signature identities, converted
  masks, constant-object keys and explicit ABI/LowIR adapters. Repeated personal
  controls exercise both host ABI directions, rejection and template lists.
- **192, extended formats** (`f3c93919`, `5369931a`, `dca6f1f2`): followed literal
  decoding and canonical format identity through constant evaluation, narrowing,
  static bytes, typed f16/f128, legalization, XMM/stack/variadic placement, helper
  calls, native encoding and external adapters. Rechecked negative-zero branch
  legalization from bit-initialized slots. Found the signaling and precision
  defects below in downstream consumers. The independent rational decoder passes
  1,642 cases, including long sticky tails and subnormal/tie boundaries.
- **193, effective member tags** (`60db24f6`): followed retained definition and
  signature indexes, prototype tag head, member-instantiation snapshot, explicit
  specialization reset, function-template inheritance and typed ABI name to ELF.
  Later definitions cannot rename already established members; overloads remain
  separate. Rechecked used-before-specialization rejection and nested definition
  ownership. The course-required tag suppression is an extension policy selected
  by the unchanged fixture, not an assertion that C++11 or GCC mandates it.
- All intervening plans/handoffs/performance commits were checked for remaining
  work and inherited gates. Their earlier five-failure ledger is fully closed by
  these implementations and the passing unchanged current contract. No handoff
  remains unaudited.

## Audit findings and repairs

1. **Signaling NaN identity lost at native/adaptor boundaries (spec §§2/6/7).**
   `Reader::literal` retains the signaling flag independently from its numeric
   carrier, but the new f16/f128 branches in `native::Operand::floating` returned
   before applying it. Static data and return literals therefore became quiet
   NaNs. The extended LowIR writer independently rendered `-snanQ` as `-nanQ`,
   and quad MIR omitted the distinction. `60c3c356` restores the quiet-bit rule
   and a nonzero payload, preserving sign; writer and MIR now retain signaling
   identity. The encoding owner serves both globals and instructions. The
   [expanded entry reducer](../student.tests/pa29/evidence194/entry-nan-expanded.json)
   reports **five failures**, while all final checks pass across half, float,
   double, x87 and quad, including negative/suffixed literals, static data,
   calls and source bit-casts. [LowIR Constants and Copies](../pa8/lowir.md#constants-and-copies)
   distinguishes `snan` and `nan`; extending the type set cannot discard that fact.
2. **Exact quad threshold changed by text rendering (spec §6 and PA29's common
   pipeline contract).** A quad classification operation can contain a literal
   carried exactly in x87 storage. Rendering that operand with x87 decimal
   `max_digits10` is insufficient when the reader rounds at binary128 precision.
   The minimum-normal threshold became slightly subnormal. The
   [reducer](../student.tests/pa29/source194/quad-classification.cpp) passes direct
   source emission but fails through the old adapter, as retained in
   [entry evidence](../student.tests/pa29/evidence194/entry-classification.json).
   The writer now emits exact binary128 hexadecimal text for f128 context,
   including legacy carriers. Max-subnormal and min-normal classification both
   pass direct and roundtripped output; all inspected code/data sections match.
   This is representation preservation, not a new floating approximation policy.

The repair changes three existing implementation files. There is no optimizer,
semantic lookup, exception policy or runtime-helper change. Ordinary output and
all earlier stages remain passing. No fixture, oracle, comparison or timeout is
changed by this audit.

Two exploratory checks are preserved with their dispositions. Complete ELF byte
identity was too strong for raw text normalization: the writer moves globals
before functions and consequently reorders the symbol table. The final check
compares every displayed data/code section, symbolic relocations and symbol set,
plus execution; it does not relax a course comparison. A first integration input
also used vector lane subscripting, which is rejected by the current compiler.
PA29 explicitly excludes runtime vector operations; handoff191 implements its
specified binary/type-operand builtin subset, not all vector language extensions.
The retained [boundary observation](../student.tests/pa29/evidence194/integration-boundary.json)
is not misreported as a new supported feature or an outstanding PA29 gate. The
final integration input uses the implemented conversion/reduction operations.

## Representative source-to-ELF and optimization traces

[Integrated source](../student.tests/pa29/source194/integrated.cpp) includes
`<cmath>`, queries implemented builtin capabilities, defines exact half/quad
constants, instantiates a dependently aligned class and an out-of-class nested
member with effective ABI tags, converts/bit-casts/reduces a vector, and calls a
nonthrowing force-inline quad function. A dormant member template stays dormant.

The parser retains `Box<N>` and its member definition once. The alignment and
member signature use typed parameter/query identities. `Box<32>` supplies one
substitution frame; signature matching publishes one source definition, applied
once to `Inner::apply`. Its conversions and call targets are recorded before
lowering. The selected empty ABI-tag set reaches the mangler, symbol and address
relocation; the dormant member has no emitted definition. Source constants
retain half/quad precision, globals store their complete bits, and native calls
consume ordinary signatures for the libgcc arithmetic ABI and C libm entry.

The trace reports four deferred regions/two demanded regions, eight fixed-fact
uses, one definition-signature check/application, 236 original instructions and
312 native instructions in two hosted live functions. The wider included header
creates 314 canonical specializations; that count is not confused with 314
emitted bodies. Statistics on/off objects are identical. [Controls194](../student.tests/pa29/evidence194/controls194.json)
retains original/canonical LowIR, MIR, symbols, disassembly, relocations, sections
and CFI observations. [Section comparison](../student.tests/pa29/evidence194/adapter-sections.json)
confirms exact executable-section equality for integration, NaN source and quad
classification across direct and adapted objects. Standalone MIR also shows
unreferenced `twice`; hosted object demand emits only main/apply. That boundary
is recorded rather than mislabeling the standalone view as hosted allocation.

The useful optimization fact is `always_inline` plus the callee's typed body,
call boundary and nonthrowing arithmetic. Target legalization creates ordinary
no-unwind helper calls while preserving source debug locations. The expander
uses canonical function identity, refuses recursion/variadic or unsupported
frame semantics, reserves a proven work bound **before mutation**, remaps
values/slots/CFG/PHIs and EH retirement, and keeps a valid call if admission
fails. Body-cost and return-region facts are scoped to immutable prepared bodies;
there is no unrelated cache invalidation or whole-program fixed-point rescan.
For this trace, one call uses 20 work units against 36 reserved. Native liveness
and call-clobber epochs preserve values across helpers; main's 304-byte and
apply's 176-byte frames plus 48-byte scratch areas expose the actual O0 costs.

Attribute-required expansion is not an optional runtime-profit claim. Extended
legalization is required target support, at most six operations per input; it
precedes inline budget accounting. Small vectors emit at most eight lane bodies,
then use one loop. No new transform enlarges those limits. The preparation and
per-function selection/encoding paths are linear or bounded by their documented
work limits. Text and runtime measurements accompany counts; later optimization
levels, broad hosted headers and inception remain owned by PA30–34.

## Reference preservation and proof review

[Coverage](../student.tests/pa29/evidence194/coverage.json) checks **403 source
inputs and 1,763 fixture/harness/manifest paths** unchanged since entry. The
only reference differences since the stage base remain the four exit-status
corrections and one Q-token line already documented by the earlier handoffs:

- [158 proof](reference-correction158.md): deleted-on-first-declaration copy
  functions are not user-provided; the empty class meets the C++11 trivial/POD
  rules, making the fixture's negated assertions false. Rechecked N3485
  [dcl.fct.def.default]/4 and [class.copy]/12 against the source and reducer.
- [189 proof](reference-corrections189.md): undefined primaries cannot supply
  demanded members, and the explicitly false primary makes its assertion false;
  rechecked [temp.inst]/1,5,7 and [dcl.dcl]/4. Reserved library names alone are not
  a portable diagnostic proof; spec §10's prohibition on synthesizing hidden
  library definitions is part of the course-contract proof. Positive supplied
  definitions preserve the original assertions and coverage.
- [192 proof](reference-corrections192.md): C++11 does not define Q literals;
  PA29's GNU extension contract does. The [GNU floating-types specification](https://gcc.gnu.org/onlinedocs/gcc/Floating-Types.html)
  assigns Q to `__float128`; the exact 2^-16382 binary128 encoding is 1<<112,
  distinct from the old x87 bytes. Rechecked the one-token/typed reducers and
  independent decoder. The [Clang ABI-tag description](https://clang.llvm.org/docs/ItaniumMangleAbiTags.html)
  does not settle the separate definition-selection discrepancy, so it supplies
  no reason to change the preserved ABI-tag oracle.

No additional correction is made. The reference bundle remains source revision
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, archive SHA-256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
Compiler agreement is corroboration, not the proof for those corrections.

## Final evidence and ledger

[Validation](../student.tests/pa29/evidence194/validation.json) on `60c3c356`:

- `make test-pa29`: **403/403**, exit 0.
- `make test-report-through-pa29`: **4941/4941**, **29/29 stages**, exit 0.
- Exact required file audit: exit 0, four inherited header warnings.
- Explicit controls191/192/193/194: **191/84/366/76 commands**, all pass;
  **81 properties**, plus **1,642 independent decoder cases**, pass.
- One additional [self-containment control](../student.tests/pa29/evidence194/self-containment.json)
  passes with no executable search path or host compiler available.

The user's entry summary reported 5104 tests, but its authoritative full primary
log and both fresh reports say 4941. Input/path hashes prove no coverage reduction;
this record uses the command's actual count. Old failing reports and preliminary
controls/measurements are retained separately. [Source binding](../student.tests/pa29/evidence194/source-binding.json)
pins all 482 tracked dev files and the tested compiler to the implementation
commit; the following consolidation commit changes documentation/evidence only.

[Performance194](performance194.md) reports 448 final observations/16 launchers,
eight identical equivalent image pairs, all four dimensions, scaling/budgets and
noise limits, plus preserved preliminary/historical evidence. It explicitly
identifies reused historical preliminary binary paths instead of falsely
claiming they still contain frozen bytes. There is no established avoidable
regression, optional-profit claim or unsupported blanket performance exit gate.

| Ledger item | Final disposition |
|---|---|
| Prior checkpoint findings 158–190 | Repairs remain present; passing through report and current shared-owner review preserve earlier behavior. Full history remains in audit190 and its linked predecessors. |
| Unaudited handoffs191–193 | All reviewed with their shared consumers and current controls. Five remaining course failures at checkpoint190 are closed without new reference changes. |
| Signaling identity; quad adapter precision | Both reproduced, repaired through owning consumers, and validated in `60c3c356`. |
| Architecture, allocation, self-containment, timeouts | Current source/trace and required suites reveal no remaining PA29 defect. Explicit tools remain adapters; no hidden hosted lowering path. |
| Performance | Stage-scoped acceptance satisfied; measurements, noise, necessary costs and mandated bounds preserved. |
| Required checks and coverage | Pass on the committed implementation; no skipped/discovered-test reduction. |
| Remaining PA29 work | None. No unaudited handoff remains. Later-stage exclusions are not asserted as implemented. |
| Commits | `60c3c356` contains the cohesive repair and controls; the following record commit consolidates this audit, compact plan and evidence. Final clean-status verification follows that commit. |
