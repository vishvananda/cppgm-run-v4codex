# Loop 69 implementation handoff

Entry `e09162fa` (**348/420**); implementation `f2a9d7a3`; reference correction
`6073dbc0`. Final **353/420**: five existing failures resolved, none introduced,
all 420 inputs retained. Four are compiler repairs; the fifth is a proved
rejection-oracle correction. PA18 remains incomplete and independent review is
pending. This document ends the implementation group, not the whole-stage audit.

## Owners, data flow and complexity

| Owner | Completed behavior | Bounds and lifetime |
|---|---|---|
| `type_query`, `assignment_operators`, `query_operator` | Assignment syntax retains an operation and typed operands. Substitution performs ordinary member/ADL selection or builtin viability, including implicit class assignment, cv/lvalue restrictions, bit-fields, arithmetic/reference conversions and complete-object pointer arithmetic. Failed candidates return structured query failure. Assignment results retain the modified object's identity. | Canonical QueryId/frame keys reuse existing substitution/fact caches. Candidate work follows actual declaration and operand-conversion sets; deduplicated builtin candidates need only the language-required combinations. No unrelated-scope search. Candidate vectors/indexes release after selection. |
| `operator_call`, `template_operator_facts`, `lowering/expression` | A builtin compound assignment through a class conversion keeps the selected destination conversion and arithmetic type separately. Concrete and retained bodies evaluate the conversion once, load, compute and store through the resulting reference. Pointer sums assigned back to bool use the ordinary standard conversion. | Constant extra facts per selected assignment; linear lowering and no repeated resolution. The arithmetic fact has no extra evaluated argument or object. Semantic conversions live with the TU; lowering values live with the function. |
| `query_destructor`, source member binding and parser | Destructors retain the target TypeId, receiver, lexical context, qualifier and explicit class arguments. Substitution resolves class/scalar destruction, access, deletion, argument count and receiver effects without demanding bodies. Template-id arguments belong to the destructor's class, not to function-template deduction. Destruction without a call is rejected. Concrete member expressions share target lookup. | Indexed lexical/member lookup; existing query graph and reverse completion edges own positive/negative facts. Forward-class failure is invalidated by that class's completion, not by a global epoch or retry. Parser accepts the destructor template-id once; no token replay or fake AST nodes. |
| `default_destructor_facts` | Concrete implicit deletion is a cached validity fact over bases and fields. Query probes consume false without an exception; hard-demand wrappers retain diagnostics. Incomplete class prerequisites and invalid demanded class definitions keep their existing distinct paths. | Each destructor's monotonic property state is evaluated once after required class completion. Work follows its subobject edges. Access remains a use-context check, outside the context-independent property cache. |
| `query_abi`, typed ABI graph/adapters | Assignment operation codes, dependent destructor names and `noexcept` retain their source expression meaning in mangled signatures. The new typed destructor node has local graph validation, text adapter read/write and direct encoding. | Canonical graph IDs, TU lifetime, linear expression/name encoding. No mangled strings are semantic keys; text adapters do not transport production facts. |

Language anchors are N3485 §5.17 [expr.ass]/1,7; §13.6 [over.built]; §5.2.4
[expr.pseudo]; §12.4 [class.dtor]/5,13; §5.3.7 [expr.unary.noexcept]/3; §14.8.2
[temp.deduct]/8. ABI expressions follow [Itanium §5.1.5](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling-expressions)
and the checked-in `doc/itanium-mangling.txt`, including `dn`, `pL` and `nx`.

## Validation and reference proof

- `make test-pa18`: **353/420**, formerly 348. Repaired original fixtures:
  complete-object pointer compound assignment; unrelated compound overloads;
  destructor template-id SFINAE; defaulted member-template assignment/trailing
  result; scalar pseudo-destructor `noexcept` (corrected rejection).
- Required prior-through command: **2609/2609**. PA18 file audit passes, with
  the same three inherited header advisories. No new debug/native inspection
  gate is mandated for PA18/O0; PA9's ABI suite passes in the prior report.
- [Query controls](../student.tests/pa18/query69_controls.py): **58/58**, all
  validating and executing generated LowIR. Includes every compound operation,
  SFINAE fallbacks, class implicit assignment, bit-fields, converted references
  (ordinary, dependent and retained fixed bodies), complete/incomplete pointer
  targets, destructor cv/access/deletion, forward completion, dormant bodies,
  template-id/qualified/base destruction and exception effects.
- Inherited controls: ordering **64**, substitution **33**, conversion **51**,
  address **63**, deduction **56**, audit completion dependencies **5**: all pass.
  Total semantic controls **330**, run explicitly outside the course suite.
- [Course runner](../student.tests/pa18/query69_course.py) validates the four
  repaired valid inputs and executes them (one uses an added empty entry TU);
  it verifies rejection of the corrected fifth input. [ABI controls](../student.tests/pa18/query69_abi.py)
  check four emitted symbols against the published grammar, without a host oracle.
- [Reference correction](reference-correction69.md) provides the reduced input,
  N3485 proof and pinned bundle revision. Only one `.ref.exit_status` changes;
  the original source, empty informational `.ref`, coverage and comparison
  rules are unchanged. The receiver's potentially throwing call makes the
  assertion ill-formed; scalar destruction does not erase that effect.
- [Performance](performance69.md) and [hashed evidence](../student.tests/pa18/loop69-evidence.json)
  retain the frozen binaries, input hashes, checks, all timing observations and
  executable results. No optimization benefit is claimed.

## Concrete handoff boundary

The initial query investigation expanded through assignment selection, builtin
class-reference execution, retained fixed bodies, destructor parsing/binding,
implicit deletion, completion invalidation, receiver effects and source ABI
encoding. All known defects found in that group were resolved and controls pass.

The remaining **43 status failures and 24 LowIR mismatches** are required
unfinished implementation. Braced signature queries have no retained list
operand; existing `ListPlan` inputs are source NodeIds. Supporting those queries
requires a typed list/conversion plan shared by substitution, narrowing checks,
constant evaluation and exception effects. Reusing the concrete list owner by
fabricating source nodes would violate the spec. This is a separate owner/data
model change, not another operator/destructor validity case, and is the concrete
boundary to further related incremental work in this handoff.

Nested aliases, outer/member packs, inherited constructors and out-of-class
contexts still need retained declaration/result ownership. For example the
mutable/const callable reducer passes in the new controls, while the original
`300-dependent-call-operator-implicit-object-const-sfinae` still fails with
`base is not a class`: its missing derived result type belongs to class/alias
substitution, not callable cv selection. Array initialization and result/metadata
LowIR mismatches likewise remain required; executable success is not a waiver
of canonical comparison. No PA19 advancement is authorized.

## Independent review obligations

Preserve Stage base `94dcb8ad21664137e87d574e878c14a4a047348a` and Last reviewed
`3a883d10a27e41d1b126eaef05eaf0b454de1646`. Loops 67 and 68 remain pending review.
Review this completed group's builtin candidate/ranking rules, single-evaluation
reference lowering, destructor property/access separation, completion cache
edges, source ABI identities, reference proof and performance evidence. These
review questions are distinct from the unfinished implementation above; neither
is waived. Ralph's next independent audit must resolve accumulated findings.
