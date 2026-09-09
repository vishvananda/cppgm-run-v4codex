# PA11 checkpoint evidence protocol

Freeze the stage-base compiler (a97e14d4, SHA256
735577050dd7757fad4b71577ac153656bf87d7d986a0f9b30a326c40ea098f7)
and the committed constructor/member implementation before timing. Both use the
same g++ GNU C++11/O3 build and course batch runner. No builds or tests run during
timing. Preserve all earlier PA performance records and every new observation.

Reuse the nine fixed compiler inputs and three long checked executable inputs
from `../pa10/benchmark.py delta`: calls/control, memory/floating, static references,
and inherited template semantics at both input scales, plus 8000 references.
The protocol is four A/A calibration observations followed by two ABBA blocks,
pinned to one available CPU. Check byte equality or the unchanged course
comparison before timing. Capture external wall latency and peak RSS separately
from one telemetry run. Retain binary/input/output hashes, native-backend hash,
compiler .text and the sectionless executable payload metric. Runtime compilation
is outside execution timing. Inputs with less than 20x startup are diagnostic;
no speed claim follows from those observations or from changed IR size.

Add PA11 constructor scaling and runtime observations with only the correct new
compiler: the stage base does not implement these constructors and is unsuitable
as an equivalent A/B baseline. Use fixed generated source sizes, stable input
hashes, A/A observations, typed LowIR validation, checked native results and phase
work counters. For the class-only runtime with no static data, the supplied
backend's sectionless payload after entry is the same text proxy used by PA10.
This measures necessary new behavior, not an optimization benefit.

Acceptance: PA11 requires O0 class semantics, bounded class-layout/action demand,
and proportional lowering; there is no mandated numeric speed limit in its
handout. Inherited 1.10x latency, 1.20x RSS +16 MiB, +128 KiB compiler text and
4x-input 5.5x-time/5x-RSS targets remain diagnostics under spec.md's stage-scoped
rule. Investigate avoidable regressions; retain correct semantic costs and all
measurements. No optional optimization is being proposed and no executable
speedup is claimed. Templates beyond inherited PA7 semantics, native selection,
allocation and ELF output remain owned by later stages.
