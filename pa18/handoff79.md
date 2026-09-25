# PA18 implementation handoff 79

Entry `ef0e43c0` (**388/420**); implementation `67c4f685` and `50407fc1`.
Final course result **393/420**: five original failures resolved, 27 remain,
no new failures. This completes the source-signature ownership group and the
implementation handoff; PA18 and its independent audit remain unfinished.
Stage base and last-reviewed markers in `plan.md` are preserved.

## Completed group and extension

Qualified template heads retain their bindings while the parser probes nested
angles. Class predeclaration recognizes an alias declaration and skips its RHS,
so a function-type alias cannot invent a member from a return-type parameter.
A typedef's parenthesized declarator retains its declaration role even after a
predeclared typedef-name becomes visible. These fix the using-directive overload,
function-argument cv alias and explicit member-specialization course cases.

The member-expression binder now owns explicit template arguments as typed
arguments. A dependent type/value argument prevents caching the member selection
as fixed, including a fixed receiver beneath a pack expansion. Instantiation
uses each argument's source facts and substitution frame. This fixes the
pack-expanded explicit member-template course case. Controls extend the same
path through empty/multiple lanes, nonpacks, values, aliases, queries, constexpr,
addresses, noexcept and dormant bodies.

The first-declaration course case exposed an incorrect reference expectation;
[the reduced proof](reference-correction79.md) applies C++11 rules to its
nondependent calls. Only its expected exit status changes. Its positive prefix
is retained as an executable personal control. Real dependent first-declaration
lookup had a separate implementation defect, which was repaired rather than
using the reference correction to end the group.

Under N3485 §14.5.6.1 [temp.over.link]/5, dependent unqualified names compare by
source name in equivalent function-template declarations and retain the first
lookup facts. Their overload-set identities remain on the semantic queries;
canonical comparison shapes omit those identities only for dependent names.
Renamed template heads substitute the first signature. A retained first-signature
record owns the original type, declarator, environment and, for a projected
member, substitution frame. The selected definition owns body lookup, parameter
names, top-level cv and parameter forms erased from the callable signature.
The raw-parameter recipe preserves those forms while retaining the first
signature's dependent lookup. Out-of-class member source signatures are matched
and rebound before body binding, so decltype/sizeof of a parameter cannot reopen
the later declaration's lookup.

Value dependence alone does not make an unqualified function name dependent:
`g(sizeof(T))` has a fixed argument type ([temp.dep]/1). Its definition-time
selection is recorded, including function-template candidates; missing or deleted
fixed selections reject before demand. Its enclosing decltype still retains the
dependent expression ([temp.type]/2). ABI projection consumes that selection
and emits an external-name expression using the existing Itanium fact encoder.
The two valid return-only overloads in the combined trace have distinct object
symbols and execute through typed function pointers. The local
[Itanium grammar](../doc/itanium-mangling.txt) supplies expression-primary's
external-name form; mangled text is never a semantic key.

| Owner | Data flow | Work / storage budget | Validation |
|---|---|---|---|
| `syntax/prediction.cpp`, declarators | Scoped binding + retained tokens → one parsed declaration | Existing angle traversal; one binding per open angle, delimiter-index skips; no grammar replay | 32 syntax/argument controls and all earlier parser PAs |
| `template_binding.cpp`, `template_object_facts.cpp` | Explicit argument syntax → typed argument/dependence → per-frame member selection | Source arguments visited once; existing cached substitution and pack lanes | Pack, value, alias, constexpr/query/address controls and scaled member arguments |
| `template_signature_shape.cpp`, `template_declaration.cpp` | Source dependent-name facts → cached comparison shape → first-signature substitution | One shape per argument/query key; one first-signature record per redeclared entity; local head/parameter traversal and memoized type graph | 33 signature controls; 600/2400 declaration workloads |
| `template_checks.cpp`, `template_definition.cpp`, `template_instantiation.cpp` | Indexed prototype → corresponding head ordinals → retained raw recipe → body/query facts | Source matching once per prototype group; head maps are scratch, frames immutable; bodies demanded normally | Member templates, renamed enclosing heads, raw cv/array/function controls; inherited scaling |
| `type_query.cpp`, `query_abi.cpp` | Fixed typed call arguments → recorded selection → ABI entity expression | Existing query cache and candidate sequence; lowering consumes IDs | Fixed-value-dependent overloads/rejections and combined source-to-native trace |

Dense vectors/flat indexes own new facts for the translation unit. Scratch head
maps and traversals end with the operation. No global invalidation, whole-program
scan, semantic text key, reference execution in production, or optional optimizer
was added. Required semantic work remains bounded by source and demanded facts;
[performance evidence](performance79.md) covers its cost and generated output.

## Validation

- `make test-pa18`: **393/420**, exit 2 while 27 required cases remain.
  Four corrected positive course inputs also validate LowIR and execute; the
  fifth resolved case is the proven rejection correction.
- `make test-report-through-pa17`: **2609/2609**, exit 0.
- PA18 file audit: exit 0, the same three inherited header advisories.
- **922 inherited + 65 new semantic controls** pass. The new controls improve
  from **25/65 at entry to 65/65**. With four repaired-course execution controls,
  this run checks **991** cases. Inherited ABI, completion and scaling checks,
  previous repaired-course executions, the audit trace and the new combined
  signature trace also pass.
- All **420 original inputs** are byte-identical. Of **1,686** tracked fixture
  files, **1,685** are unchanged; one expected exit status has the cited proof.
  Comparison rules, LowIR sidecars and required behavior coverage are preserved.

[Evidence manifest](../student.tests/pa18/loop79-evidence.json) retains commands,
exits, hashes, original/final failure sets, fixture manifest, new control results,
entry comparisons, work counters and the combined trace. Failed intermediate
checks remain in `/tmp/pa18-loop79`; final evidence points at the passing run.

## Concrete handoff boundary

The initial parser/member-argument work was extended through equivalent-template
identity, first-declaration lookup, renamed heads, raw body parameters, source
member matching, query dependence, fixed overload selection and ABI projection.
The known defects found in these consumers are resolved and checked together.

The remaining **27 course failures** require ordinary initialization and LowIR
owners: one unknown-bound array after empty expansion, plus 26 comparisons
covering constant/array and empty-tag initialization, object-root and bool/result
metadata, class-result conventions and discarded loads. The nested-alias cast
and class-ellipsis reducers recorded by earlier handoffs remain real unfinished
implementation. Earlier fixtures also require pooling for small scalar-array
shapes, so the next group must reconcile shared initialization and emission
policy across stages. Signature rebinding cannot supply object extents, change
pool materialization or repair result conventions; that work requires a separate
trace through those owners. No mismatch is dismissed as merely cosmetic.

Independent audit should check the separation between declaration comparison
keys and semantic lookup, source/occurrence recipes under renamed enclosing and
pack heads, and fixed-type value-dependent call selection/ABI. These are review
questions with passing controls, not substitutes for the 27 known requirements.
The reviewed marker stays unchanged until Ralph performs that review. PA19 must
wait for the full through-PA18 report and whole-stage audit.
