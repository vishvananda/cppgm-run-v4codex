# Nested catch continuation corrections

Bundle source `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, SHA256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
Local revision **pa21-nested-catch-106** corrects two LowIR references. It
preserves every source, exit status, comparison rule, constructor/destructor
call and catch. The [reconstruction](../student.tests/pa21/reference106.py)
reads the untouched stage base, applies four exact replacements, and records
original/revised hashes in the [manifest](../student.tests/pa21/reference106-revision.json).
It does not read student output.

The [reducer](../student.tests/pa21/nested_catch_reducer106.cpp) has two cases:
a type mismatch inside a live automatic object's scope, and the same mismatch
inside an active handler. In both cases the outer matching handler must run
after precisely one `Guard` destruction. The normative proof is N3485
[except.handle] 15.3/3,4,6 and [except.ctor] 15.2/1
([standard text](../doc/n3485.txt)): `int` does not match `long`; a failed
match continues at the dynamically enclosing try; automatic objects since
entry to that try are destroyed in reverse order. [except.handle] 15.3/7,8
and [except.throw] 15.1/4 also require the abandoned inner handler to finish
before the outer handler remains active. These obligations fix the expected
result without relying on compiler agreement.

The LowIR [handler-stack contract](../pa8/lowir.md#handler-stack-management)
requires balanced protected regions. A cleanup-bearing landing pad retains
its protected region so that cleanup may resume; entering ordinary source
catch matching must first retire that region. Forwarding by an ordinary jump
to an outer catch entry must likewise leave the outer try region. The old
references omit these exits. The simple mismatch also omits the outer catch
clause and cleanup marker: host personality selection bypasses the inner
landing pad, losing the live `Guard` destruction. The revision adds only those
clauses and exits; it preserves the existing reverse cleanup and matching code.

Execution corroborates the proof. Through the pinned supplied object backend
and host runtime, the original simple-mismatch reference aborts with an uncaught
`int`; the active-handler reference is rejected for an active protected region
at a function exit. Both minimally revised references execute with status zero.
The student implementation also passes both reduced cases and both full inputs.

A separate supplied-tool limitation remains: `lowir2native-ref` rejects the
unchanged scalar-throw reference with `duplicate native object-symbol label`
because it contains both a declaration and a definition for fundamental RTTI.
The unchanged class-template throw reference independently fails with
`duplicate native symbol` for its `@`-prefixed exception-storage object alias.
The source-EH controls retain that observation and require successful execution
through the supplied object backend linked to the host exception runtime.
This does not change required LowIR comparisons or reference RTTI coverage.
