# Recursive call lowering and dispatcher stack

PA34's self compiler exhausted the ordinary 8 MiB process stack while compiling
the long stream-expression chain in `semantic/output.cpp`. The first fault was
the prologue of `Procedural::call` after repeated `expression`/`converted`/`call`
frames. It was not a fault in the stream-output implementation itself.

The frozen seed/self binaries and stack trace are retained in
`$RALPH_ARTIFACT_DIR/pa34-221/{stack-baseline,output-divergence}`. A diagnostic
64 MiB stack lets both binaries finish with identical objects and nontiming
work counters. The original-source object's `.text` is 39,499 bytes (46,365
including other read-only sections in `size`'s aggregate column). A/A followed
by six ABBA blocks, pinned to CPU 2, records seed/self medians 3.069/6.093 s,
peak RSS 262,976/279,476 KiB, paired B/A median 1.972 and range 1.424–2.005.
All observations, including the broad A/A noise range, are preserved. The
generated result is a compiler component object, so executable runtime is N/A.

Disassembly isolates the resource difference: the host-built expression
dispatcher reserves 0x520 bytes, while the self-built dispatcher reserves
0x4910 (18,704). The latter includes aggregate, vector, construction and other
unrelated expression paths on every nested call. This is code-quality evidence;
identical outputs and work counts exclude the suspected semantic retry loop.

`lowering/expression_dispatch.cpp` now owns the original debug/invocation scopes,
guard and call dispatch. Other expression paths use `expression_value`. The
recorded semantic form selects the path; no type spelling, input filename or
depth threshold is recognized. Calls retain their existing argument order,
conversions, result construction and cleanup. The dispatcher has a 432-byte
frame in the self-built diagnostic. There is no new heap storage, worklist,
cache, IR transform, code-growth policy or process-resource setting.

The original source now compiles with the original 8 MiB stack and produces
the seed's identical object. `check_deep_calls.py` reproduces the failure with
400 ordinary procedural calls and checks O0/O3 LowIR/object equality and runtime
side effects under that same stack. The final check remains the canonical
PA34 builds, never the diagnostic mixed-object compiler or larger stack.

Frozen before/after dispatcher measurements (`stack-dispatch-measure`) retain
all 28 observations. Paired latency B/A is 1.006 [0.881–1.317], with overlapping
spread; peak RSS falls from 279,320 to 275,052 KiB. Generated object text and
work are identical. The diagnostic compiler's `.text` grows 704 bytes (0.008%);
`size`'s aggregate text/read-only column grows 1,592 bytes (0.017%).
The change fixes stack exhaustion without a measured general speedup or a new
optimization pass. Final common workloads and canonical inception remain the
acceptance checks; none uses the diagnostic stack increase.
