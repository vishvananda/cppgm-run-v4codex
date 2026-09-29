# PA19 final independent audit — loop 93

**Spec Alignment: aligned for PA19/O0. Full-stage audit complete.** Reviewed
`e5f4c3ed78972c8d161671d145bf525cb99033f4` through `228d7fc7`, including both
previously unaudited implementation handoffs. The working tree was clean at
entry and no validation or benchmark process remained live. The previous
implementation turn made progress: it established the completed variable-fact
path, corrected proved oracles and produced reproducible validation evidence.
This audit independently read the current spec, handout, contracts, commits,
implementation and evidence; checkpoint conclusions were not its substitute.

No additional compiler defect was found in the reviewed PA19 ownership paths.
Compiler sources and output contracts remain unchanged. This commit adds
cross-handoff controls, a reproducible architecture trace, a full-stage frozen
performance comparison, evidence verification and the consolidated plan/audit.
It does not advance the implementation to PA20.

## Scope and architecture reconstructed from source

PA19 composes the PA14–18 template features and emits **unoptimized LowIR**.
PA8 defines the typed/output contract; PA16 owns constant objects and automatic
scalar-array images. Native generation, optimized LowIR and native debug output
are explicitly outside the PA19 handout. The source-to-ELF audit therefore
traces compiler-owned production to LowIR and then executes/inspects that exact
output using the course's supplied native backend, separately as validation.

| Spec requirement | Actual owner and review conclusion |
|---|---|
| §1 immutable source, streaming tokens | `preprocess/source.h`, preprocessor/macro cursors, `posttoken/cursor.*`, `syntax/cursor.*`: immutable source bytes, file/offset identities, interned identifier IDs and a geometric lookahead ring. Macro bodies/arguments and unresolved grammatical prefixes are the retained tokens. There is no sequence of complete owning token streams. Delimiter/angle annotations skip already inspected spans. |
| §1 one parse, integrated semantics | `Parser::translation_unit` calls `Analyzer::consume` after each parsed declaration. The same `syntax::Ast` supplies structured names and source locations to semantic facts. `NodePool` keeps source nodes once; specialization occurrences are eight-byte source/context pairs. `Ast::source_region/instantiate` projects demanded regions and defers member bodies/defaults; it does not invoke the parser. PA5's documented lexical hints are subordinate to declared categories, not fixture recognition. |
| §§2,8 canonical graph and storage | `semantic/model.cpp`, `template_arguments.cpp`, `template_call.cpp`, `type_query.cpp`, `support/id_index.cpp`: compact entity/type/query/argument identities; half-full flat hash tables with geometric growth. Keys use typed components and argument slices, never rendered names. `FactStore` allocates slabs; `ExpressionStore` shares fixed properties with separate use/object/conversion state. Relationships are IDs, not owning per-node pointers or recursive ownership trees. |
| §3 lookup/overloads | `lookup.cpp`, `overload.cpp`, `template_deduction.cpp`, `template_ordering.cpp`: indexes by scope/name/kind, explicit parent/using/base/ADL edges, shape/arity filtering, then required candidate substitution and conversions. Ordering caches include original signature identities, argument count, operator/member context and conversion-ordering mode. Ordinary candidate failure returns zero or a structured failed fact. Selected-body/class-definition errors remain hard errors. |
| §4 demand and environments | `template_type_facts.cpp`, `template_lexical_frame.cpp`, `template_definition_environment.cpp`: immutable parent-linked frames keyed by specialization, parameter slice, parent and arguments; overlays contain only local substitutions. Concrete member-template identity owns its enclosing frame. Specialization keys are canonical pattern/argument pairs; explicit extensible pack prefixes have a distinct index. Nondependent types/queries and expression properties are reused. |
| §§4–5 facts and scheduling | Declaration, definition, class layout, defaults, exception checks, member bodies, constants and emission have separate state records. `template_instantiation.cpp` and `template_variable.cpp` observe active/success/failure states. `Analyzer::finish` drains monotonic cursors for targeted demands; `deferred_function_uses.cpp` deduplicates owner/target edges. `query_dependencies.cpp` invalidates only reverse consumers of the completed class/query and increments local revisions. Incomplete failures retain prerequisites rather than becoming permanent negatives. No global generation flush or retry of every specialization was found. |
| §6 direct lowering | `lowering/driver.cpp` directly constructs `Procedural` from AST/semantic facts, then writes the typed `lowir_model::Program` once. Function/initializer/ABI identities select one emitted definition per entry; field layouts, calls, conversions, object destinations and cleanup actions are consumed by ID. The few full declaration traversals reserve symbols/final boundaries once; they are not retry loops. Required missing facts throw invariant errors. `--validate-lowir` is an explicit audit option. |
| §§7,9 bounded work | PA19 adds no optimization pass. Argument/pack composition visits the actual lists; specialization work is charged to demanded facts, projected occurrences and query edges. Required overload comparisons may inspect the applicable candidate pairs, not unrelated declarations. Existing local simplifications have explicit proofs and bounds discussed below. No new fixed point, inlining, unrolling or growth policy was introduced. |
| §8 lifetime boundaries | `Preprocessor`, cursor, AST, analyzer and lowering adapter are stack-owned per translation unit in `emit_lowir`; their pools and transient vectors release at TU exit. Function builders and lifetime scratch reset at each function. The typed LowIR program and compact linkage/ABI graph survive through final serialization because LowIR is this stage's required output. No duplicate textual IR is retained/reparsed, and no process-global accumulated semantic cache exists. |
| §10 self-containment | Source review of production entry/lowering and subprocess APIs, plus an `execve/openat` trace: one compiler execution; reads are its source and host runtime/timezone files; no reference, prior compiler or cached output is opened. The supplied backend is invoked only by the independent test scripts. |

