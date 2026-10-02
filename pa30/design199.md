# PA30 implementation199 ownership and validation

Entry: `d9a625844ca9a182f74152abfb08e2e67b0057b9`.
Code tip: `d6408429`. This extends the cumulative compiler at O0. No fixture,
harness, comparison rule or reference changed during this implementation.

## Completed behavior group

| Owner | Source → fact → consumer | Complexity and lifetime | Validation |
|---|---|---|---|
| `semantic/type_builder.cpp`, `query_new.cpp` | First `new[]` extent becomes a query child, separate from canonical element/inner-array types. Query children are construction, optional extent, then placements. Substitution checks extent and access; actual allocation lowering keeps its recorded runtime extent. | O(declarator dimensions + initializer/placement children); canonical query/type IDs in TU storage. No constant evaluation of a runtime first bound and no broader definition demand. | Runtime arrays, inner fixed extents, dependent return `decltype`, placement allocation; negative bounds, invalid bound types and nonconstant inner extents. |
| `semantic/friends.cpp`, `access.cpp`, `template_call_facts.cpp`, `query_call.cpp` | An enclosing current-instantiation friend is an explicit source-class friendship edge. Protected access requiring a dependent base stays a query obligation instead of a completed fixed-call fact; concrete substitution rechecks privilege. | Indexed class-pair friendship key, lexical parents and relevant base edges only. Existing TU source/query owners; no global invalidation/retry or new cache. | Tag/simple-type friends, wrong-specialization and absent-friend rejection; dependent protected call, unrelated/specialized base rejection. |
| `semantic/template_checks.cpp` | Signature normalization follows indexed fixed-base member lookup and canonical alias/enum identity. Original source access/type obligations remain. | Relevant fixed-base lookup followed by existing signature memoization; ambiguity is a failed match, not an invented type. | Inherited catalog alias declaration/definition match and distinct-type negative. |
| `syntax/prediction.cpp` | A member-template function with a template-parameter return type establishes the complete-class lookahead function boundary. Trailing cv qualifiers cannot classify the next member's type as a value template. Ordinary parsing still owns the body once. | Existing bounded lookahead and indexed category binding; no token cloning/body replay. | Three consecutive member templates with const/const-volatile suffixes, hosted regex iterator, full earlier report. |
| Builtin metadata, `semantic/builtin_functions.cpp`, conversion facts, `lowering/intrinsics.cpp`, `values.cpp`, static initializer lowering | Five fixed vector builtins acquire typed declarations and intrinsic IDs. Immediate lane validity is checked in ordinary, fixed-template and substituted-query calls. Lowering consumes selected calls/conversions, creates vector slots and lane stores/loads. Equal-size GNU vector/integer representation casts have a distinct conversion fact and typed byte copy. Volatile vector values snapshot the full operand before extraction/copy. | Fixed vocabulary; initialization at most eight lanes. Representation transfer O(bytes). Volatile snapshots use existing `vector_each` (unroll ≤8, loop thereafter). Function-local IR/storage is reclaimed by the existing per-function pipeline; no external assembler or compiler. | Arity, sign/truncation, side effects once, constant/range lane checks, cast size/kind rejection, vector-to-void preservation; checked object/LowIR execution and four volatile lane loads for two full two-lane reads. |

The fixed builtin vocabulary is `vec_init_v8qi`, `vec_init_v4hi`,
`vec_init_v2si`, `vec_ext_v4hi`, `vec_ext_v2si`, each with the
`__builtin_ia32_` prefix. No fictitious `vec_ext_v8qi` declaration remains.
The course's signed-char lane contract is retained. This does not claim all
GNU SIMD intrinsics or vector expression syntax are implemented.

## Trace and legality

[Trace evidence](../student.tests/pa30/evidence199/trace.json) retains validated
LowIR, exact object hashes with/without telemetry, ELF symbols, disassembly and
CFI for nine reducers. An explicit external-LowIR reader/production-object
adapter links and executes each result. The representation reducer also passes
standalone LowIR → MIR → native execution; its MIR is retained. These adapters
are test-only. Production still directly passes typed graphs/IR to object emission.

`allocate<int>` illustrates demand: parsed dependent array syntax produces the
allocation query and element type, specialization supplies int, the ordinary
expression owner records allocation/conversion facts, lowering emits its size
calculation/allocation and the backend emits ELF relocations. Source regions
are shared; this change does not replay template grammar or scan unrelated
pending entities. `friend-current` traces indexed privilege into a demanded
nested-class member and then ordinary call lowering.

For vectors, the fact is the selected builtin plus conversions and a proven
constant lane. Legality requires the fixed signature and immediate range;
representation casts require equal object sizes and an allowed cast category.
No optional transform or new optimizer profitability policy is introduced.
The conservative O0 implementation has actual slot traffic and byte copies,
visible in the retained disassembly/MIR, rather than a claim based on fewer IR
nodes. Vector snapshots preserve volatile effects. Inherited native/inline
work limits and ABI/frame/unwind owners remain unchanged.

## Validation and remaining boundary

Final reports: PA1–PA29 **4941/4941**, PA30 **138/153**, through PA30
**5079/5094**. File audit passes (four inherited header-division warnings).
Six existing failures are fixed, none introduced; all 153 fixtures and their
expected outcomes remain unchanged. The supplied cached 154 count was already
reconciled to the primary log and inventory by audit198.

There are **88** implementation controls, **107** rerun accumulated checkpoint
controls, and **93** trace commands. All succeed with expected negative cases
rejected. An intermediate vector-to-void regression was corrected before the
final reports. The volatile operand defect found during implementation review
is fixed and included in the final binary and measurements. The vector runtime
benchmark also exposed invalid scalar initializers for zero vector static
storage. Global zero initialization and omitted aggregate vector elements now
emit a typed zero byte span (O(1) IR data items). Plain/volatile globals, local
statics, arrays and struct members pass direct and roundtrip execution;
`zero-before.json` retains the entry and intermediate failures.

The initial eight constant/access failures were followed through downstream
owners and extended to fixed-vector emission: five initial fixtures plus the
fixed-vector fixture now pass; regex compilation advanced through allocation,
signature lookup and parser repair to constructor selection; both random cases
advanced through friendship to unavailable packed arithmetic. Implementing
saturating pack operations is a new typed SIMD legality/lowering group, not an
extension of construction/extraction by name. Completing callable scheduling
and tuple/pair construction requires tracking missing prerequisite facts and
candidate conversions across different owners. Three existing declaration/flow
rejections also require their own proofs. None can be made correct by loosening
access, retrying globally or adding intrinsic stubs. This is the concrete
boundary for an incomplete implementation handoff after extending the related
group; all 15 remaining required failures remain implementation obligations.

The preexisting general vector-subscript gap is separately reproduced in
`student.tests/pa30/pending199/vector-subscript.cpp` and `evidence199/pending.json`:
entry and current reject the valid expression. It is unfinished implementation,
not an independent-review question or a waived requirement. The completed
builtin interface is tested directly rather than depending on that missing
syntax. Independent review of the new delta is pending; no whole-stage audit
or advancement is claimed.
