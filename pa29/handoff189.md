# Implementation189 — template definitions and constant boolean conditions

Entry `54ddacb1ced906e692ea94bde5110b0757e1b2af` had **395/403** PA29 passes.
The handoff has **398/403**: three demonstrably erroneous success oracles now
require rejection, with every original input retained. The implementation also
fixes shared contextual-conversion and assertion-message defects found while
extending the group. This is an implementation handoff, not full PA29 completion.

## Reference corrections and implementation

`32e2430a` corrects three exit-status sidecars. The
[proof and bundle record](reference-corrections189.md) derives two undefined-class
failures from N3485 [temp.inst]/1, /5, /7 and the false-trait failure from
[expr.unary.op]/9 and [dcl.dcl]/4. It explicitly addresses reserved spellings and
the spec's prohibition on library-name shortcuts. Compiler agreement is only
corroboration. Real definitions supplied by personal headers make every original
fixture compile, link and run without changing its source or assertions.

`4ed34a87` fixes a related language defect: the old static-assertion path tested
constant object identity instead of executing its contextual boolean conversion.
It could accept a false, missing, nonconstant, deleted, private or ambiguous
conversion. The shared conversion owner now selects and checks the conversion;
constant execution consumes that same record. Pointer and string conditions work,
dependent conditions wait for demand, and fixed conditions retain definition-time
checking. Assertion messages must be string literals; failure diagnostics include
their basic characters, including content after an embedded NUL. Message-less
assertions retain the existing hosted extension.

`e08707fc` extends this work through concrete `noexcept`, conditional `explicit`,
constructor-template specialization and deduction-guide conditions. Concrete
exception specifications and explicit conditions previously bypassed conversion
access checks. They now use the existing typed conversion validator with the
declaration's lexical access scope, and constant evaluation is explicitly manifest.
Valid private conversions remain available in friend contexts. No synthetic
frontend expression or library definition is introduced.

`383fae65` completes the constructor-template boundary: an invalid contextual
conversion or nonconstant explicit condition returns a compact failed result and
memoizes the failed specialization, allowing another constructor to win. Errors
inside demanded definitions continue to propagate. This follows the explicit-
specifier immediate-context rule in [N4868 temp.deduct/8](https://timsong-cpp.github.io/cppwp/n4868/temp.deduct#8)
for PA29's required conditional-explicit extension. The runtime control exercises
private, deleted, absent, ambiguous and nonconstant conversions plus true/false
copy-initialization behavior. A separate negative keeps the demanded-definition
boundary covered.

## Owners, data flow and complexity

| Owner | Published facts and consumers | Work and lifetime |
|---|---|---|
| Parser | Existing literal identity on the assertion's message node; malformed messages fail before semantic demand. | One token/literal-kind check; no reparsing, extra spelling copy or new node. |
| Assertion declaration | Existing expression and conversion records hold the selected contextual conversion; constant execution computes the actual boolean. A ready assertion is not checked again. | One ordinary conversion resolution per demanded occurrence, following relevant candidates. Existing evaluator limits apply. Constant-size successful storage per assertion; failure rendering is linear in message code units. |
| Exception/explicit conditions | The existing selected conversion is validated for deletion/access using the owning declaration scope before constant execution. Template constructors and guides supply that scope explicitly. | Existing monotonic exception/specialization demand, query identities and conversion records. No new cache, invalidation policy, global retry, token replay or retained optimization body. |
| Template definition/member demand | Class identity is `(primary, canonical arguments)`; completing a selected definition publishes members, not unrelated bodies. Qualified member facts are cached only for complete classes. Missing definitions cannot synthesize members; initializer facts preserve source-defined values. | Existing flat identity indexes, parent-linked substitution frames and TU arenas. Work follows demanded declarations, inheritance/lookup edges and source nodes. This owner needed no spelling-specific implementation. |
| Lowering/native emission | Recorded selected declarations/conversions feed typed LowIR and ELF. Compile-time-only conversions and dormant invalid member bodies have no emitted call/body. | No new lowering pass or ABI representation. Function-local lowering and backend storage retain their existing release boundaries. |

For a concrete source-to-object trace, `defined-conversions.cpp` creates
`probe<character>` declaration identity before the primary `traits` definition.
Its later `int_type` demand completes that definition and selects the actual
alias. Calls to `to_char`/`to_int` instantiate only the selected bodies, record
the constructor/conversion function, and lower directly to validated LowIR and
host-linkable ELF. `dormant()` never appears in the object. The `traits<int>`
explicit specialization supplies its distinct `long` alias and adjusted runtime
conversion values; no rendered type/name selects between them.

The assertion trace starts from the source operand, resolves `operator bool`
through the ordinary conversion candidates, checks access/deletion, and stores
one conversion record on the assertion. The constant evaluator executes that
function with the actual receiver. A false result reaches lazy message rendering;
a true result completes the assertion with no runtime conversion symbol.

## Validation and remaining boundary

The [evidence manifest](../student.tests/pa29/evidence189/manifest.json) binds the
final sources, frozen binaries, coverage, checks and measurements.

- PA1–28: **4538/4538**, exit 0.
- `make test-pa29`: **398/403**, exit 2; original failures **8 → 5**.
- Root through PA29: **4936/4941**, exactly those five failures.
- File audit: pass with four inherited header-division warnings.
- **221 explicit control commands**: semantic acceptance/rejection, host link/run,
  seven validated LowIR outputs and symbol inspection. The entry compiler accepts
  eleven invalid assertion controls and rejects the valid assertion composite.
- All **403** course inputs and discovery/comparison rules remain intact. Of
  **1,707** inherited contract/harness paths, only the three documented
  exit-status sidecars differ.

Original fixture/definition controls run with both host compilers. One additional
member-template conditional-explicit positive uses Clang for corroboration because
the installed GCC rejects that extension's explicit-bool conversion. A demanded-
definition negative uses GCC because Clang suppresses the error in this extension
context. Both remain fully checked by this compiler. No host outcome overrides a language/contract rule.

[Performance189](performance189.md) preserves preliminary and final observations,
reports all four dimensions and distinguishes necessary semantic work from an
optimization claim. No optional transform or new growth/work allowance is added.

This group now covers declarations without definitions, actual primary/explicit
definitions, source-defined trait values, deferred member bodies, substitution,
contextual constants, access, deletion, message syntax and generated behavior.
The remaining implementation failures need **binary128/half representation and
ABI (three)** and **executable vectors/type-operand intrinsics (one)**. Those
require numeric storage, operations, typed IR and native ABI work; another
template-demand or boolean-conversion adjustment cannot supply them. The fifth
failure is the independent **nested-template ABI-tag** contract question retained
from implementation176. It is still counted and not waived.

The compact plan preserves Stage base and Last reviewed commit. Ralph must review
these corrections and changes; full-stage/root-through success and whole-stage
audit remain necessary before PA30. Earlier audit markers and evidence remain.
