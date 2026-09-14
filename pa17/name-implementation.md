# PA17 source-name implementation — loop 51

Entry `0fe13f160c921db3ca68bfefc857b3a6be40cf3f` passed 301/343. Source
commits `3ce88dd7`, `d7f0055d` and `6afed48e` pass 306/343, fixing the five original
introducer failures with no new failures. Tests and references are unchanged.
This is implementation evidence; the cumulative independent audit is pending.

## Rules and ownership

The governing checked-in rules are N3485 [temp.names] 14.2/4–5,
[temp.res] 14.6/3–7 and [temp.dep.type] 14.6.2.1/1–4, in
[the course standard text](../doc/n3485.txt). PA17's README explicitly requires
these checks on retained definitions, including dormant templates.

`syntax/name_parser.cpp` already records `template`, and type specifiers retain
`typename`. `semantic/dependent_type.cpp` checks those source facts as it binds
a type path. A dependent qualifier keeps its canonical type identity. Only a
qualifier equal to a lexical class's current-instantiation type can use a known
member type without `typename`. A dependent base with no established member,
another specialization, and a `sizeof...`-dependent qualifier retain the
obligation. Fixed bases, aliases, nested classes and partial patterns use their
existing entity/type graph and indexed lookup.

The current-instantiation type cache belongs to the translation unit. Its key
is class entity plus immutable source-parameter slice. The entity fixes head
width and partial shape; a renamed definition owns a different slice. A new
head changes that slice, while adding defaults does not change the fully
specified injected type. Partial shapes substitute the source parameter
identities once, preserving their existing pack-expansion boundary. There is
no rendered-name key or whole-program search. Miss work is proportional to the
head and dependent shape; completed lookup is expected O(1). A qualifier check
walks only lexical class edges and the relevant indexed member lookup.

Non-type arguments initialized directly from a parameter, including a chain of
such constant aliases, use that parameter's canonical argument. An expression
containing it, including `N+0`, retains a distinct query. This implements the
current-instantiation equivalence rule without substituting concrete values to
decide source dependence. It also preserves the distinction between a fixed
integer and `sizeof...(Pack)` before substitution.

Out-of-class leading types bind before the qualified declarator's class scope
is installed. Parameters, trailing returns and the body use the retained member
environment. Access has a separate context: the member's access rights cover
the entire definition, including its leading return type. The retained access
recipe now preserves that definition context through substitution. Explicit
instantiation access exemptions still do not propagate into unrelated bodies.

Member-expression checks consume the receiver's retained type query, or the
already-bound implicit-object context. Value dependence alone does not require
`template` on a fixed receiver type. Current-object and explicitly qualified
current-class calls keep their exception; other dependent qualifiers require
the introducer before a template-id. Qualified type paths and value calls use
the same canonical owner identities. No call is resolved again in lowering.

## Single-parse declaration ambiguity

A parser flag identifies a sole parenthesized qualified name without
`typename`. Only flagged declarations reach semantic disambiguation. The
semantic owner decides whether that name is an established type or a dependent
value expression. The latter can initialize a variable even when the primary
has a same-named type and a later explicit specialization supplies a value.

One monotonic `ParenResolution` stores three source IDs and six flat role
entries. `AstView` presents the resolved declaration using the existing name
and delimiter nodes. Parsed nodes remain immutable; no tokens are replayed,
no subtree is cloned, and no new frontend nodes are manufactured by lowering.
Source binding establishes this interpretation before that region's semantic
facts or specialization occurrences are published. Region projection uses the
resolved edges and shares them across demands. The original parsed syntax
remains available to the AST tool. Storage is O(number of ambiguous source
declarations), with TU release and expected O(1) view lookup. TUs without a
resolved ambiguity take the empty-index fast path.
The resolved-source and substituted-node cases share one out-of-line view
fallback, avoiding duplicate inline compiler code at each semantic read.

The neighboring grammar fixes publish a class-template category at its point
of declaration, making rooted names visible inside the definition, and use
the existing memoized unexpanded-pack query to distinguish `R(A...)` with a
scalar `A` from an actual parameter-pack expansion. No second template model
was introduced for parsing.

## Validation and boundary

All 56 new [name controls](../student.tests/pa17/name_controls.py) pass: 36
native behaviors and 20 rejections. The frozen entry fails 19 of them. Coverage
includes dormant/demanded definitions, fixed/dependent/current receivers,
qualified member chains, ordinary and partial renamed heads, value aliases,
pack identities, leading/parameter/trailing return positions, access,
elaborated type contexts, varargs, direct initialization and function pointers.
All 205 inherited controls pass, including the four LowIR ownership checks.

The root earlier-stage report passes 2266/2266. PA17 remains 306/343; the root
through-PA17 report is 2572/2609, with every failure in PA17. File audit passes
with its three inherited header warnings. Exact sets, logs, hashes and commands
are recorded in [handoff.json](../student.tests/pa17/handoff.json).

[Performance evidence](name-performance.md) covers frozen binaries and inputs,
A/A noise and ABBA compiler latency/RSS, and separate checked native runtime
and text size. The initial campaign remains intact. The parser flag removes
avoidable ambiguity inspection from ordinary declarations; it adds no optional
generated-code transform. PA17/O0 has no mandated numerical performance ceiling.
Evaluator limits and existing lowering work/growth policies are unchanged.

The remaining 37 failures require different owners: partial/variable/alias
selection and substitution, expression-query recursion and candidate handling,
or LowIR storage/initialization/transfer/cleanup. The rooted static-member case
now compiles and reaches a storage-initialization LowIR difference; additional
name-introducer checks cannot decide that emission policy. All 37 remain
required implementation, not review questions or waivers. Further related
source-name work has been exercised across these interactions; moving into
the remaining failures requires separate query, selection or lowering algorithms.

Independent review must assess the cumulative source-head/cache and friend
ownership changes from earlier handoffs, plus current-instantiation cache
validity, source ambiguity publication/projection, and the separation of
access context from lookup position. Passing implementation checks does not
replace or waive that review.
