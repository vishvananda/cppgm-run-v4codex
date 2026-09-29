# Active-handler lifetime correction (110)

Bundle source `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, SHA256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
Local revision **pa21-active-handler-lifetime-110** changes only the exceptional
continuation of `read(Value(11))` in the handler-context reference. Sources,
normal execution, signatures, coverage and comparison rules remain intact.
[Reconstruction](../student.tests/pa21/reference110.py) reads entry HEAD, never
student output; the [manifest](../student.tests/pa21/reference110-revision.json)
contains both hashes and the exact replacement.

The original exceptional edge ends the active catch before destroying its
argument temporary. It can therefore destroy the caught exception object while
that temporary is still alive. The revision destroys the temporary first, then
jumps to the existing complete handler-exit continuation. That continuation owns
both required region exits and the terminal resume. No cleanup is omitted.

The proof is C++11 N3485 [class.temporary] 12.2/3 (full-expression temporaries
are destroyed even when evaluation exits through an exception), [except.handle]
15.3/7–8 (active/currently handled exception), and [except.throw] 15.1/4 (caught
exception destruction at handler exit), together with [except.ctor] 15.2/1
(reverse destruction on scope exit); see the [standard text](../doc/n3485.txt).
The [PA21 cleanup contract](README.md#assignment-boundary) explicitly requires
complete handler-exit and terminal context for shared cleanup suffixes. The
[LowIR handler contract](../pa8/lowir.md#handler-stack-management) gives region
exits their dynamic effect. The call in this reference has the default, possibly
throwing boundary; its exceptional continuation must preserve these ownership
facts. The original fixture's local read body does not exercise the edge, so its
ordinary exit status alone cannot validate this continuation.

The [reduced execution](../student.tests/pa21/reference110_execution.py) retains
`choose` and all its cleanup blocks byte for byte, externalizes `read`, and moves
aside only the fixture main. The controlled read throws a class exception on
value 7 and a long on value 11. The class exception destructor appends 3 to the
same trace. Required destruction order gives **813**: value 7, value 11, then
the caught class exception. The original edge instead gives **741**, ending the
catch between the two temporaries. The supplied object backend and host runtime
produce exit 1 for the original and exit 0 for the revised reference. The
student source-level counterpart independently checks the caught object's
liveness during temporary destruction in `closure110.py`. Compiler agreement
is corroboration; the cited lifetime and LowIR contract rules establish the
required order.
