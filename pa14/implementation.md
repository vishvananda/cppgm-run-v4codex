# PA14 implementation ownership

Current code `16e7c163`: **281/314**, 33 failures. This continuation fixes **59
of its 92 entry failures**, with no lost passes. Cumulatively, 197 of the stage's
230 original failures are resolved. Fixtures, references and comparison rules
are unchanged; PA15 has not been started.

## Owners, data flow and bounds

| Owner | Implemented data flow / complexity | Validation |
| --- | --- | --- |
| `syntax/ast.h`, `occurrence.cpp` | One parsed source graph; compact source/context occurrences project edges without grammar replay or source-node copies. Ordinary reads retain the inline O(1) path. All storage belongs to the translation unit. | Previous frozen source-read measurements; current sanitizer parity. Whole-region occurrences remain a limitation below. |
| `semantic/template_declaration.cpp`, `template_call.cpp` | Parameter ordinals normalize callable shapes; canonical template entities and interned type-argument packs index specialization declarations. Direct/target deduction feeds ordinary candidate/conversion machinery. | Existing declaration, call, address and overloaded-argument personal programs. |
| `semantic/dependent_type.cpp`, `types.cpp` | `DependentName` owns canonical type qualifier, interned member name and type-argument slice. Substitution follows only these structural edges, completes the concrete owner as required, and resolves the member type. Qualified types remain non-deduced contexts. | Renamed function declarations/definitions, qualified return/member aliases, concrete short/long identity in `dependent-types.cpp`. |
| `semantic/template_class.cpp` | Class identity precedes completion. Default types are resolved in their declaring head; ordinal substitutions merge defaults across renamed heads. A later forward declaration preserves the defining head/body. Default bindings are allocated only when a default changes heads. | Later defaults, forward upgrades, renamed heads and dependent alignment in `instantiation-demand.cpp`; related course families pass. |
| `semantic/template_definition.cpp` | A canonical owner-path trie and `(path, member-name)` buckets retain parsed nested-class/function/static-data definitions and parameter slices. A concrete root/definition key has Active/Success/Failure state. A requested bucket gets one application per key and a compact head overlay; no scan of unrelated templates or specializations. Source definition environments feed ordinary declaration, body and initialization facts. | Renamed namespace-qualified constructors/destructors/static initializers, nested definitions, late bodies/destructors and unused invalid bodies in `member-definitions.cpp`; definition application scaling is measured separately. |
| `semantic/explicit_instantiation.cpp`, `member.cpp`, `operators.cpp` | Explicit class demand visits only that class's declarations and available definitions, recursively including defined nested classes. Evaluated values/addresses request static storage; unevaluated operands do not. Member bodies retain separate demand. | Explicit qualified/global class instantiation, namespace/class-key rejection, zero-initialized static objects and unevaluated invalid initializers. Extern declaration syntax is retained; full extern emission suppression and general explicit function instantiation are **not** implemented. |
| `semantic/template_checks.cpp` | One source walk checks active template-parameter redeclarations and direct type-parameter value misuse. An owner/name prototype index compares independent nullary exception specifications; dependent/signature-sensitive checks stay with concrete declarations. | Shadowing, using-declaration and exception rejection families. This is not full definition-time body checking. |
| `syntax/class_parser.cpp`, `declarator.cpp`, `prediction.cpp` | Qualified definition overlays retain head names and class member precedence. Existing class parser scopes survive redeclaration. Base clauses see injected nested-class names; elaborated template-ids resolve to canonical semantic class identities. | Nested bases, renamed heads, local typename declarations, ordinary methods hiding outer class templates, elaborated aliases. |
| `semantic/converting_constructors.cpp`, `lowering/*` | A variadic constructor with no named parameters records the actual ellipsis argument/conversion. Selected static constant values and template-member linkage provenance feed ordinary typed LowIR. ABI construction consumes canonical dependent type paths. | Native ellipsis-conversion counter, constant through dependent base, static storage address and nine personal executables. |

Definition paths, indexes, parameter overlays, occurrence facts and demand queues
are translation-unit owned. Head merge work follows parameters/default type edges;
member application work follows definitions in the requested owner/name bucket.
An overloaded bucket currently applies all matching-name retained definitions;
finer signature ownership and failure records remain to be developed. Body/storage
queues deduplicate entities. Available late definitions are attached when the
already-demanded entity is drained; there is no whole-program restoration retry.

## Remaining groups and concrete checkpoint boundary

| Owner | Next data flow / complexity requirement | Representative failures |
| --- | --- | --- |
| Symbolic expression/signature facts | Bind declarator parameters in a signature environment; retain dependent `decltype`, trailing-return and bound expressions as typed facts. Substitute dependent edges and share fixed bindings/results. Work follows newly demanded facts, not every projected node. | `100-function-template-parameter-decltype-ref-array`, `100-function-template-result-member-trailing-return`, `100-template-auto-trailing-return`, `300-dependent-decltype-function-pointer-reference-call`. |
| Definition-time lookup and control facts | Record fixed ordinary bindings and base-specifier dependence before specialization. Check unused bodies with real lexical/condition scopes, value categories and jump-lifetime facts. Lookup follows indexed lexical/base/associated edges. | Missing/ambiguous/type-as-value rejection cases; direct/local dependent-base provenance; switch jump bypass. |
| Inherited object, lifetime and ABI facts | Preserve implicit move/reference transfers, empty-object actions, local enum identity, constructor ABI entries and virtual-destructor lifetime through specialization. Lower recorded facts once. | Reference-member/defaulted moves, local constref iterator, nested out-of-class defaulted copy, inherited conversion, reentrant layout and rvalue-reference return cases. |
| Remaining parser/template contexts | Replace spelling-based category guesses with declaration-owned context; complete the remaining inherited-call and template declaration forms within the contract. | Lazy right-shift member lookup, inherited constructor using, variable-template-defaulted fixture. No fixture is excluded from the 314-case exit suite. |

The retained-definition group is complete as a checkpoint and was extended
through explicit class demand, renamed/default heads, static address storage,
elaborated identity, alignment and method hiding. The remaining nested-definition
fixture now attaches its body successfully but differs in defaulted-copy lowering;
that failure belongs to inherited transfer/triviality facts, not registry lookup.

Further related work requires a new semantic representation: expression-valued
dependent types need parameter bindings before a concrete callable exists, and
fixed body names must keep definition-time results before substitution. Adding
more concrete re-resolution to the registry cannot supply those facts and would
violate dependent-only checking. That is the architectural boundary for this
incomplete checkpoint, rather than a test-progress or commit-count threshold.

Parsed-node sharing is implemented; sharing all nondependent **semantic facts**
is not. Finer occurrence demand, explicit typed demand edges/reverse dependencies,
separate layout/default/exception/body states and narrow structured expected
failure memoization remain **current PA14 spec requirements**. They are not
waived by the performance review or deferred to PA15.
