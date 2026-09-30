# PA27 implementation149 evidence

Code tip: `df5aef0879cb53129cf6b524229559fd4b0a4aa9`.
Entry: `226b88668284e116a4d273627bf4810bd682226e`.
Final compiler SHA-256: `453baae0b401a336074ae51c13f7297cee911edfa03d08ebe5c863cf7ee7f12e`.
Entry compiler SHA-256: `2dd35668617b07ddd6cb5f24c699d12bbaf02bdb92a06e2c186bbcee26e29fa5`.

## Protocol and results

[Common observations](evidence149/common-performance.json) and
[affected observations](evidence149/affected-performance.json) retain every
sample, compiler/input/image hash, flags, host tool version, phase/work counters,
RSS and checked result. Frozen A/B binaries use `-O0 -c --stats`, pinned to CPU 0
on the same Linux x86-64 host. Each supported pair has four A/A calibration runs
and six ABBA blocks, separately for compilation and executable runtime. Host
linking is outside the timed regions. No concurrent build or test suite ran.
Runtime `argc`, input bytes, loops and checked sums keep the work live.

The inherited common inputs cover 2400 template demands, calls, loops, memory,
floating point, exceptions and unused local functions. The new signature input
demands 1000 redeclared function templates with dependent qualified values and
types. The newly supported stream input repeats 200,000 parses of stdin. Its
entry compilation fails, so only final costs are reported (12 compiler and 12
runtime observations); a host-built executable independently checks its result.
Self-hosting remains PA34's scope.

| Input | Compiler seconds A/B | Compiler peak KiB A/B | Runtime seconds A/B | Executable text bytes A/B | Paired compiler ratio (range) | Paired runtime ratio (range) |
|---|---:|---:|---:|---:|---:|---:|
| memory | 0.1579 / 0.1608 | 29008 / 29248 | 0.0533 / 0.0530 | 151633 / 151633 | 1.013 (0.754–1.110) | 0.996 (0.975–1.030) |
| floating | 0.1590 / 0.1588 | 29112 / 28812 | 0.0496 / 0.0496 | 151474 / 151474 | 0.909 (0.528–1.038) | 1.000 (0.928–1.011) |
| exceptions | 0.1568 / 0.1571 | 29068 / 29256 | 0.2559 / 0.2543 | 151781 / 151781 | 1.010 (0.908–1.756) | 0.996 (0.979–1.006) |
| pruning | 0.1977 / 0.2133 | 34628 / 34808 | 0.0531 / 0.0531 | 151633 / 151633 | 1.065 (0.976–1.854) | 1.005 (0.988–1.085) |
| signature | 0.1147 / 0.1631 | 22564 / 22836 | 0.0762 / 0.0850 | 63092 / 63092 | 1.138 (0.973–2.919) | 1.088 (0.971–1.166) |

A/A compiler/runtime ranges, seconds:

| Input | Compiler min–max | Runtime min–max |
|---|---:|---:|
| memory | 0.1571–0.2361 | 0.0525–0.0532 |
| floating | 0.1548–0.1590 | 0.0491–0.0503 |
| exceptions | 0.1560–0.1573 | 0.2508–0.2553 |
| pruning | 0.1962–0.2501 | 0.0536–0.0651 |
| signature | 0.1062–0.1204 | 0.0809–0.0941 |

The hosted input compiles in median **1.0747 s** (1.0509–1.1163), peaking at
**67,568 KiB**. Its checked runtime is **0.5043 s** (0.2750–0.6760), peak
**3,520 KiB**, with **31,873 bytes** executable text and a **198,864-byte** object.
These are costs of newly supported semantics, not an A/B optimization claim.

All five supported pairs produce **byte-identical objects and executables**.
The diagnostic-only final repair also preserves every measured output from the
preceding binary. There are 304 final observations and 304 retained
[pre-diagnostic common](evidence149/pre-diagnostic-common-performance.json) /
[pre-diagnostic affected](evidence149/pre-diagnostic-affected-performance.json)
observations. The rerun was required by the diagnostic repair; no sample was
discarded. Earlier [148 evidence](performance148.md) and its historical inventory
remain unchanged.