## Representative end-to-end data flow

[audit93_trace.cpp](../student.tests/pa19/audit93_trace.cpp) combines ordinary
class layout, a member alias forwarded as a template-template argument,
defaulted argument-pack deduction, lazy member variable values/storage,
constant-array copying and an effectful receiver around a constant conversion.
[The verifier](../student.tests/pa19/audit93_verify.py) retains its LowIR,
telemetry, hashes, native exit and optional syscall/disassembly evidence in
[audit93-evidence.json](../student.tests/pa19/audit93-evidence.json).

1. `Pair<T>` is parsed once. The binder retains the source field/constructor/sum
   recipes. Applying `Source<int>::Rebind` preserves the **alias declaration** as
   a template-template argument; `build` then applies it to `short`. The alias
   result and ordinary `Pair<short>` share canonical class identity. Completing
   that demanded class establishes fields at offsets 0 and 8, size 16/alignment
   8. The constructor and `sum` bodies are demanded independently.
2. `Source<T>::width<U>` keeps its initializer QueryId at class declaration.
   The outer and inner parameter frames compose when the value is required.
   `width<long>` for `T=short` is 10 and for `T=int` is 12. Value demand checks
   the ordinary initialization conversion and persistent constant; address use
   separately requests storage. Two addresses of the same specialization are
   equal. There are exactly **two initializer transitions**; the invalid
   `dormant<U>` body is neither instantiated nor emitted.
3. `count(Tuple<>())` sees the complete argument tuple `{int,long,char}`, not
   just the empty written list. Deduction binds `A=int`, `B={long,char}`.
   The ABI argument-pack metadata and returned constant both describe that
   same two-element tail. No type spelling is parsed to recover it.
4. LowIR uses `obj<16x8>`, recorded `i16`/`i64` field loads, a sign extension,
   typed constructor/build calls and normal class return storage. `local_abi.cpp`
   walks typed names/argument records into the PA9 encoding graph. Its mangling
   strings are output metadata, not semantic keys.
