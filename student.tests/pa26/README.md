# PA26 explicit checks

Run `python3 student.tests/pa26/controls.py`. The harness compiles student sources
with this compiler and host-only helpers with g++, then links and executes the
objects. It covers incoming/outgoing EH, large frames, termination, allocation
and builtin aliases, empty/effectful implicit destruction, 140-entry LSDA action
tables, selector identity across functions, and explicit object format selection.
Generated files default to `/tmp/pa26-142/controls`.
Run `python3 student.tests/pa26/native_inspection.py` for the demanded-template trace.

`dump.cpp` is an inspection adapter: link it against cppgm++'s frontend source
set, then pass one source filename, optionally followed by `--mir`. It prints the production host LowIR after
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

`python3 student.tests/pa26/validate.py` records the exact required checks plus
controls/inspection. At this incomplete handoff it verifies the documented
29/30 result and the one remaining required header case; it does not waive that
failure or replace the root suite's nonzero exit status.

Performance observations and interpretation are in [performance142.md](performance142.md).
The candidate and accepted frozen binaries remain under `/tmp/pa26-142`; their
hashes, generated input hashes, every A/A and ABBA observation and final validation
are retained under `evidence142/`. The inspection helper is named
`native_inspection.py` to avoid shadowing Python's standard `inspect` module.

Implementation143 header controls: `python3 student.tests/pa26/header_controls.py`.
These exercise configured host include roots, user search/macro precedence,
namespace and parameter attributes, explicit function/object asm labels,
block-scope function linkage/visibility, and the SysV AMD64 `va_list` shape.
The last follows the [x86-64 psABI, variable arguments](https://gitlab.com/x86-psABIs/x86-64-ABI/-/blob/master/x86-64-ABI/low-level-sys-info.tex).
Compiler build configuration probes only host target metadata/header roots;
compiling a user input never invokes host preprocessing or code generation.
Unsupported host floating extensions and language-feature macros are not
advertised. GNU compatibility selects conservative header paths. No header or
library body is replaced. Full `<string>` remains in progress.
