# PA26 explicit checks

Run `python3 student.tests/pa26/controls.py`. The harness compiles student sources
with this compiler and host-only helpers with g++, then links and executes the
objects. It covers incoming/outgoing EH, large frames, termination, allocation
and builtin aliases, empty/effectful implicit destruction, 140-entry LSDA action
tables, selector identity across functions, and explicit object format selection.
Generated files default to `/tmp/pa26-142/controls`.

`dump.cpp` is an inspection adapter: link it against cppgm++'s frontend source
set, then pass one source filename. It prints the production host LowIR after
all semantic and lowering work, without making text part of the object pipeline.

Host output follows the [Itanium EH ABI](https://itanium-cxx-abi.github.io/cxx-abi/abi-eh.html)
(landing register payload, personality, throw/catch/resume APIs) and the
[LSB exception-frame format](https://refspecs.linuxfoundation.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/ehframechpt.html).
The writer uses indirect PC-relative personality/type references and actual
machine offsets. No reference correction is made.

At this implementation boundary, `.obj` keeps PA25's private object format;
other output names select host ELF. `--object-format=elf` and
`--object-format=private` override the extension. Host EH objects use the external
host linker specified by PA26. The private driver/runtime remains available for
PA25 compile/direct/mixed linking. Full integration of the host ELF link path
into that private linker is unfinished and must be reviewed before stage closure.
