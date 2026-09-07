# PA4 completion architecture audit

Stage scope is phases 1–6 and phase-7 tokenization. Read against `spec.md`, the
PA4 macro/directive handouts, unchanged contract fixtures, and the actual
implementation at `957b47c37`. Performance evidence is in the personal suite.
Review markers in `plan.md` preserve the original baseline; this local completion
check does not advance the independent Ralph review marker.

## Ownership and data flow

`SourceBuffer` owns immutable UTF-8 bytes. File frames borrow those buffers and
hold one PA1 character/token cursor plus local conditional state. The TU owns
source buffers, the flat identifier interner, dense macro records, once-identity
table, and spelling slabs. No mutable compiler cache outlives a primary source.
Physical file/offset identity survives macro expansion; `presumed_file` and line
carry logical location through `#line` and replacement. The next primary creates
a new owner, resetting macros, conditionals, counters and once state.

`Preprocessor::raw` recognizes a directive boundary without executing it ahead
of preceding macro expansion. It streams ordinary tokens; it captures only one
directive. `MacroExpander::next` returns one expanded token; the shared
`PostTokenCursor` performs literal decoding and string concatenation directly.
The preproc CLI alone writes the observation records. Later parsers can consume
the same structured tokens without reparsing output or building duplicate token
streams. The optional source-spelling view remains an adapter concern.

For `CAT(A,B)`, collection retains raw argument slices. Definition-time parameter
indices decide whether substitution uses raw, stringized or once-prescanned
arguments. Pasting joins only boundary spellings, invokes the phase-3 lexer to
validate exactly one token, and resumes macro scanning. Phase 1/2 transformations
are disabled for this already-translated text, preserving raw-string trigraphs.
A pasted helper can invoke macros using following source tokens.

For `I(I(...42...))`, the first invocation captures and indexes delimiters once.
Each nested prescan borrows a range and jumps between top-level delimiters.
An explicit task deque holds pending work, so nesting does not grow the host
call stack. Each argument is expanded at most once per invocation if ordinarily
used; stringizing/pasting/unused arguments do not trigger unnecessary prescans.
Replacement scratch stores only produced tokens and is reused between calls.
Parameterless invocations consume their closing parenthesis directly: no raw
argument buffer, delimiter index, or prescan is needed for that grammar form.

Recursion suppression belongs to tokens. Persistent radix sets record ancestry;
permanent unavailable paint survives parameter substitution. Function replacement
ancestry intersects the head/closing-token contexts, while a head that came from
parameter substitution preserves the course's nesting relationship. Deferred
function names become permanently unavailable only when an invocation is found.
These rules cover the handout's recursive examples and the helper-boundary
fixtures without a global recursion cutoff or knowledge of fixture spellings.

## Complexity, lifetime and validation

Identifier/macro/parameter/once lookup uses compact IDs or flat hashing. Macro
parameters are bound once using a reusable dense scratch index that clears only
touched names; definitions never scan all unrelated identifiers. Ordinary token
work is proportional to source and expansion tokens. Radix membership/insertion
has fixed 32-bit depth; intersection visits differing shared branches and stops
at identical or empty subtrees, never scanning macro definitions. Delimiter
indexing is linear in newly captured tokens, and nested ranges borrow the index.

TU definition/source storage releases at TU end. Invocation arguments and
prescan results release after substitution; expansion-context and generated
spelling storage rewind when pending expansion drains. Only translated deferred
source spellings use persistent slabs. No hot token owns a string, shared pointer,
or individually allocated AST node. The PP3 evaluator consumes expanded tokens
incrementally and retains only typed value/operator stacks. Lazy arithmetic
errors remain governed by conditional selection.

Course checks cover all macro/directive groups and required rejections.
Personal/API checks add identity, lifetime, deep-work bounds and randomized
expansion checks; ASan/UBSan checks both personal and course inputs. The file
audit checks the implementation tree. No course fixture, reference, test coverage,
timeout, or comparator was reduced. Performance measurements compare equivalent
outputs from frozen functional implementations, not from the original stub.

## Applicability of the full compiler spec

- Source/preprocessing (§1), interned identity (§2), ownership (§8), complexity
  and measurement (§9), and self-contained output (§10) have direct PA4 evidence.
- Lookup/overloads, template demand and semantic worklists (§3–5) have no PA4
  language surface. Macro state uses the intended compact TU ownership rather
  than introducing a later semantic representation prematurely.
- Semantic graphs, direct LowIR, MIR, ELF, optimization policies, debug locations
  and executable profitability (§6–7) are later-stage requirements. PA4 produces
  no executable, so generated runtime/text and source-to-ELF/template-demand
  traces are inapplicable here. There is no claim to have implemented them.
- No output delegates to a host/reference compiler. File reads and device/inode
  identity use the authorized host interfaces; references are only test oracles.

PA4's hosted-header compatibility remains the later handout's responsibility.
The required default include search and all checked course behaviors are complete.
