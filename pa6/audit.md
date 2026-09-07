# PA6 independent final architecture audit

Review boundary: stage base `9249196518f45492822fb2e3da4eb5d82af0ed13`
through checkpoint `15c7f1fcd23fd3145b2ec0a611af28c78c64825a`, plus the final
corrections below. All stage implementation commits were inspected, including
`5749f43b4`, `78df00b67`, `3f6de92f5`, and `1edcbe5db`; the checkpoint ledger did
not substitute for tracing the current source. PA6 emits AST/types. Template
instantiation, expression resolution, LowIR, native optimization, MIR/ELF,
generated executable runtime/text and self-hosting have no production surface
here and remain obligations at their owning assignments.

## Final Spec Alignment

| Spec | Actual owner and invariant | Independent evidence |
| --- | --- | --- |
| 1, 8: source and parsing | `Preprocessor` owns immutable sources/identifier bytes; PP, post and syntax cursors pull tokens. The syntax ring retains only unresolved prefixes/class lookahead. `Parser::translation_unit` calls the semantic consumer on each completed region of the single arena graph. | Reviewed cursor fill/take/delimiter indexing, class category lookahead and declaration callback. PA5 graph/literal lifetime API and all inherited contracts remain passing. |
| 1, 2: one typed graph | Semantic `Fact` entries attach to original NodeIds; no expression/declaration tree clone or text transport. Entity types use structural canonical IDs; declaration facts retain source forms. | Direct API checks aliases, nested function signatures, source parameters, completed arrays, body identities and arena growth. |
| 2, 3: scopes and lookup | Flat indexes use `(ScopeId, IdentifierId)` with ordinary/tag/namespace/qualifier roles. Using edges use `(ScopeId, ScopeId)` identity, separate inline adjacency, and traversal stamps. | Qualified owner, shadowing, source-point, cycle, duplicate-path, inline-set and graph-change probes. |
| 4, 5: demand and validity | Parent-linked template parameter scopes; one retained pattern. Only complete-class bodies defer, in the owning class queue interval. Signature/adjustment caches key immutable TypeIds. Constants key a source NodeId in its single lexical environment. | Nested/local class and source-point probes; API verifies completed signatures do not repeat work. No global retry, generation counter or negative lookup cache. |
| 6, 7: useful facts and budgets | No lowering or optimizer exists at PA6. Required constants/layout/signatures are retained typed facts, not reconstructed from dumps. Compiler data-structure improvements preserve all declarations and checks. | Class padding/overflow, conversions, constant short-circuiting and compound-type regressions. No runtime optimization claim inferred from node counts. |
| 8, 9: storage and complexity | Geometric TU-owned arenas; no per-node owning smart pointers, individual node allocation or recursive graph destruction. Scratch vectors have lookup/body lifetimes and reuse capacity. | Sanitizers/leak checks, direct lifetime API, frozen 1x/4x benchmark families and explicit work counters. |
| 9, 10: evidence and containment | Driver writes views from this implementation; no external compiler/reference produces required output. Ordinary timing is separate from telemetry. | File audit, source invocation review, unchanged course discovery/oracles, binary/input/output hashes and A/A + ABBA records. |

## Representative data paths

1. `namespace N { using F=int(const int); extern F* p; } int (*N::p)(int);`:
   immutable source bytes enter the streaming cursors; the parser retains one
   structured namespace/declarator graph with source locations. The consumer
   creates N's scope, one alias entity and one object entity. Source function
   parameter qualifiers remain on declaration types, while structural signature
   normalization gives both p declarations the same pointer-to-function type.
   Qualified ownership resolves N by its indexed scope, never by rendered text.
   The output walks the original declarations and retains their source views.
   `extern int a[]; int a[3]; using U=int[];` similarly completes the object
   entity without changing U or an interned incomplete array; array rendering
   reads the completed bound alongside its declaration's source element type.

2. `template<class T> struct Box { T member; void f(){Later x;} using Later=int; };`:
   the parser records the template region once, using delimiter-indexed class
   category lookahead to recognize Later in the body. The semantic parameter
   scope is a parent-linked environment with a stable T entity; Box/member
   reference its named TypeId. Class completion establishes Later before the
   queued body is demanded. f's definition points to its original source node
   and function scope; x resolves directly to int. A local class drains only its
   own queue interval, so later outer declarations cannot change its body.
   PA6 does not instantiate Box: no substituted tree, template replay, layout,
   emission, or fictitious ELF trace is claimed.

3. `struct S{char x;int y;static int z;}; int a[sizeof(S)];`:
   decoded type-forming syntax resolves S's named TypeId. Layout transitions
   from not-started to in-progress to completed in S's class fact record;
   nonstatic members yield size 8/alignment 4. Checked additions and alignment
   prevent wraparound. The sizeof node retains a typed unsigned-long constant,
   and the array constructor interns its positive bound. Further demands reuse
   the layout. Failure aborts this TU, so there is no recover-and-retry path
   requiring a cached failure payload. No extra analysis is run for telemetry.

