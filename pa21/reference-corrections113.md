# Failed scalar-new initialization (audit 113)

Revision **pa21-failed-new-113** corrects the inherited PA12 reference
`300-class-new-expression-default-constructor.ref`. Bundle source
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, SHA256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
The [reconstruction](../student.tests/pa21/reference113.py) retains the original
bytes at `f3af4630` and applies three exact replacements. It adds a saved
allocation address and exceptional deallocation continuation around the
external constructor. Normal construction, the result check, sources, statuses,
comparison rules and coverage are unchanged. Student output is not an input.

The fixture already is a reducer: `struct S { S(); };` followed by `new S()`.
The external constructor has no nonthrowing specification. The reference calls
it after allocation without any exceptional deallocation. C++11 N3485
[expr.new]/18–21 ([local text](../doc/n3485.txt:6519),
[C++11 draft](https://timsong-cpp.github.io/cppwp/n3337/expr.new#18)) requires
matching deallocation if initialization throws, with the original allocated
pointer. Global `operator delete(void*)` is the matching function here.
An unavailable constructor body cannot justify removing that obligation.
The inherited PA12 scalar-new subset and PA21 exception construction semantics
compose; the fixture's unresolved constructor is not a proof of no unwind.

The [execution reducer](../student.tests/pa21/reference113_execution.py) changes
only the entry symbol for host invocation and supplies an external `S::S()`
that throws `int`. Replacement global allocation/deallocation functions count
outstanding storage. The original leaves one allocation live; the corrected
reference releases it before the caller's handler runs. The standard establishes
the obligation; this execution corroborates it independently of student IR.
