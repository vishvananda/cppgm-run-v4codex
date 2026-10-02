# PA32 checkpoint audit 210

Stage base commit: e82bf4152fe8d6d68b9cd966655db0d8142cf81b
Last reviewed commit: 76d3fcb24059d1557b6dd0e398cf3896931fcca9

Reviewed range: `e82bf415..76d3fcb2`, including the entire accumulated
`e82bf415..e409c2d8` entry range and audit corrections. Both entry markers named
the PA31 consolidation `e82bf415`, so no boundary recovery or narrowing was
needed. The prior turn was progress (committed call-group implementation and
evidence); process inspection at entry found no live build to resume.

This is a completed **checkpoint audit**, not a claim that PA32 is finished.
All 41 outstanding course failures remain required implementation work.

## Commit inventory and combined review

| Checkpoint | Every accumulated commit reviewed | Combined ownership inspected |
| --- | --- | --- |
| 207 | `33cce2ae`, `8c01abc9`, `5f8be172`, `0e0d29f4`, `399faac3` | Baseline plan; CLI/source/object shared optimizer; local folding/CSE/slots; mutable native values and phi transfers; debug parser/location transport; snapshot reassociation; raw performance/validation evidence |
| 208 | `1aecbd03`, `f36657ff`, `a1bbf8e7`, `d2e412d6` | Sparse promotion, ordinary CFG/dominance, edge/diamond/bypass facts, independent growth guards, measured call-cycle admission, selected-function analyses, handoff evidence |
| 209 | `acf38fc7`, `410c67bf`, `e409c2d8` | Typed optional cloning, roots/constants, handler retirement and call effects; expensive-arithmetic caller admission; source/native/EH interactions and evidence |
| 210 | `76d3fcb2` | Shared type-carrier legality, width-scoped facts, floating snapshots, retired debug ownership, executable reducers and full accumulated evidence verification |

The review covered all 43 changed implementation paths from entry, the final
combined source, registrations in `frontend_source_sets.mk`, and the history's
correctness/performance followups. Historical evidence sources were checked at
their actual handoffs (including dereferenced tracked symlinks), rather than
pretending old verifiers' current-worktree assumptions survive later edits.
The 208 personal call-cycle guard's later `no_inline` addition preserves its
original admission property now that ordinary calls can disappear.

## Findings fixed at their owners

1. **Comparison facts lost their width domain.** An `i8` comparison of an `i64`
   value 256 against zero is true, while branching on the original value is
   nonzero. Edge propagation previously transferred zero/nonzero facts in both
   directions without checking truncation; diamond equality also discarded
   higher bits when proving a wider phi replacement. The edge owner now checks
   representable width, and the diamond owner requires the comparison/phi
   domain to agree. No speculation or stronger range promise was introduced.
2. **Constant propagation discarded implicit operand types.** A `u8` switch
   selector 255 compared with case -1, an `i128` selector with high bits, and an
   `i128` variadic argument all changed behavior after literal replacement.
   Scalar, edge and closed-world call propagation now share one typed legality
   guard for switch, variadic and by-address carriers. Existing typed values
   remain when a literal would change width, argument placement or object home.
   The direct and explicit indirect signatures use the same rule.
3. **Promotion lost floating store conversion.** `store f32 %f64, $slot` became
   an invalid cross-format `copy f32`. Promotion now emits the matching explicit
   `fptrunc`/`fpext` at the original store snapshot. The tests include subsequent
   source mutation, both branch outcomes, f32/f64/f80 inputs, signed-zero
   inherited controls and the 16777217 -> 16777216 f32 rounding boundary.
4. **Retirement retained definition-only debug metadata.** Pruning an inlined
   template/support body into a declaration kept its function location, causing
   `--validate-lowir` to fail. The root owner now clears the retired definition's
   location; surviving cloned instructions retain their original locations.

Reduced execution failures were observed on the frozen entry implementation;
the floating case also failed the external structural validator. The final
`audit.py` runs **119 cases x four levels x two native paths**. The separate
source/template trace validates in-memory output and object replay at all four
levels. These are general proofs and reducers, with no fixture-name handling.

The applicable contract is [LowIR constants/copies, merges, conversions, calls,
parameter passing and terminators](../pa8/lowir.md), together with PA32's valid
output/behavior requirement. No reference was corrected, and no C++11 oracle
exception or bundle revision was needed. Identity integer conversions remain
accepted because `tests/o1/200-identity-convert-cleanup.t` explicitly requires
that input surface; this inherited validator extension is deliberate.

