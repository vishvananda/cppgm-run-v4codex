# Frozen final-audit protocol

Freeze incoming `7313af05a` LowIR as A and the audited implementation as B,
using the same GNU C++11/O3 build and test-runner flags. A's SHA-256 is
`844b2f121dcd29a691ba7fe5de6c15e1b5df0937e94016d7694ea8edaf728c75`,
which also matches the preceding audit's B. Hash inputs, binaries and harnesses
before measurement and recheck afterward. Keep every observation, including
failed campaigns. A later run may supersede timing only with its reason recorded.

Use the existing compiler generator at 12,000/48,000 functions, extended before timing
with a valid handler/cleanup/ordinary-phi family for the changed validation path.
Use the existing four runtime families with 40 million iterations, checked
results and volatile dynamic inputs. Compiler invocation, supplied-backend
invocation and native execution remain separate measurements. Pin one allowed
CPU; do not run builds/tests concurrently with timing. For each family retain
AAAA noise calibration followed by two wall-time ABBA blocks. Report paired
block ratios, medians and calibration spread; do not infer speedups from noise.
These counts double the historical run before measurement to leave greater
headroom above startup costs; corpus logic and budgets remain unchanged.

The prior numeric budgets remain: compiler latency <=10% + A/A spread; peak
RSS <=20% +1 MiB; compiler text <=25% growth; 4x inputs <6x wall / <5x RSS +1 MiB;
all principal observations >20x startup; executable runtime <=5% + A/A spread;
executable text growth 0%. New validation is mandatory correctness work,
bounded by two instruction traversals plus one O(E log E) ordinary-edge sort,
O(IR+E) scratch, and zero IR/code growth. Constructor slice checks are O(1).
No optimizer, fixed point, speculative code growth or new production phase is
introduced. Work-budget exhaustion in future optional transforms must retain
the input IR; malformed external IR here must be rejected, never skipped.

Also remeasure the unchanged frontend's fixed PA7 template-demand 1x/4x family
using one frozen `cppgm++` binary for both labels, eight independent TUs per
sample. Reuse its existing A/A+ABBA protocol, semantic-work assertions and
budgets unchanged. This establishes absolute current latency/RSS/text and
scaling, with no frontend speedup claim. Template source-to-LowIR, owned native
encoding and self-hosting remain later-assignment surfaces, not executable
benchmark capabilities of PA8.
