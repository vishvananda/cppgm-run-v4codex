# Aggregate construction prefix correction

Bundle source `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, SHA256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
Local revision **pa21-aggregate-prefix-108** corrects
`200-indirect-param-prologue-copy.ref`. All source inputs, statuses, comparison
rules, constructors, normal destruction and coverage are preserved.

The [reducer](../student.tests/pa21/aggregate_prefix_reducer108.cpp) retains three
source objects and an aggregate initialized by three nontrivial copies. The
[companion](../student.tests/pa21/aggregate_prefix_probe108.cpp) throws from each
copy in turn and counts live objects. It also checks normal completion. Copy 2
must destroy member `a`; copy 3 must destroy `b`, then `a`. Those cleanups precede
reverse destruction of source objects `z`, `y`, `x`.

The proof is C++11 [except.ctor] 15.2/1–2 in the supplied
[standard text](../doc/n3485.txt:21482): failure during initialization requires
destruction of the fully constructed subobjects, and automatic objects unwind
in reverse completion order. PA21 README goal 5 explicitly requires that same
construction prefix. A declared, potentially throwing copy constructor and an
externally defined destructor cannot be presumed effect-free. The original
reference sends all three copy failures directly to source-object cleanup, so
it omits required destruction; this conclusion does not depend on compiler
agreement.

The revision saves the addresses of completed members needed by later failing
copies and adds a shared reverse cleanup suffix, ending at the existing source-object continuation.
Address calculations move outside the corresponding call's protected region;
these calculations cannot throw and each constructor remains protected. Ordinary
loads, stores, `eh_try`, calls and jumps obey the existing
[LowIR contract](../pa8/lowir.md#handler-stack-management). Explicit pointer slots
also preserve these addresses across native unwind edges. The final member has
no failing later constructor and needs no saved prefix.

[Reconstruction](../student.tests/pa21/reference108.py) applies three exact,
reviewable replacements to the untouched stage-base reference. It never reads
student output. The manifest records original/revised hashes. [Execution evidence](../student.tests/pa21/reference108-execution.json) records
both versions and the reduced source through the pinned object backend and host
runtime: the original reference and entry compiler fail the live-object check;
the revised reference and final compiler pass. Every invocation, output hash
and checked exit is retained.
