# Nondependent result lookup correction

Pinned reference bundle: `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.
Entry: `ef0e43c0f1c1792590ca3e21b49b3e6a82bbd5f3`. The downloaded bundle is
unchanged. Only `tests/general/300-function-template-result-first-lookup.ref.exit_status`
changes from success to failure; its input, informational LowIR/stdout and all
comparison rules remain unchanged. The original 420 inputs still run.

[Reducer](../student.tests/pa18/signature79_reducer.cpp) removes the unrelated
qualified aliases and retains the two declarations and ambiguous call.
The proof uses [N3485](../doc/n3485.txt):

- §14.6 [temp.res]/9 and §14.6.3 [temp.nondep]/1 (lines 18951 and 19439 onward):
  nondependent names bind at their use. Neither `select(0)` operand contains or
  depends on `T`. The first operand sees only `select(double)`; the second also
  sees `select(int)`, whose identity conversion is better than conversion to double
  under §13.3.3.1 [over.best.ics] and §13.3.3.2 [over.ics.rank].
- §7.1.6.2 [dcl.type.simple]/4 (lines 8595–8601): the two prvalue operands make
  their `decltype` types respectively `long` and `int`.
- §1.3.18 [defns.signature.templ] (lines 831 onward) and §14.5.6.1
  [temp.over.link]/6 (lines 18613 onward): return type is part of a function
  template's signature. These are distinct function templates, each deducing
  `T=int` for `result(0)`. Neither has a better argument conversion or is more
  specialized, so §13.3.3 [over.match.best] requires ambiguity.
- The first-declaration rule in §14.5.6.1/5 (lines 18596–18608) applies to
  **dependent names**; it cannot turn these different nondependent result types
  into a redeclaration. An unevaluated `sizeof` operand still requires valid
  overload resolution.

The unchanged course input has the same ill-formed call in its final assertion.
Its earlier `rooted` and `qualified` assertions remain exercised as positive
personal controls, as do dependent-name first-declaration cases with renamed
heads, bodies, defaults, addresses, constexpr and member-template consumers.
The correction preserves each behavior obligation; it does not make the invalid
call compile or suppress a required check. Compiler observations are supporting
evidence, not the proof: [record](../student.tests/pa18/loop79-reference.json).
