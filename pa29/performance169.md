# PA29 implementation169 performance and design evidence

Code: `3b670e56`; inspection/performance harness: `9772ca29`; PA29/O0.
This change implements required block-pointer declarations and invocation, plus
related type, constant-value and exception facts. It adds no optional optimizer
pass and claims no speedup. The [manifest](../student.tests/pa29/evidence169/performance-manifest.json)
freezes entry/final binaries, flags, scripts, source hashes and CPU affinity 0.
Compiler bytes: **4,105,424 → 4,109,760** (+4,336 bytes).

## Protocol and equivalent existing workloads

[All common observations](../student.tests/pa29/evidence169/common-performance.json):
four A/A observations followed by six ABBA blocks per workload/mode, **224 total**.
Compiler wall latency and `/usr/bin/time` peak RSS are measured separately from
host linking and checked executable execution. No builds or tests overlapped
measurement; documentation and external scheduling were uncontrolled. All outliers
remain. The fixed inputs cover 2,400 demanded template specializations, loops,
calls, memory, floating arithmetic, exception handling and 1,200 unused functions.
Self-hosting remains PA34-owned.

Every A/B object and executable is **byte-identical**. All existing non-time work
counters are identical across every sample, including parsing, semantic facts,
substitution and lowering/native work. RSS remains an observation, not a work
counter. The [counter assertions](../student.tests/pa29/evidence169/scaling-counters.json)
record these checks. Times below are median seconds; paired ratios are medians of
each ABBA block's B/A means, followed by their complete range. RSS is maximum KiB.

| Workload | Compile A/B s | Paired compile [range] | Compiler RSS A/B | Runtime A/B s | Paired runtime [range] | Text A=B bytes |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.2643/0.2650 | 1.0044 [0.9754–2.6837] | 28992/29160 | 0.0541/0.0547 | 1.0020 [0.9841–1.0398] | 151633 |
| floating | 0.2899/0.2506 | 0.8156 [0.5252–0.9895] | 29008/29164 | 0.0581/0.0580 | 0.9988 [0.9923–1.0192] | 151474 |
| exceptions | 0.2799/0.2799 | 1.0032 [0.9782–1.1819] | 29012/29072 | 0.2539/0.2539 | 1.0061 [0.9799–1.0330] | 151781 |
| pruning | 0.2112/0.2092 | 0.9970 [0.9685–1.5083] | 35460/35624 | 0.0534/0.0531 | 0.9951 [0.9751–1.0436] | 151633 |

| Workload | A/A compiler range s | A/A runtime range s |
|---|---:|---:|
| memory | 0.2708–0.3121 | 0.0534–0.0554 |
| floating | 0.1651–0.1718 | 0.0491–0.0500 |
| exceptions | 0.2800–0.4088 | 0.2537–0.2603 |
| pruning | 0.2131–0.2261 | 0.0525–0.0569 |

Compiler invocations contain substantial scheduling spread, including the memory
B outlier and uneven floating A/B observations. The apparent floating reduction
is not a demonstrated compiler speedup. Identical executables also show runtime
spread. Unchanged work/output and the paired observations establish no repeatable
avoidable regression; no optional transform is being accepted on noisy timing.

## Affected capability and scaling

[All affected observations](../student.tests/pa29/evidence169/affected-performance.json):
**48 samples**, eight per size/mode, plus six launcher observations. Entry rejects
all three affected sources, so no correct A implementation exists for this input.
These are necessary capability costs, not speedup comparisons.

Each input demands N functions taking a block pointer. A native block record
carries a runtime capture; the hidden receiver reaches its entry, and each caller
adds its template argument's low bits. The program checks all N demands and then
20 million calls with argc-derived inputs against an independently computed sum.
Neither entry selection nor the runtime checksum is constant/dead work.

| Demands | Compile median [range] s | Compiler RSS KiB | Runtime median [range] s | Runtime RSS KiB | Text bytes |
|---|---:|---:|---:|---:|---:|
| 600 | 0.0630 [0.0618–0.1020] | 15688 | 0.1511 [0.1494–0.1540] | 1760 | 41424 |
| 1200 | 0.1181 [0.1159–0.1191] | 23988 | 0.1497 [0.1488–0.1505] | 1760 | 82224 |
| 2400 | 0.2341 [0.2281–0.2811] | 40584 | 0.1501 [0.1486–0.1509] | 1884 | 163824 |

