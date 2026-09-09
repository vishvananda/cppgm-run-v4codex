# PA11 implementation

Stage base commit: a97e14d49c7edfc7acc115b974ab667cc90480db
Last reviewed commit: a97e14d49c7edfc7acc115b974ab667cc90480db

Target: PA11 full-stage. Phase: implement. Goal remains active.

## Design and remaining groups

Extend the integrated syntax/semantic graph and typed PA10 lowering. Stable
EntityId/TypeId own layout, selected members, constructor actions and demand.
Rare object-use facts have a TU arena; temporary initialization paths die after
lowering. No text transport, fake AST calls, host compilation or fixture logic.
PA12 value transfer and PA13 polymorphism remain separate extensions.

| Owner / group | Data flow and complexity | Remaining validation |
| --- | --- | --- |
| Class facts / member lowering | Cached field/base layout, recorded object argument and cv overload choice; O(fields + selected bodies). | empty-base collisions, remaining conversion/member shapes |
| Semantic construction / typed initialization | Selected constructor/defaults, ordered scalar/reference/base/member actions, memoized helper triviality; O(candidates + actions + output). | aggregate brace elision, arrays, union/volatile/zeroinit boundaries |
| Lifetime and control flow | Next: typed destruction actions and interned lexical cleanup tails, including loop/goto/return context. O(actions + distinct cleanup states). | destructor, arrays, globals/TLS, shared cleanup and inline-policy controls |
| Scope lookup / conversions | Indexed lexical/base/ADL edges, selected conversions recorded once. Work proportional to required candidates/edges. | access/friends/ADL/operators, inheriting constructors |
| Layout / lvalue facts | Next: bit width/sign, storage, requested alignment and union/volatile boundaries. | bit-fields, alignas, packed and metadata cases |

## Performance evidence

[Checkpoint evidence](checkpoint.md) and the frozen
[protocol](../student.tests/pa11/performance-protocol.md) retain all measurements.
Nine common compiler output pairs and three native executable pairs are byte
identical. Compiler text +33664 bytes; common peak RSS +5600 KiB maximum.
The template timing spike is preserved; same-binary follow-up does not reproduce
it. New constructor source growth 4x gives 4.03x latency, 3.68x RSS and exactly
4x action/demand work. Checked 96M-iteration runtime: 0.64312 s, 235-byte text proxy.
No optimization benefit is claimed. Inherited numeric targets are diagnostics,
not extra PA11 gates; correctness, complexity and coverage remain mandatory.

## Handoff ledger

- Entry a97e14d4: authoritative baseline evidence (previous turn classified as
  progress), 43/302; 259 failures. Earlier suites and file audit passed.
- 4113c34d: member ABI/this, cv selection, reference fields, typed projections,
  static members and unreachable; checkpoint 84/302.
- 17897014: constructor selection/defaults, explicit copy-list rejection,
  ordered member/base/DMI actions, nested initialization destinations, namespace
  startup, compact object facts and measured demand closure: 156/302.
- Final validation: 113 original fixture failures fixed, zero newly failing
  original fixtures; unchanged coverage/comparator/references. PA1–10 1025/1025;
  file audit exit 0 (advisory counts Analyzer declarations as body lines).
  Both explicit personal programs validate LowIR and execute with exit 0.
- Incomplete boundary: the scalar/reference and single-base constructor group
  is finished. Further array construction needs a shared lifetime owner for
  partial construction, reverse destruction and equal cleanup suffixes across
  loop/goto/return/global/TLS paths. Adding constructor calls alone would leave
  observable cleanup incorrect. That larger control-flow group is next; the
  full-stage objective is unchanged and 146 current failures remain.
- Continuation at cd054d80: previous turn is progress; verified clean tree and
  reran turn baseline (156/302). Extend lifetime owner to semantic lexical states,
  destructor/subobject demand, normal and exceptional exits, and shared return/
  unwind suffixes. Then consume those states for array construction/destruction.
  Complexity: one lexical traversal, one helper action computation, and one
  emitted continuation per complete (action, tail, terminal, control) key.
- Scalar lifetime increment: destructor demand and reverse subobject actions,
  explicit destructor calls, implicit exception specifications, namespace fini,
  lexical exits and shared return/unwind continuations. PA11 165/302; PA1–10
  1025/1025; file audit passes; three explicit personal executables pass. The
  goto control passes. Performance delta remains to be measured after arrays.
- Array/temporary increment: flat bounded construction loops with partial-prefix
  unwind, reverse nested/loop destruction, member-array suffix cleanup, scalar/
  enum pseudo-destructors, static member lifetime, temporary member-call storage
  and full-expression cleanup, parsed noinline/always_inline policy. Five
  personal programs validate and execute. PA11 172/302, no prior fixture losses;
  all four PA11 controls pass. PA1–10 remain 1025/1025. Small-array expansion is
  capped at eight total elements; large dimensions use one counter loop.