5. The readonly array image contains `{1,2,3}` once, with **two** `copyobj 12x4`
   operations into distinct local slots. Mutation of one array does not affect
   the other. `Constant<7>` retains constructor/destructor effects (1 + 2)
   while its already-proved scalar conversion supplies 7.
6. Instrumented and ordinary output are byte-identical. The trace has 612
   parsed nodes, 497 compact occurrences, seven inherited expression facts,
   one conversion-result inspection, one constant-data record and one data
   reuse. It emits 142 instructions/215 operands and returns zero through the
   supplied backend. The sectionless ELF payload is 1,616 bytes, including data.
   Inspection shows two 8+4-byte array copies from the same readonly address
   into different frame offsets, a store of immediate 7, and retained lifecycle
   calls. The ordinary O0 frame/exception scaffolding is visible; this audit
   claims no improvement to that backend's spills, allocation or encoding.

## Optimization legality, profitability and budgets

`semantic/conversion_result.cpp` accepts only a requested, completed, nonvirtual
scalar conversion with a single return of an established nonvolatile named
constant and a standard result conversion. At most eight parenthesis wrappers
are inspected. `lowering/user_conversions.cpp` still evaluates the receiver,
performs the second conversion and preserves temporaries/cleanup and exception
actions. The canonical completed function owns the immutable proof; the local
query-completion dependency machinery owns any earlier incomplete query state.
There is no mutable global analysis to invalidate. Unknown/effectful/volatile/
virtual cases retain their ordinary call. New independent controls exercise
both successful forwarding with receiver effects and the effectful/volatile
fallbacks. Existing profitability evidence in PA18 audits 82/86 is retained;
no new runtime-profit claim is inferred from the smaller trace IR.

Automatic nonvolatile scalar-array images are a **PA16 contract requirement**.
`prepare_constant_array` validates the complete initializer plan, excludes
volatile/class-array cases from the automatic-copy rule, and preserves each
destination's identity/lifetime. `initialize_constant_array` hashes typed bytes,
relocations, extent and alignment; it checks equality before sharing data.
Work is proportional to the checked plan/emitted data and copy sites. Sparse
omitted zero ranges stay compressed until output. Unproved initializers keep
ordinary initialization. Data reuse cannot add code growth.

`fixed_layout_operand/reuse_value_conversions` preserve dependent-layout
provenance for O0 conversion presentation. Each source has at most four
two-operand flag variants. Signed/unsigned values use the ordinary conversion
rules; the independent negative-NTTP/sizeof controls execute both operand
orders. This is no new arithmetic optimization or runtime speedup.

Pipeline accounting is the sum of source processing, demanded semantic facts,
candidate/edge work and emitted LowIR/data. No optional transformation restarts
the pipeline or duplicates a function body. Constant execution separately caps
each top-level demand at 1,000,000 steps and depth 512, memoizes completed
activations, and does not cache resource-limited attempts as permanent semantic
failure. Ordinary optional constant probes fall back conservatively; required
constant-expression failures diagnose. PA19 adds no search/growth allowance.
MIR, allocator, direct ELF and native debug obligations remain with their owning
later assignments; the PA19 boundary is not a waiver of an available surface.

## Findings, reference review and ledger

| Handoff / commits | Independent disposition |
|---|---|
| 91: `1747113b`, `dbe5e97c`, `57e27df2`, `4dd6e737`, `065d6783` | Reviewed function-type/value classification, friend/elaborated-tag lookup, alias declaration identity through substitution, comma-optional ellipsis, defaulted argument packs, empty-tail ordering, ADL template-id recognition and reference-cast identity. Declaration-order, ambiguity, const/access and cross-feature controls pass. No outstanding ownership defect found. |
| 92: `42251d95`, `ea5c1d82`, `5763cf6c`, `946c651b`, `228d7fc7` | Reviewed declaration/value/storage separation for member variable templates, source/enclosing frame composition, partial selection, ordinary initialization conversion, persistent class values, SFINAE failure boundaries, class lookahead and O0 layout provenance. Dormant, repeated-address, recursion, invalid/deleted/explicit/access and signed-value controls pass. |
| Reference corrections 91/92 | All **15** altered oracles reconstructed from original oracles and contract/source facts, never student output. Read the cited N3485 and PA16/LowIR rules and ran the reduced controls. The pinned bundle/revision and original/corrected hashes are documented. No additional oracle correction was needed. |
| Audit 93 | Added 16 independent cross-handoff controls and the combined trace; preserved all previous controls/evidence. Added whole-stage A/B evidence and reproducible validation/coverage records. A verifier initially assumed both reference manifests had the same schema; that audit-tool issue was corrected and the complete verifier passes. No compiler mutation or source-set change was needed. |