Timing is noisy: the signature paired compiler ratio was 1.003 (0.980–1.022)
before the diagnostic repair, versus 1.138 (0.973–2.919) afterward. Hosted runtime
was 0.1717 s (0.1702–0.3668) before it, on the identical executable. These data
do not establish a repeatable speedup or a precise slowdown. The final signature
RSS increase is 272 KiB; the earlier measured increase was 476 KiB.

## Required work and budgets

The signature trace has the same 10,189 tokens, 15,299 parsed nodes, 3010
specialization records, 1000 body transitions, 1001 class completions and 14,042
native instructions A/B. It adds 2000 successful access checks (qualified value
and type per demanded signature), uses 2001 cached type substitutions instead
of one, and reduces uncached type-substitution work from 8018 to 5018. Lookup
work rises from 16,071 to 17,071. The additional facts check each declaration's
own access context; they are required semantic work, not an optional transform.

Budgets are structural and stage-scoped. Access recipes are source-owned,
subtree-pruned and memoized by the full `(frame, recipe)` key, with at most one
completed check per key. Frames retain parent identity; qualified receivers
remain typed. Lexical access context does not demand an unrelated class, while
receiver/body projections still enforce the concrete-context invariant. All
facts and caches are released with the translation unit. Explicit-instantiation
selection uses the existing cached partial ordering with linear winner and
dominance passes over matching candidates. Header probes share include search
and inspect directory candidates without reading header contents. Atomic
signatures are cached by builtin kind and unqualified pointee type; lowering
adds one typed atomic operation and at most one subtraction per call, evaluating
all supplied arguments once. Native emission remains direct typed LowIR → MIR
→ ELF. No optional optimizer or new speculative growth budget was added.

Existing native work limits, constant-evaluation step/depth limits, mandated
correctness and coverage are preserved. Under spec §9, inherited 15% latency/RSS
and zero-growth targets remain diagnostics, as established by audit148. The
necessary access facts and hosted behavior do not create another exit gate.
No optional transform is retained on an unsupported profit claim. O0 is the
current native policy; PA32/PA33 optimization and PA34 self-hosting are later
boundaries, not waived present requirements.

## Semantic controls and reproduction

[Validation](evidence149/validation.json) records PA27 **158/158**, earlier PAs
**4283/4283**, through PA27 **4441/4441**, and file audit passing with four
inherited header warnings. [66 new control commands](evidence149/controls.json)
check preprocessing, builtins, all integer atomic widths, pointer byte increments,
side effects, four-thread contention, alias bases, enum attributes, friendship,
explicit-instantiation ordering, void parameter lists and private-access SFINAE.
[33 inherited signature cases](evidence149/first-signature.json) preserve earlier
lookup and body-parameter semantics, including ordinary rejection diagnostics.
The course fixtures, references, harnesses and comparison rules are unchanged.

Relevant C++11 rules are N3485 [dcl.fct]/4 (sole unnamed void),
[temp.friend]/3 (all friend-template specializations), [temp.deduct.decl]/2
(partial ordering for explicit instantiation), [class.access]/5–6 and
[temp.deduct]/8 (source access and immediate-context substitution failure).
See the [local standard](../../doc/n3485.txt). GNU extensions follow the official
[variadic macro](https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html),
[header probe](https://gcc.gnu.org/onlinedocs/gcc-14.3.0/cpp/_005f_005fhas_005finclude.html)
and [atomic builtin](https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html)
contracts. The atomic implementation conservatively uses sequential consistency,
which that contract permits for dynamic orders and weaker requested orders.
No reference correction was needed.

Reproduce measurements with `PERF_CPU=0 python3
student.tests/pa27/performance147_common.py OUT ENTRY FINAL` and `PERF_CPU=0
python3 student.tests/pa27/performance149.py OUT ENTRY FINAL`. Frozen binaries,
inputs, objects, executables and complete outputs remain under
`$RALPH_ARTIFACT_DIR/pa27-149/`. Final measurements use `validated-cppgm++`;
`final-cppgm++` is the preserved pre-diagnostic binary.