Launcher median **0.0049 s** [0.0048–0.0056]; shortest median runtime is **30.4×** launcher median.

Every compile sample has exactly N specialization/body transitions, N+3 checked
bodies, N+2 object-use facts, 12N+217 tokens and 18N+387 parsed nodes. The fixed
block signature needs zero type substitutions in this family; nondependent types
and call facts are shared. Separate positive controls exercise dependent return
and argument substitution, partial deduction, SFINAE and expression queries.
Runtime remains roughly constant while compiler storage and produced text track N.
The data supports bounded work; it does not establish an exact timing exponent.

## Ownership, budgets and validation

The parser retains `^` as a declarator operator. Semantic formation validates its
function child and interns a distinct `BlockPointer` TypeId. Invalid direct forms
produce diagnostics; invalid substituted function types use the existing compact
failure/state machinery. Declaration scope/name helpers keep the owning function
inside the file-audit limit without changing declaration semantics.

`invoke<T>` retains one source body. Existing immutable substitution frames create
only dependent facts. Concrete, fixed-template and type-query call owners store
the canonical block signature on their sparse TU-owned ObjectUse record. They
record argument conversions once. Lowering evaluates the callee once, loads the
invocation pointer at byte offset 16, and passes the block reference after any
hidden result parameter. Its cached signature is keyed by the distinct block type,
so ordinary indirect-call signatures cannot collide. Argument preparation is
O(arity); each invocation adds one byte projection and one load. The existing
backend handles variadic registers, aggregate returns, references and unwind.

The PA9 graph consumes the type as a vendor-qualified function type, producing
`U13block_pointerF...E`; its existing fact reader/writer roundtrips that identity.
Block exception objects use fundamental-type RTTI, preserving their distinct
mangled identity. This follows the [Clang block language specification](https://clang.llvm.org/docs/BlockLanguageSpec.html),
[block ABI](https://clang.llvm.org/docs/Block-ABI-Apple.html) and
[Clang RTTI implementation](https://github.com/llvm/llvm-project/blob/main/clang/lib/CodeGen/ItaniumCXXABI.cpp).
No source replay, synthetic syntax, textual transport, name-based call recovery,
global retry or process-global cache was introduced. Source/type/call facts live
in existing TU arenas; temporary argument sequences and MIR keep existing local
release points. Type/signature equality and completed lookup remain O(1) average.

[29 explicit controls](../student.tests/pa29/evidence169/controls.json) cover scalar
storage, null constants, cv, aliases, function-pointer distinction, templates,
queries, single callee evaluation and required rejections. Cross-Clang controls
exercise captured blocks, scalar/floating/aggregate/reference results, varargs,
throwing entries and block exception transport. **60 inspections** cover validated
and lossless LowIR, independent native execution, MIR, symbols, disassembly,
telemetry equality, all four required block fixtures and standalone PA9 facts.
Inherited controls168 **52/52** and controls167 **45/45** pass. The current handout
has no additional debug-line gate; no debug behavior was changed or claimed.

An optional by-value catch in the host helper crashed even when both TUs were
compiled by Clang. The [host-only reducer](../student.tests/pa29/pending169/clang-block-catch.cpp)
and [observations](../student.tests/pa29/evidence169/host-diagnostic.json) retain the
Clang 21.1.8/libstdc++ limitation: its catch uses the exception-storage pointer as
the block reference. The helper now catches by reference. Student value catches,
host-thrown/student-caught values and student-thrown/host-reference-caught values
all execute correctly. No required fixture, oracle or comparator was changed.
Earlier preliminary controls and file-audit findings are retained in evidence169;
final acceptance uses the final binary and final required/control runs.

Optional optimizer work/growth budgets remain **zero**. The extra invocation work
is required by the block ABI. Existing evaluator step/depth bounds, native size/
frame/alignment limits and course timeouts remain unchanged. Historical blanket
15% latency/RSS and zero-growth targets stay self-selected diagnostics under
spec §9; their measurements remain available. Broader hosted runtime, optimization
and self-hosting retain PA30–34 ownership. Implementation handoff does not waive
independent architecture or whole-stage review.