The [pack proof](reference-correction91.md) follows N3485 [temp.arg]/4,
[temp.deduct.type]/9 and [expr.sizeof]/5: omitted defaults are real arguments,
and the trailing pack includes them. Its native reducer passes. The original
course fixture intentionally remains a comparison against 2 and thus its
correct native result would be 1; compilation/LowIR remain the graded contract.

The [fourteen corrections](reference-correction92.md) distinguish five required
PA16 readonly-array copies, five undemanded static definitions, discarded
reference consumption, two constant-initialization cases and no-effect explicit
instantiation after specialization. [temp.inst]/1,2,8,10 separates declarations
from definitions; [expr.sizeof]/1 and [basic.def.odr]/2 explain lack of demand;
[expr]/11 forbids the extra discarded-reference load; [basic.start.init]/2
requires early constant initialization; [temp.explicit]/5 gives the no-effect
rule. Variable-template syntax is the inherited course extension, not a claim
that C++11 standardizes it. The reducers use defined behavior rather than the
null-derived reference in one original fixture. All 423 source inputs and
exit-status sidecars, structural validation and comparison rules are unchanged
from the stage base. Only the 15 documented `.ref` files differ.

**Unaudited handoffs: none through `228d7fc7`.** No remaining PA19 implementation,
correctness, timeout, self-containment or architecture finding is open. Later
stage boundaries are recorded above and are not unfinished PA19 work.

## Performance and validation

[Final performance evidence](final-audit-performance.md) reports compiler
latency/RSS, runtime and code-size measures together, with fixed hashes,
source/output checks, A/A calibration, four ABBA blocks and all spreads.
The new full-stage run has nine workloads, 324 observations and 34 warmups.
Seven equivalent comparisons have identical output; two measure newly supported
behavior only. Compiler text grows 0.298% across the stage. The largest
namespace-variable paired latency is +2.8%, with one required extra conversion
check per value. Runtime loop ratios center near 1.00 with identical bytes.
No avoidable regression or unprofitable new transform was found. Historical
+15%, +16 MiB and 5.5× targets remain diagnostics under spec §9; no mandated
limit or coverage requirement is weakened. Earlier measurements remain intact.

[Validation record](../student.tests/pa19/audit93-validation.json):

- `make test-pa19`: **423/423**, exit 0.
- `make test-report-through-pa19`: **3452/3452**, **19/19 stages**, exit 0,
  plus **22** focused properties reported separately by the owning harnesses.
  These are the direct report counts rather than a wrapper's combined total.
- `perl scripts/cppgm_file_audit.pl --stage pa19 --paths dev/src`: **pass**,
  exit 0. Three inherited header-division advisories remain (`procedural.h`,
  `analyzer.h`, `model.h`); they are not errors. Their shared records/accessors
  and substantive `.cpp` ownership were inspected, not treated as test failures.
- Personal verifier: **100/100 controls** (79 native, 21 required rejections),
  plus the native architecture trace and defaulted-pack reducer. All 15 oracle
  reconstructions and unchanged coverage/comparison checks pass.
- Optional syscall trace proves a single compiler execution and no reference
  reads for the representative source. Telemetry/validation output parity passes.
- Source/binary hashes bind these results to the reviewed implementation;
  audit-only artifacts do not change it. All intended audit files are committed
  together, with a final clean-tree check required after the commit.
