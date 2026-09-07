# PA5 architecture and completion audit

Scope: PA5 source-to-AST mode. The stage began at the recorded plan base with
0/188 cases and an unimplemented driver. It now implements the syntax handout
and keeps PA1–4's shared frontend. Review markers in `plan.md` remain unchanged;
this is the implementation audit, not an independent Ralph review.

## Required surfaces and evidence

- `--emit-ast -o output source...` creates separate TUs in operand order, with
  exact deterministic AST views and ordinary failure exits. The unchanged 188
  contract cases cover declarations, templates, classes, expressions, statements,
  ambiguity and rejection; the root through report also includes all 205 earlier
  cases. No references, fixtures, comparators, discovery or coverage were changed.
- Personal tests are explicitly run from `student.tests/pa5`: 10 core cases,
  15 extended cases, and C++ API checks for stable graph links, decoded literals,
  physical/presumed locations, user-literal payloads and bounded nested-angle work.
- The isolated ASan/UBSan build uses every registered frontend source, C++11,
  `-O1 -g -fno-omit-frame-pointer`, leak detection, and a separate object directory.
  It runs the API checks, all 15 extended cases, and all 188 course fixtures with
  exact success-output/status comparison. Sanitizer diagnostics fail the runner
  even when a fixture expects a parsing failure.
- `performance.md`, its raw datasets and the executable verifier document the
  final compiler measurements. Generated executable runtime/text and actual
  self-hosting are N/A at PA5. Syntax workloads include templates, loops, calls,
  array memory access and floating-point expressions; they do not claim to
  measure generated code.

## Ownership and source-to-AST trace

The driver owns a `Preprocessor`, post-token cursor, `Ast`, syntax cursor and
parser per TU, in that lifetime order. Immutable source buffers and interned
identifiers live in the preprocessor. The post-token cursor borrows them and
keeps only phase-7 lookahead and a string-literal decoder. The syntax cursor
copies demanded decoded literal values into TU arrays and interns persistent
spellings; it never retains borrowed post-token scratch pointers.

For `template<class T> struct packet { T items[3]; T at(int i) { return
items[i]; } }; packet<int> object;`, the parser creates one template-parameter
node and a parameter scope, publishes `T` as a type there, builds one class
body and its function/subscript nodes, and publishes `packet` as a template
type in the enclosing scope. The later `packet<int>` owns structured name
components and a type-id argument. It does not instantiate or reparse the class
body. Syntax-only name facts choose grammar; no type checking is claimed.

Nodes are 32-byte values in one geometrically grown array. Their 32-bit IDs
survive reallocation; children/next/detail are non-owning IDs. A separate source
location array retains physical file/offset ranges, presumed filename IDs and
lines, so syntax wrappers can share location identity. Literal records retain
scalar bits, array bytes, element counts, kind/type, suffix and numeric prefix.
The API explicitly reads these values after the source cursor reaches EOF and
checks a macro expansion following `#line`.

Names and template arguments are structured nodes; rendered qualified names and
types are never lookup keys. The AST writer follows node IDs and renders inline
name syntax only for the requested view. No parser consumes the dump. Node
arrays have no owning shared pointers, per-node child vectors or recursive
ownership/destruction. All TU arrays and sources are released before processing
the next source operand; no mutable cache is process-global.

## Parsing, lookup and bounded work

- Each grammar region constructs its syntax once. Declaration/function common
  prefixes share the same specifier/declarator nodes; there is no token rewind,
  alternative tree clone or grammar replay. Precedence climbing supplies
  associativity, with explicit template-argument delimiter context. Logical
  halves of `>>` preserve shift behavior inside parentheses.
- A circular token buffer retains only unresolved prefixes and complete-class
  lookahead required for later-declared nested types. Delimiter mates are indexed
  once while pulling tokens. Class-category lookahead skips indexed nested
  bodies; later body parsing constructs the only AST for that region.
- Successful angle-pair observations are indexed on the retained opening token.
  The key is the token's absolute cursor ordinal, with the same immutable token
  spelling and delimiter context. These cache punctuation facts, not semantic
  classifications. Failed outer scans still use the ordinary parser choice.
  Split closing-angle pieces retain their original ordinal; consuming the
  token discards its annotation. No global invalidation or persistent cache exists.
- Lexical fallback flags are cached once per interned identifier. Their complete
  key is immutable identifier identity; declarations and parameter categories
  are checked first and can override the hint. No cached semantic answer can
  survive a scope mutation incorrectly.
- The name table is flat open addressing keyed by `(ScopeId, IdentifierId)`.
  Namespace reopening and aliases share target scope IDs. Using/base edges are
  explicit linked records; lookups visit lexical parents and required imported
  scopes only. A reusable worklist and per-traversal visitation stamps break
  import cycles. Those stamps are scratch visitation state, not semantic-cache
  invalidation generations. There are no whole-program declaration scans.
- Arena and ring growth are geometric. Grammar construction is proportional to
  consumed tokens/nodes; indexed nested-angle probes now exhibit linear work.
  Lookup additionally pays for the lexical/using/base edges required by syntax.
  Rendering pays for actual output, including indentation. Deeply nested class
  dumps can contain quadratically many indentation bytes; that is output size,
  not repeated parsing. The performance campaign separates frontend and emit
  times and describes its independent fourfold depth workload.

## Optimization evidence and later boundaries

The frozen full-behavior baseline exposed repeated nested-angle scans: fourfold
input increased wall time far beyond the predeclared sixfold bound. The final
indexing change removes those scans without changing the AST. Compiler budgets
were fixed in the plan before the campaign; the final verifier checks both ABBA
blocks, A/A noise, RSS, host text growth, scaling, binary/input hashes and exact
output equivalence. Optional telemetry counts existing work/allocation growth
and times the fused frontend and requested AST view; it triggers no extra
semantic analysis. No generated-code optimization is claimed.

PA6+ canonical types, overload facts, specialization-demand states, typed LowIR,
MIR, ELF and optimization policies have no PA5 output surface. The stable graph,
interned keys, structured template bodies and direct cursor are the extension
points for those phases. No second syntax/semantic graph, serialized interphase
transport, host compiler delegation or native-code substitute was introduced.
