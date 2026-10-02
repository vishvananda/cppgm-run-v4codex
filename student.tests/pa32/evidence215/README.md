# Implementation 215 evidence

Code owner boundary: `7dd38d40`; entry `f4f075b8`, whose frozen compiler matches
Audit 214's final binary. This is an incomplete implementation handoff, not an
independent stage audit. No course fixture, reference or comparison rule changed.

`binding.json` binds code, inputs, binaries, evidence and retained artifacts.
Run `python3 student.tests/pa32/ranges_verify.py`. The compact stage plan retains
both review markers. The six newly passing course cases are the partial-slot/phi
fixture, three dynamic fills, and two backward byte-stride loops. Eleven course
failures and three inherited source-debug failures remain.

The semantic owner is the existing function-local CFG/slot/loop analysis:

- Missing slot-entry facts propagate through indexed reverse dependencies.
  A load with mixed known/unknown reaching definitions keeps the home; an
  unresolved early read does not invalidate unrelated later store facts.
- A scalarized empty diamond retires its own edges. Boolean materialization
  retains canonical 0/1 values for arbitrary nonzero conditions.
- Immutable pointer inductions and equivalent twin phis share one update.
  An odd byte stride is coprime to the 64-bit address modulus, so every target
  address is reached before repetition. This proof does not assume object
  bounds, alignment or noalias. Even strides conservatively decline absent a
  congruence proof. An initial dead load is removed only on a straight, pure
  path to an equal-extent write to that same immutable address.
- Exact contiguous byte counts permit constant repeated-byte stores, invariant
  scalar byte values, and byte references. If a reference aliases the written
  range, each write has the original loaded byte and cannot change subsequent
  loads. Zero trips branch around the load and call. Volatile accesses, wider
  loaded patterns, mutable count/address snapshots and unknown strides decline.
- Exported phis and comparisons retain their final values. Phi uses are checked
  at their incoming edges. Fills add the corresponding exit-phi edge explicitly;
  complete rewrites are charged before mutation.

The range proof shares the existing `16*(I+O+E+1)` candidate allowance and
bounded dominance. It adds no instructions or blocks; exit-phi operands can at
most double, and repeated copying spends the shared work budget. Partial slot
facts share promotion's `16*(I+O+B)` budget and monotonic worklist. Storage and
analysis release at their function/invocation boundaries.

The one optional per-unit runtime entity is serialized as
`object=cppgm_opt_fill_bytes`, with boundary `(ptr, i32, i64) -> void`, nounwind.
It writes the low byte of its second argument to exactly the unsigned byte count
in its third; zero performs no memory access. The text reader decodes this ABI
identity once; optimization/native emission use typed IDs. Our backend emits
four MIR instructions and nine text bytes: moves to RAX/RCX, `rep stosb`, return.
The helper has no frame and uses only caller-clobbered registers. No host fill
implementation is used. Direct/replayed ELF, debug MIR, live GP/FP inputs and a
malformed helper boundary were checked.

`affected-runtime.json` preserves frozen A/D A/A plus six ABBA blocks for seven
workloads, both compilation and execution. D precedes the final exit-phi edge
repair. `affected-compile.json` remeasures A/E compiler latency/RSS and verifies
**identical objects and executables** for all seven A/D and final A/E workloads
before reusing their runtime observations. All inputs, flags, hashes, samples,
paired medians and spreads are retained. These and O0/O1/O3 common controls plus
the compiler-component control total 1,288 bound observations; 1,092 directly
support current acceptance (D's 196 compiler samples are superseded).

The initial policy's empty-range slowdown (1.453x) is preserved in
`diagnostic-initial.json`. Caller guards resolve it: current empty ranges are
0.949x, short ranges 0.798x, large reference fills 0.009x, zero fills 0.043x,
finite pointer walks 0.039x and truth materialization 0.891x runtime. Current
compiler ratios are 0.766–0.963x for these workloads, with text reductions.

Partial promotion is mandated by the course's `none(store)` outcome. Its
compiler ratio is 1.079x, runtime 0.999x, and text size unchanged. Native
inspection shows the same 80-byte body with a 32-byte rather than 16-byte frame:
this allocator does not coalesce the dead original home with the phi home.
The small bounded representation cost is disclosed; no standalone runtime gain
is claimed. Machine allocation is PA33 work and adds no PA32 exit gate.
All twelve common objects and the compiler-component object are byte-identical;
common compiler medians are 0.987–1.015x and the component is 0.987x. Raw spreads
include scheduling outliers; none were discarded. Historical evidence and
self-selected diagnostic ratio targets remain preserved, with no added gate.

Required checks are in `checks.json`, with full logs in the bound artifact
root. An earlier attempt pinned 32 test workers to one CPU and produced three
reported timeouts and one implementation-status mismatch; its records remain under `checks/`. The completed
`checks-final/` run used CPUs 4–15 and passes all 5,178 earlier tests. Performance
runs used CPU 2. The current course suite is 208/219 and the through report is
5386/5397; the exact 11 remaining failures are bound in the manifest. All personal
reducers, work/growth guards and 25 normal plus 25 debug object replays pass.
Direct debug is 5/5; source debug remains 0/3, explicitly unfinished.

The concrete boundary is the independent-argument, even-stride pointer loop:
its modulo-eight residue is invariant, yet neither argument carries a matching
residue fact. In the reduced body `p = phi(end, next); next = index i8 p, -8`,
`p != begin` cannot become false when their low three bits differ. The fixture's
three-instruction/no-phi envelope does not follow from the current proof.
Implementing further deletion requires establishing a valid contract guarantee
or a proved reference correction, not assuming alignment from a pointer's type.
This remains required implementation work; independent review must resolve the
contract/proof question. Contextual EH/calls and source ABI/debug facts remain
separate unfinished owners, including the inherited nested-cleanup reducer.
No requirement or independent review obligation is waived.
