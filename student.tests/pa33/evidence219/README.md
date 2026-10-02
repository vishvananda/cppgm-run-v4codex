# PA33 implementation 219 evidence

Implementation is cumulative from `676b6e328c7a05334b15f1c9ee1ec30c783e9aff`.
This is an implementation handoff, not the independent whole-stage audit.
The stage base and last-reviewed markers in `pa33/plan.md` are unchanged.

## Ownership and bounds

- The native level travels from `lowir2native` invocation or `cppgm++` options
  through compile/object/image to the function Selector. PA32's typed optimizer
  still precedes source/object native lowering. O0 keeps existing placement;
  O1–O3 share the bounded PA33 policy. No textual production transport exists.
- `global_placement.cpp` consumes typed definitions/uses/phi edges and the shared
  fixed-clobber census. Only single-definition scalar integer values qualify.
  Each admitted value gets a unique register reserved for the entire function,
  including backedges and deferred parallel phi transfers. At most seven
  registers are admitted, caller registers only when parameters/hidden results
  and all fixed effects exclude interference. EH functions retain conservative
  storage. Ranking favors repeated consumers. Complexity is O(I+V log V), with
  O(V) dense temporary storage; no evictions, retries, cloned blocks or IR growth.
  Existing selection bounds spill work by consumed operands. At most five
  preserved-register save homes are needed. Other values keep ordinary placement.
- Control cleanup compacts each function once, removing fallthrough jumps and
  reversing a final conditional/jump pair when the true target falls through.
  It preserves labels and surviving debug provenance, never reorders effects,
  and has zero code growth. Bulk register operands mean addresses, consumed as
  such by both MIR inspection and encoding. Integer Boolean returns no longer
  request the O0 conversion scratch frame at optimized levels.
- Direct strlen admission consumes typed runtime identity, exact call signature,
  fixed arity, direct ptr argument, i64 result, readonly/no-unwind/default-return
  and default-query facts. The call carries `strlen_prefix=16` in consumed MIR.
  Encoding checks the remaining bytes in the current 4K page before probing;
  it calls the original target on page-boundary or no-zero fallback. Every path
  has the ordinary call clobbers. At most eight sites/function and 128/unit;
  each probe adds 53 bytes (a conservative 64-byte reservation gives <=512 bytes
  per function and <=8192 per unit). Admission and emission are O(1)/call.
- Unused memcpy results admit REP only with canonical identity, exact fixed
  ptr/ptr/i64 ABI, noalias pointers, no unwind and ordinary effects/return/query.
  The existing bounded parallel ABI move scheduler captures inputs; a count
  move and `copy_bytes_dynamic` consume rdi/rsi/rcx. Used results and unmarked
  functions retain calls. This required control applies at O0 as well. No
  mutable cache or cross-function body retention is introduced.
- Hosted object spellings do not identify builtins. Explicit LowIR serializes
  `builtin=strlen|memcpy|fill_bytes` when the legacy private object marker cannot
  preserve the typed identity. The reader validates the function-only field,
  rejects duplicate/unknown/conflicting identities and retains existing private
  markers. Signature/effect gates remain independent. This closes byte-for-byte
  object replay differences without recognizing hosted names as semantic keys.

## Reproduction

Run `python3 student.tests/pa33/check_native.py` for four-level runtime checks:
cyclic parallel phis, six-parameter call pressure, reversed copy arguments,
zero/small/large copy sizes and every tested string length/alignment ending at a
protected page. `check_budgets.py` verifies 8/128 prefix caps at all levels,
retained ordinary calls and incompatible signature/effect boundaries.

`performance.py OUT ENTRY_CPPGM FINAL_CPPGM` freezes generated inputs, checks
results and hashes, and records A/A followed by six ABBA blocks for compiler
wall time/peak RSS and executable runtime separately. Inputs prevent dead or
constant-folded timing. `measure_common.py` uses the unchanged PA32 fixed
benchmarks at the same O0/O2 levels and the compiler-owned folding component.
Host C++ only builds benchmark drivers and links student-generated objects.

All 756 final and 168 rejected-experiment observations are retained in
this directory's JSON records. Full logs, frozen binaries, generated inputs
and objects remain under `$RALPH_ARTIFACT_DIR/pa33-219`; the final binding names
and hashes them. Intermediate runs overlap pinned checks or exhibit A/A noise;
all paired extrema are retained. No compiler speedup is inferred from noisy
ratios near one. Required checks are rerun under normal settings; the one
restricted-CPU PA3 timeout is preserved as an unsuccessful observation.