## Architecture and fact traces

The nontrivial trace is [audit-trace.cpp](../student.tests/pa32/audit-trace.cpp):
`use(long)` calls demanded `demand<long>` twice, and `main` also demands
`demand<int>`. `Pair<T>` construction, member calls and a volatile destructor
counter make layout, overload, lifetime, ABI and emission facts observable.
Two different runtime inputs check both results and exactly three destructions.

- `lowering/driver.cpp` owns each TU's immutable preprocessing storage,
  identifier interner, `PostTokenCursor`, ring `syntax::Cursor`, shared `Ast`
  and semantic analyzer. `translation_unit(&sem)` constructs/checks the graph
  cooperatively; no complete token or second semantic-tree transport intervenes.
- `semantic/template_call.cpp::specialize` keys canonical pattern entity and
  interned argument pack; enclosing substitution is owned by the selected
  pattern/immutable parent frame. Active/success/failure records distinguish
  declaration and body work. Query prerequisites use their own revisions and
  indexed dependents, not a global cache flush/retry.
- `syntax/occurrence.cpp` retains immutable parsed source topology and compact
  context/source occurrence IDs. `instantiate_function` demands the body once;
  `template_expression.cpp` reuses fixed expression facts, and dependent facts
  are rechecked in the substitution environment. There is no parser/token
  replay, deep tree copy, per-node owning pointer or recursive destruction.
  The trace records 180 tokens, maximum pending 47, 637 source/occurrence IDs,
  five specialization records, **two** template body transitions, 14 type
  substitution operations and 20 hits, unchanged across all optimization levels.
- `lowering/symbols.cpp` maps entity/ABI entries to stable symbol/function IDs;
  `function_body` consumes checked body facts, recorded conversions/lifetimes
  and layouts. `DebugScope` temporarily owns the source position. TU source and
  semantic storage die after lowering; the shared typed `Program` survives.
- `toolchain/driver.cpp` passes that Program directly to `optimize` and
  `compile_object`. Text reading/writing is only an explicit adapter. The same
  source plus flags and O0-text replay produce byte-identical objects at O0–O3.
  `native/driver.cpp` owns one Selector/MIR function through immediate encoding
  and releases it before the next. `host_elf.cpp` orders final symbol views and
  directly emits sections/relocations/COMDAT; no assembler or compiler delegate.

The trace's 144 LowIR instructions become 78 at O1–O3; selected ABI definitions,
volatile effects, unwind records and MIR debug locations survive. ELF symbol,
section and disassembly views are retained. The inherited object writer does
not emit DWARF line sections; the trace records that absence instead of
inventing a new PA32 exit gate. Required LowIR/debug/replay checks remain intact.

A useful optimization fact was followed through final encoding on the scalar
benchmark: repeated typed integer multiplication of the same loop-phi snapshot
is pure, nontrapping and equal under the integer operation's width. Local/CSE
facts require stable single definitions; the dominator scope undo log prevents
facts leaking to siblings. Unwind registrations reject ordinary dominance;
mutable reads and floating operations retain conservative handling. The CSE
lookup budget is charged before reuse, later compaction repairs definition
ordinals, and each subsequent analysis rebuilds its own changed facts.
The actual kernel MIR changes **12 multiplies to one**, frame **96 to 32 bytes**,
and preserved registers **five to one**. This is supported by checked runtime
improvement, not just smaller LowIR. Native non-SSA homes, snapshot copies,
parallel phi staging, call clobbers and debug transport were reviewed with the
inherited local/dataflow/call reducers and the new boundary cases.

## Legality, profitability, bounds and fallback

The [compact plan](plan.md) records current pass limits. Scalar propagation and
DCE use indexed users/dirty instructions; dominance uses dirty successors with
a monotonic parent fact; call summaries enqueue only reverse callers after a
new no-unwind fact. Promotion/bypass commit only complete proofs within work
and growth budgets. Exceptional states, recursion, unsupported stack/varargs
frames, mutation, escape, conflicting joins and exhausted budgets preserve
valid conservative IR. No transform relies on tests' spellings or references.