At TU end the analyzer (types, bindings, facts, class records and lookup
scratch), parser category index, syntax graph, cursors, source buffers and
identifier storage are released by the driver's nested ownership. The optional
unique_ptr owns the whole analyzer; hot records contain compact IDs, not owning
pointers. PA7 must extend facts on this graph and use substitution/environment
identities for future instantiated facts, rather than reuse a source NodeId
cache in a different environment.

## Findings fixed across their owners

| Finding at checkpoint | Final correction and regression evidence |
| --- | --- |
| `using` aliases silently replaced incompatible declarations; typedef and using function aliases disagreed; object/parameter pointers could retain unnormalized function signatures. | One alias declaration path validates and reuses entity identity. All entity type paths normalize structural function children. Source spellings stay in original declaration facts; the renderer separately consumes array completion. Direct API tests both canonical equality and source distinctions. |
| Qualified class/enum definitions accepted non-enclosing scopes. Parser class scopes attached qualified definitions to the lexical writer, losing lowercase aliases in member bodies; enum base lookup used that same wrong environment. | Semantic class/enum owners enforce enclosure; parser scopes reuse the qualified target and restore the lexical scope; enum base types resolve after their qualifier. Valid namespace/class owners and invalid sibling definitions are independently checked. |
| Constructor function-try bodies disappeared; loop/branch declarations leaked into surrounding blocks. | Special definitions retain their FunctionTry node, and control/unbraced statement scopes own their declarations. Tests inspect body/handler variables, shadowed constants, and rejection of expired bindings. |
| Qualified lookup applied using-directive suppression per namespace instead of to the complete inline namespace set. | Collect direct declarations across the inline set first, then follow ordinary directives only if that set is empty. Tests cover root/child hits, absent direct names and ambiguity. |
| Adding each using edge rescanned all prior edges; direct qualified hits scanned ordinary edges looking for inline ones. | A flat edge index deduplicates each pair in average O(1); an inline adjacency chain visits only inline edges. The new fixed corpus checks two requests per edge, linear lookup work and actual latency/RSS. |
| Class layout arithmetic could wrap a huge object into size 1. | Checked member additions and padding, including final tail alignment, reject overflow and preserve a representable maximum-size array. |
| A call to a template parameter could mutate sentinel class state. | Implicit class-constructor scope demand requires an actual class-fact identity. Instantiation remains outside PA6. |

The authoritative `300-ambiguous-using-directive-type-bad.t` requires distinct
aliases reached through competing directives to remain ambiguous even when they
name the same canonical type. This contract is preserved; alias compatibility
within one scope does not conflate independently declared entities in other
scopes. No checked-in fixture, reference, harness, timeout or discovery rule was
changed to accommodate the corrections.

## Work, growth and invalidation ledger

| Operation | Legality and validity | Work / storage budget and fallback |
| --- | --- | --- |
| Structural signatures and parameter adjustment | Pure function of immutable TypeId and its parameter slice; source cv remains available. Incomplete object-array completion creates an interned result without mutating a key. | Each distinct signature computed once; repeated completed lookup O(1). Linear work in first-seen type structure, one cached ID per source type; no grammar replay or speculative expansion. |
| Using-edge index and inline lists | Pair identity includes source and target scopes. An inline promotion updates only that edge/list; declaration insertion remains visible because lookup results are not cached. | O(E) retained edges/index storage; geometric tables, average O(1) insertion; each qualified traversal visits required scopes/edges. No graph duplication or fixed-point global scan. |
| Body scheduling | Only language-required complete-class deferral; original source and definition IDs retained. | One queue entry/body, each drained once in its owner interval. No unrelated body retry. |
| Constants/layout | Source-point constants and completed class facts; target integer and size overflow remain checked. | At most one constant evaluation/node and one completed layout/class; conservative nonconstant result or required rejection if facts are unavailable. |
| Whole compiler | Required correctness changes may add bounded work; no executable transformation/optimization level is present. | Frozen stage budgets: wall <=10% + calibrated A/A noise, RSS <=15% +1 MiB, host text <=35% over PA5 and <=5% over first PA6; 4x input <6x wall and <5x RSS +1 MiB. New edge corpus uses the same budgets against the checkpoint. |

The edge index has an explicit memory cost; the final performance report must
show it alongside timing and disclose unaffected-workload regressions. Runtime,
spill, ABI and code-growth profitability are not measurable at this PA and are
not inferred from graph size. `performance.md` retains historical observations;
the independent campaign will identify its exact final binary separately.

## Validation and handoffs

Current corrected semantic build: unchanged PA6 105/105 and through PA6
498/498; 57 personal PA6 cases; semantic and inherited PA5 graph APIs;
ASan/UBSan/leak checks on both APIs, all personal cases and all PA6 fixtures.
The file audit checks 71 implementation files. The final telemetry-only addition
and final performance/exit evidence are pending below until rerun.

Unaudited checkpoint handoffs are now reviewed: first semantic construction,
anchored lookup/caches, declaration-point/body identities, class-state compaction,
and the historical evidence commit. The independent fixes above close the new
findings. No handoff is waived on the basis of an earlier green checkpoint.
