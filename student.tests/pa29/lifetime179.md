# Parameter lifetime control

The first `array-default-lifetime.cpp` counted `Item` constructor-body entries
minus `Item` destructor calls and expected zero. The reduced original is retained
as `array-default-original.reproducer`; the original failing student run (exit 1)
and successful GCC run are retained in preliminary-expanded-controls.json.
This was an overconstrained personal test, not a course/reference correction.

C++11 N3485 [expr.call]/4 ends parameter lifetime when the function returns;
[except.ctor]/2 destroys completed subobjects when initialization exits by an
exception. The body having run does not establish successful return of its
constructor. [CWG1880](https://cplusplus.github.io/CWG/issues/1880.html) documents
the callee/caller destruction distinction and its ABI consequences; P0135R1
made the choice implementation-defined. This compiler's existing callee cleanup
can throw before the copy constructor returns; GCC's caller cleanup throws
after that return. The original counter is consequently not a portable leak
test. We retain the callee convention, without changing any ABI/lifetime owner.

The revised test tracks a completed `Guard` member. It requires destruction of
all completed members of the interrupted copy and of all earlier array copies,
and requires all ten source objects to survive until their scope ends. The
additional `array-default-temporary.cpp` binds a default temporary to a const
reference: N3485 [class.temporary]/5 keeps that temporary until the full-expression
ends. It verifies cleanup of the completed sixth copy when that temporary throws.
Both checks have the same required result on this compiler and GCC. No failing
course case or required coverage was removed.