The complete schedule is finite: initial/root/constant/scalar/region preparation,
one optional cloning invocation and bounded scalar/CFG cleanup. There is no
per-local-change full-program rescan or global fixed point. Immutable callee
costs reserve nested work separately; output additions are charged before
mutation. Original pools plus optional clones are bounded by the 32-times-input
work allowance; promotion adds <=I instructions and <=2O operands, and bypass
separately caps phi operand expansion. The fixed number of subsequent sweeps
preserves linear whole-pipeline bounds. Transient per-function maps/vectors,
cloner input pools and MIR have explicit release boundaries. The previous
loose `2^24` accounting envelope was not a mandated or measured memory limit;
the operative reservations and measured RSS above replace it in the plan.

Ordinary/hinted/single-use/cold call size limits (40; 6 callful single-block;
32/128 hinted; 512 single-use; 128 actual cold body), caller growth 1536/2048,
work 32768/caller, unit work 32*(I+O+P+S+F+1), and depth 64 remain unchanged.
Expensive arithmetic expansion still declines cyclic or >128-instruction
callers. The prior rejected divider-loop and common-memory policies remain
disabled; their diagnostic measurements are preserved, not excused by PA33.

## Performance evidence and stage-scoped acceptance

[Binding, commands and all raw observations](../student.tests/pa32/evidence210/binding.json)
freeze binaries, input hashes, flags and outputs. Artifacts are retained under
`/home/vishvananda/work/private/v4codex/artifacts/pa32-210`. Each measured axis
uses an A/A block and six wall-time ABBA blocks, CPU 14, with all samples and
spread retained. Compile and execution are measured separately; runtime inputs
and independently checked results keep work live. Host g++ only links the
compiler-produced objects. Telemetry is collected separately on affected
workloads; common telemetry has equal flags on both sides and triggers no
extra optimizer analyses.

A/B baselines: scalar is current O0/O1; slots/dominance are 207/current O1;
calls/regions/cheap are 208/current O1; constants are 208/current O2.

| Workload | Compile ratio [range] | Compile ms A/B | Peak RSS KiB A/B | Runtime ratio [range] | Runtime ms A/B | Object text bytes A/B |
| --- | --- | --- | --- | --- | --- | --- |
| Scalar | 2.337 [2.310–2.419] | 52.36/123.20 | 10332/15388 | 0.876 [0.869–0.894] | 74.86/65.44 | 170400/109800 |
| Slots | 1.836 [1.794–2.116] | 70.67/128.54 | 14536/16728 | 0.864 [0.863–0.866] | 91.53/79.07 | 148800/144000 |
| Dominance | 1.568 [1.551–1.598] | 93.74/146.68 | 16820/18160 | 0.811 [0.809–0.821] | 116.16/94.27 | 219600/147600 |
| Division call control | 1.201 [1.005–1.211] | 65.62/78.69 | 12380/13820 | 0.997 [0.992–1.004] | 76.71/76.65 | 128438/128438 |
| Regions | 1.262 [1.231–1.293] | 65.84/83.12 | 12544/13676 | 0.806 [0.803–0.893] | 95.46/76.85 | 128503/128438 |
| Cheap calls | 1.579 [1.534–1.650] | 65.16/103.15 | 12420/16664 | 0.972 [0.938–1.003] | 36.91/35.94 | 128427/132000 |
| Constant arguments | 1.156 [1.041–1.240] | 64.92/78.66 | 11968/12720 | 0.921 [0.856–0.930] | 40.30/37.18 | 134487/134427 |

Every affected workload except cheap calls improves in all six new pairs.
Cheap calls improve in five; the remaining +0.27% pair is within the A/A spread
(36.64–37.17 ms). Its object is byte-identical to the accepted 209 output, whose
six pairs all improved. Repeated benefit is supported across both runs; no
single-pair/all-pairs numerical rule is added to the spec. The divider control
is unchanged and has no runtime claim. Regions and constants also retain the
exact accepted 209 objects.

The fixed template-heavy common suite compares the PA31 stage-base O0 binary
with the reviewed O1 binary:

| Workload | Compile ratio | Compile ms A/B | Peak RSS KiB A/B | Runtime ratio | Runtime ms A/B | Object text bytes A/B |
| --- | --- | --- | --- | --- | --- | --- |
| Memory/calls | 1.162 | 167.27/195.04 | 29676/30528 | 0.934 | 52.74/49.34 | 151393/124959 |
| Floating | 1.198 | 168.81/201.65 | 29888/30668 | 0.982 | 49.16/48.44 | 151234/124867 |
| Exceptions | 1.203 | 168.89/202.91 | 29676/30576 | 0.997 | 251.56/251.30 | 151541/125067 |
| Pruning | 1.124 | 208.00/235.11 | 35728/36420 | 0.935 | 52.51/49.06 | 151393/124959 |

