# PA6 architecture and contract audit

Scope: the current PA6 handout and `spec.md`. PA7 expression typing, overloads,
substitution/instantiation, lowering, optimizer policies, MIR/ELF, executable
runtime/text and self-hosting have no production surface at this stage. They
are future obligations, not simulated by PA6.

## Source to semantic facts

`syntax/driver.cpp` owns one immutable preprocessed translation unit, its
identifier arena and one `Ast`. The existing PP -> post -> syntax cursors remain
streaming. No new token stream, text serialization or external compiler enters
semantic construction. The optional `Analyzer` is allocated once per TU only
for `--emit-types`. `Parser::translation_unit` gives each completed source region
to `DeclarationConsumer` before parsing the next region. Semantic construction
walks the same source-faithful nodes and attaches `Fact` records by `NodeId`;
it never builds a replacement declaration/expression tree.

A namespace/class region includes its already-parsed children. Member signatures
and member declarations are analyzed once. Bodies are queued once and analyzed
after their complete outermost class, so later class typedefs and nested types
are visible. Free functions are analyzed at their declaration point. Local
classes own queue intervals, preventing later enclosing-scope declarations from
changing an earlier body. The parser's category-only class lookahead indexes delimiters and
recognizes alias names; it constructs no second grammar tree. Function parameter
facts are reused by the body owner. Statement traversal creates required block
scopes without adding PA7 expression typing.

For `extern int a[]; int a[3]; using U=int[];`, the source declarations share
one entity whose completed type becomes `array[3](int)`. Their source nodes and
locations survive. `U` still denotes the canonical incomplete array. The output
view reads the entity's completed object type. It does not mutate the interned
array record or retroactively change an alias's meaning.

For `void f(const int); void f(volatile int);`, source parameter types remain
separate canonical structural records, while both declarations share one entity
and the adjusted `function(int)->void` signature. Arrays/functions adjust to
pointers; cv below pointers survives; sole void and variadic identity are
normalized. The identity API verifies these relationships without parsing a dump.

## Identity, lookup and lifetime

Identifiers use the inherited TU-local identifier table. Types, entities,
scopes, source declarations and constant facts use stable integer identities.
The structural type index compares typed fields and parameter slices. Name
indexes are open-addressed arrays keyed by `(ScopeId, IdentifierId)` with
separate ordinary/tag/namespace/qualifier roles. No rendered spelling, type,
mangling or serialized syntax is a semantic equality key.

Scopes store linked source-order declaration and child sequences. Namespace
reopening extends the same scope; namespace aliases retain their target identity.
Using declarations import entity identity. Directives store explicit edges;
lookup observes later declaration and graph additions without a stale snapshot
or global cache invalidation. Unqualified nominations participate at their
nearest common enclosing scope; qualified lookup follows its direct-name and
inline-namespace rules. Local indexes and traversal stamps visit relevant
lexical scopes and edges only. Geometric ancestor links use one extra ID per
scope. There is no scan of unrelated translation-unit declarations.

Class/enum type identity is independent of source spelling (`struct` versus
`class`, or qualified enum definitions). Qualified enum definitions retain a
source scope view while binding enumerators in the original semantic enum
scope. An injected class name and its constructors have separate identities.
Template parameter environments are parent-linked scopes. A template body is
parsed once and retained; template-template inner parameters are not exported.
No instantiation cache or invented specialization semantics is present.

Class-only constructor/layout demand state lives in a separate indexed arena.
The common entity record and inline constant payload are packed.
Persistent records use geometrically growing flat vectors; variable function
children use one parameter arena. Temporary suffix/parameter/lookup work vectors
have lexical lifetimes. The graph, type/fact arenas, sources, lookup scratch,
identifier bytes and parsed template bodies are released together at TU end.
No owning smart pointer appears in a hot node, and no process-global mutable
cache accumulates translation units. The driver's sole optional `unique_ptr`
owns the whole analyzer, not individual graph nodes.

## Completed facts and constants

Signature and parameter-adjustment caches belong to the immutable type arena,
keyed by `TypeId`. A completed signature request is O(1) average; first work
visits structural children and argument slices. Object array completion creates
or reuses an interned composite type rather than invalidating old types.

Constant results belong to the parsed expression `NodeId` in its single PA6
lexical environment. A sparse value arena stores successful facts and one
expected-nonconstant sentinel. Results are established at the declaration point;
later entity insertion does not replay already-resolved expressions. Future
instantiation must use its own substituted-fact/environment identity, never
reuse this source-region cache across different substitutions.

The evaluator reads PA2 decoded literal payloads, propagates integer types,
performs promotions/conversions, rejects required signed overflow and bad
bounds, and does not evaluate unselected logical/conditional operands. Enum
initializers can use prior definition-time integral values; completed enumerator
entities transition to the enum type. Scoped conversions/comparisons remain
checked. Reference constants preserve the referred value. Plain complete class
layout has explicit in-progress/success state, handles target alignment and
static-member exclusion, and is computed only when sizeof/alignof demands it.

## Complexity and evidence

Parsing and semantic traversal are proportional to consumed nodes and emitted
facts. Structural interning has expected constant-time probes plus necessary
parameter comparison; each signature fact is completed once. Lookups visit
lexical ancestors and reachable nominated scopes; nearest-common-ancestor work
uses geometric links. Output is linear in emitted text, including repeated
source-facing type spellings. Constant work is bounded by demanded expression
nodes, and layout by the demanded class's member facts. No optimization pass,
fixed-point global rescan, global generation counter or phase text roundtrip
has been introduced.

Optional telemetry reports total frontend/output time, semantic time included
in frontend time, peak RSS, arena/node counts, semantic entities/scopes/types,
signature transitions, type probes, relevant lookup work and constant work.
Telemetry reads existing work and does not request extra semantic analysis.

Validation evidence: unchanged PA6 105/105; full PA1–6 through 498/498; 33
independent personal behavior cases; direct semantic identity/lifetime API;
inherited PA5 10 core +15 extended +19 audit cases and API. The final isolated
ASan/UBSan/leak build passes both APIs, all 33 personal cases and all 105 unchanged
PA6 contracts, including multi-primary translation units. The file audit checks
71 implementation files. Frozen ordinary timing, separate telemetry and all
raw performance observations are recorded in `performance.md` and
`student.tests/pa6/performance.json`.

Course fixtures, references, discovery, comparator behavior and coverage remain
unchanged. Reference observation was limited to two personal anonymous unions
to determine that their presentation suffix is the preprocessed token interval;
production constructs those spans itself and never calls the reference tool.
