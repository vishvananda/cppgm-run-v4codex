# PA26 explicit checks

Run `python3 student.tests/pa26/controls.py`. The harness compiles student sources
with this compiler and host-only helpers with g++, then links and executes the
objects. It covers incoming/outgoing EH, large frames, termination, allocation
and builtin aliases, empty/effectful implicit destruction, 140-entry LSDA action
tables, selector identity across functions, and explicit object format selection.
Generated files default to `/tmp/pa26-142/controls`.
Run `python3 student.tests/pa26/native_inspection.py` for the demanded-template trace.

Audit144: `writer_audit144.py OUT ENTRY FINAL` compares all PA26 translation
units and five additional layout controls byte for byte and checks write errors.
`writer_performance144.py OUT ENTRY FINAL` measures a fixed large data object
with A/A calibration and six ABBA blocks, including checked runtime and text
size. [Performance144](performance144.md) and `evidence144/` retain the results.
`output_policy144.py OUT COMPILER` checks arbitrary output names against the
handout; it currently fails for `.obj`. The full audit remains open in
[audit144](../../pa26/audit144.md).

`dump.cpp` is an inspection adapter: link it against cppgm++'s frontend source
set, then pass one source filename, optionally followed by `--mir`. It prints the production host LowIR after
all semantic and lowering work, without making text part of the object pipeline.

Host output follows the [Itanium EH ABI](https://itanium-cxx-abi.github.io/cxx-abi/abi-eh.html)
(landing register payload, personality, throw/catch/resume APIs) and the
[LSB exception-frame format](https://refspecs.linuxfoundation.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/ehframechpt.html).
The writer uses indirect PC-relative personality/type references and actual
machine offsets. No reference correction is made.

The known `.obj` default-format gap and independent whole-stage review are
recorded in [the plan](../../pa26/plan.md). PA26 uses the host final linker;
integrating host objects into the private driver/runtime is outside this stage.

`python3 student.tests/pa26/validate.py` runs the prior suites, PA26, file audit,
personal controls/inspection and the through-PA26 report. The current results
are 30/30 and 4283/4283, with unchanged protected fixtures and references.
Current observations and interpretation are in [performance143.md](performance143.md)
and `evidence143/`; historical142 measurements remain unchanged. The inspection
helper is named `native_inspection.py` to avoid shadowing Python's `inspect` module.

Implementation143 header controls: `python3 student.tests/pa26/header_controls.py`.
These exercise configured host include roots, user search/macro precedence,
namespace and parameter attributes, explicit function/object asm labels,
block-scope function linkage/visibility, and the SysV AMD64 `va_list` shape.
The last follows the [x86-64 psABI, variable arguments](https://gitlab.com/x86-psABIs/x86-64-ABI/-/blob/master/x86-64-ABI/low-level-sys-info.tex).
Compiler build configuration probes only host target metadata/header roots;
compiling a user input never invokes host preprocessing or code generation.
Unsupported host floating extensions and language-feature macros are not
advertised. GNU compatibility selects conservative header paths. No header or
library body is replaced. The required `<string>` fixture and all four reduced
conditional paths, heap-backed copies and concatenation now pass.

`python3 student.tests/pa26/intrinsic_controls.py` compiles and executes the
trait, variadic and header-intrinsic sources here, including host va_list
consumption and rejected invalid calls. Type traits use GCC's documented
[type-trait builtins](https://gcc.gnu.org/onlinedocs/gcc/Type-Traits.html) and
C++11 class/initialization rules. `__func__` is a local static constant array;
the GNU function-name aliases currently use the same unqualified name format.
The compiler owns these data and operations; it does not replace library code.

Integer-pack expansion is linear in the generated argument count and diagnoses
bounds outside 0..1048576. GCC 15 headers use `if constexpr` in C++11 mode;
expression conditions are supported as an extension, with discarded template
branches excluded from instantiation and emission. Condition declarations in
this extension currently diagnose unsupported use. `noexcept_parameter.cpp`
checks that exception specifications see constructor prototype parameters.

`include_next` follows the found-directory index, including quoted sibling headers
and duplicate physical search directories; see GCC's
[wrapper-header contract](https://gcc.gnu.org/onlinedocs/cpp/Wrapper-Headers.html).
`auto_decltype.cpp`, `dependent_enum_bound.cpp` and `noexcept_emission.cpp`
cover instantiation-owned local types, delayed enum bounds and exception facts
completed before lowering. Intrinsic queries also cover expect, constant_p,
abort/unreachable, and integral/floating absolute-value calls.

`python3 student.tests/pa26/function_address.py` links against a host DSO and
checks imported function-pointer identity, indirect calls, local addresses and
GOT relocations in a default-PIE executable. `string_paths.cpp` is included in
the intrinsic runner; `partial_cleanup.cpp` checks effectful cleanup beside an
effect-free implicit destructor during constructor failure.
