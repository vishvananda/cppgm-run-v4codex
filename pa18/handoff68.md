# Loop 68 implementation handoff

Entry `047215cf` (**343/420**); implementation `3c78f083`, `e0cc21b4`,
`bd1d7b4c` (**348/420**). Five original required failures are gone, none added,
and all 420 sources/references/comparison rules are unchanged. One additional
case now compiles correctly but still fails its required LowIR comparison.
This is an implementation handoff, not completion of PA18 or its audit.

## Ownership, data flow and bounds

| Owner | Completed behavior | Work and lifetime |
|---|---|---|
| `template_call`, `template_class`, `type_query` | Explicit heads validate supplied arguments, retaining omitted type **and non-type** parameters in an immutable canonical frame. Deduction precedes omitted defaults. Symbolic non-type parameters acquire the substituted declared type (for example `T N` after `T=int`). Array-reference results and address deduction consume the resulting signature normally. | One head-sized symbolic argument slice per uncached partial specialization; existing entity/argument/frame keys own positive/negative declaration facts. Type/query substitution remains cached by complete frame and typed identity. No source replay or full environment copy. |
| `template_call`, `template_packs` | A braced argument is non-deduced. Other arguments, explicit arguments or defaults establish the target before ordinary list conversion. Explicit pack lanes accept lists; unbound list lanes fail as candidate state without calling decay on type zero. | Work follows supplied arguments and required pack lanes. Existing local binding indexes/scratch die with the candidate; canonical signatures and selected conversions live with the TU. |
| `constant_execution` | Empty braced scalar references use their selected list materialization and storage identity, preserving constexpr reference arguments. The scalar zero shortcut applies only to non-reference values. | Consumes the recorded conversion once per constant activation; existing typed activation/storage caches and work/depth limits remain. No new evaluation or lowering policy. |
| `template_deduction` | Try the direct class specialization first. Only after failure, consider reachable base types, including another specialization of the same primary. Reject distinct successful alternatives; discard bindings from failed trials. Repeated paths to the same base type do not duplicate deduction. Selected conversion still enforces access and unambiguous subobject identity. | Traversal uses explicit base edges and a local visited index: O(V+E) plus required matching-argument work. Filter canonical primary identity before copying candidate bindings. No unrelated declaration scan, global cache or broad retry. |
| `conversion` | Array qualification comes from the underlying element, including multidimensional/unknown-bound pointees. Pointer-to-void conversion/composition preserve it. Reference qualification restrictions apply to reference-related types; unrelated converted temporaries (including a string literal converted to a pointer) remain valid. | Per conversion, work follows array dimensions and required base/qualification paths. Existing conversion facts record the result for lowering. No extra type node, persistent map or alternate ABI. |

C++11 anchors in the checked-in [N3485](../doc/n3485.txt):
[temp.deduct.call] §14.8.2.1/1 (braced non-deduction), /4–5 (failed direct
matching and derived alternatives); [temp.arg.explicit] §14.8.1 and
[temp.deduct] §14.8.2 (deduction/default completion); [basic.type.qualifier]
§3.9.3/5 (array cv); [dcl.init.ref] §8.5.3/4–5 (related types and temporary
binding); [dcl.init.list] §8.5.4 and [expr.const] §5.19 (typed materialization).
No reference correction or bundle change is made.

## Validation

- `make test-pa18`: **348/420**. The five repaired fixtures cover array-reference
  returns, array-pointee cv rejection, explicit ADL template-ids, constexpr
  member-template braced arguments and an empty braced array argument.
- Required prior-through command: **2609/2609**. An intermediate string-literal
  reference regression was repaired; its failing log is retained as historical
  evidence, superseded by final validation.
- PA18 file audit: pass, with the same three inherited header advisories.
- [New controls](../student.tests/pa18/deduction68_controls.py): **56/56**,
  executing valid cases through the supplied backend and checking rejections.
  These include all 16 array-cv/void combinations, partial heads, dependent
  non-type parameter types, braced/default/explicit/pack arguments, constexpr
  temporaries, base ambiguity/access, failed-trial isolation and reference
  conversions. They run explicitly, outside the course suite.
- Inherited controls: ordering **64**, substitution **33**, conversion **51**,
  address **63**, all passing on the final implementation.
- [Course execution](../student.tests/pa18/deduction68_course.py) validates all
  five repaired inputs and executes four. The array-reference-return fixture
  intentionally declares but does not define `cast`; its checked contract is
  compilation/LowIR, and the native backend correctly reports the unresolved
  symbol. Executable reference-return behavior is covered by the new controls.
- [Performance evidence](performance68.md) records frozen compiler and runtime
  measurements, code sizes, complete observations and stage-scoped acceptance.
  [Handoff evidence](../student.tests/pa18/loop68-evidence.json) hashes commands,
  binaries, sources, references and artifacts; its verifier checks progress.

## Concrete boundary and remaining implementation

The initial call/array group was extended through partial/default/address
signatures, pack lanes, constexpr materialization, base deduction and array
qualification. The new controls all pass. Remaining **48 status failures and
24 LowIR mismatches** are required unfinished implementation, not review questions.

Further braced **signature-query** work needs a retained typed list expression
and conversions shared by query validation and constant evaluation. The current
query vocabulary has no list operand; changing deduction cannot provide those
facts. Nested alias/member/default/outer-pack failures likewise need their
retained declaration contexts. Those owners require a distinct implementation
group, rather than further changes to this completed concrete-call path.

`300-explicit-template-call-transitive-base-deduction` now selects the intended
base overload and emits valid LowIR, but still mismatches one extra reference
load in the oracle's discarded-value lowering. No credit as a passing test is
taken and no oracle is changed. The other LowIR mismatches involve initializer
representation, selected constructor/result conventions or constant facts.
Earlier suites require constant images where some PA18 cases expect stores;
changing the global small-array policy would break existing coverage.

A [virtual-diamond probe](../student.tests/pa18/virtual_base_pending.cpp), retained
from the expanded experiment, still fails in the **existing base-subobject
conversion/layout owner**. Its source and failed observation are preserved;
virtual-base identity/layout is later PA23 work in the project map, not a
claimed result of this deduction change. Nonvirtual ambiguity remains tested.

## Independent review obligations

Preserve Stage base `94dcb8ad21664137e87d574e878c14a4a047348a` and Last reviewed
`3a883d10a27e41d1b126eaef05eaf0b454de1646`. The pending loop 67 review is unchanged.
Review this increment's partial-frame identity, default timing, base-alternative
binding isolation, array/reference qualification and constexpr temporary
identity, including its performance evidence, in the next independent audit.
These are review questions on completed work; they do not replace the explicitly
unfinished implementation above. Do not advance to PA19.