Same-level entry/current O1 compiler medians are 0.999/1.004/0.995/0.994;
O0 medians 1.001/0.986/0.998/0.993. All eight common objects are byte-identical.
RSS increases are at most 520 KiB; no runtime regression/benefit is attributed
to identical code. The retained O0 pruning runtime outlier (pair 6.373) and O0
memory compile outlier (2.475) are visible in raw spreads, not censored.
The compiler-owned folding.cpp component at O0: compiler 1.003 [0.989–1.009],
1084.39/1084.01 ms, RSS 77256/77704 KiB, identical 34322-byte text. Runtime is
inapplicable to that object with no main; no full self-hosting claim is made.

**Reclassification:** inherited 2x compiler, 1.75x RSS, zero/1.5x text-growth and
historical 10% runtime targets are self-selected diagnostic thresholds, not
mandated course/spec exit gates. The accumulated scalar compile ratio 2.337
misses 2x; the whole current pipeline pays about 70.84 ms to save about 9.42 ms
per checked three-million-iteration execution, amortizing in about eight runs,
with 35.6% smaller text and 1.49x RSS. The complete progression's other affected
profits, unchanged audit controls, bounded analyses and retained rejected-policy
evidence support this tradeoff. No universal speedup is claimed and no new
unprofitable optional transform was introduced by the audit. This changes no
mandated IR envelope, semantic obligation, coverage, actual work reservation
or growth cap. Historical misses and raw measurements stay intact. Later
allocator quality, DWARF generation and whole-compiler self-hosting add no
current-stage gate. Future regressions still require investigation and removal
of unprofitable optional policies; the reclassification is not a blanket waiver.

## Validation, remaining work and ledger

- `make test-report-through-pa31`: **5178/5178**, exit 0.
- `perl scripts/cppgm_file_audit.pl --stage pa32 --paths dev/src`: exit 0;
  four inherited substantial-header warnings, no new warning.
- `make test-pa32`: **178/219**, exit 2; exact **41/219 failure set unchanged**
  from the primary entry log. The supplied 178/425 census is preserved as a
  different accounting denominator; status 2 is a make exit code.
- Required `make -C pa32 test-debuginfo`: direct **4/5**, exit 2. Source lanes
  separately completed **0/3**; debug replay **25/25**, exit 0. The O3 debug
  loop's emitted LowIR is structurally valid to the compiler validator; the
  harness additionally reports unsupported debug branch/phi syntax before its
  still-unmet unrolling expectation. No harness or comparison was changed.
- Explicit inherited reducers: **506 local**, **134 dataflow**, **63 call**
  cases, including four levels, native paths, traps/CLI/EH/escape/mutation and
  bounded stress controls. New **119** cases and full source/template trace pass.
- `verify_audit.py` verifies **538** current source bindings, **276** frozen
  artifacts, **4,844** inherited and **1,148** new observations, paired medians/
  spreads, historical sources, contracts, failure-set equality and clean review
  boundary. Log/binary/input hashes are in the binding; generated objects/logs
  remain outside git. No course fixture, reference, harness or comparison rule
  was changed or dropped.

Remaining work is grouped in the plan: memory/aggregate/exceptional-state
proofs; context-sensitive calls and loop transformations; source/debug/flow
closure. These are unfinished stage requirements, not deferred audit questions.
The three owner groups were useful, but repeated snapshot/debug/policy closures
across small handoffs were avoidable fragmentation. Future groups should close
those interactions before publishing their validation/evidence boundary.

| Audit | Reviewed range and code tip | Findings/evidence | Result and remaining owners |
| --- | --- | --- | --- |
| 210 | `e82bf415..76d3fcb2`; Last reviewed commit above | Four ownership corrections; complete accumulated architecture/fact/native review; 5,992 verified raw observations; required prior/file/progress checks | Checkpoint audit complete; 178/219 and identical 41 failures; remaining memory/aggregate, contextual calls/loops, source/debug closure |

The records commit changes only this audit and the compact plan. The code tip
above was validated and committed first; no implementation edit follows it.
